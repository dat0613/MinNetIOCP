#include "MinNetRoom.h"

#include "MinNetIOCP.h"
#include "MinNetOptimizer.h"
#include "MinNetPool.h"
#include "MinNetGameObject.h"

#include "MinNetCache.h"
#include "MinNetp2pGroup.h"

//MinNetRoom::MinNetRoom()
//{
//	SetMaxUser(10);
//}

MinNetRoom::MinNetRoom(boost::asio::io_service& service) : strand(service)
{
	SetMaxUser(10);
}

MinNetRoom::~MinNetRoom()
{
}

void MinNetRoom::SetName(std::string name)
{
	this->name = name;
}

std::string MinNetRoom::GetName()
{
	return name;
}

void MinNetRoom::SetNumber(int number)
{
	this->room_number = number;
}

void MinNetRoom::SetMaxUser(int max)
{
	max_user = max;
}

int MinNetRoom::GetMaxUser()
{
	return max_user;
}

int MinNetRoom::UserCount()
{
	return user_list.size();
}

int MinNetRoom::GetNumber()
{
	return room_number;
}

bool MinNetRoom::IsPeaceful()
{
	if (lock)
		return false;

	if (UserCount() < max_user)
		return true;

	return false;
}

void MinNetRoom::SetLock(bool lock)
{
	this->lock = lock;
}

std::list<MinNetUser*> * MinNetRoom::GetUserList()
{
	return &user_list;
}

MinNetp2pGroup * MinNetRoom::Createp2pGroup()
{
	MinNetp2pGroup * group = new MinNetp2pGroup(this);

	p2pGroupList.push_back(group);
	
	return group;
}

std::shared_ptr<MinNetGameObject> MinNetRoom::Instantiate(std::string prefabName)
{
	return Instantiate(prefabName, Vector3(), Vector3());
}

std::shared_ptr<MinNetGameObject> MinNetRoom::Instantiate(std::string prefabName, Vector3 position, Vector3 euler)
{
	return Instantiate(prefabName, position, euler, GetNewID(), false);
}

std::shared_ptr<MinNetGameObject> MinNetRoom::Instantiate(std::string prefabName, Vector3 position, Vector3 euler, int id, bool casting, MinNetUser * except, bool autoDelete)
{
	auto obj = std::make_shared<MinNetGameObject>();

	obj->SetName(prefabName);
	obj->position = position;
	obj->rotation = euler;
	obj->SetID(id);

	if (casting)
	{
		if (obj->isSyncingObject)
		{
			MinNetPacket * packet = MinNetPool::packetPool->pop();
			packet->create_packet(Defines::MinNetPacketType::OBJECT_INSTANTIATE);
			packet->push(prefabName);
			packet->push(position);
			packet->push(euler);
			packet->push(id);
			packet->create_header();

			manager->Send(this, packet, except);

			MinNetPool::packetPool->push(packet);
		}

		if (except != nullptr)
		{
			MinNetPacket * packet = MinNetPool::packetPool->pop();
			packet->create_packet(Defines::MinNetPacketType::ID_CAST);
			packet->push(obj->GetName());
			packet->push(obj->GetID());
			packet->create_header();

			manager->Send(except, packet);

			MinNetPool::packetPool->push(packet);

			if (autoDelete)
			{
				obj->owner = except;
			}
		}
	}

	AddObject(obj);

	obj->Awake();

	if (casting)
	{
		if (obj->isSyncingObject)
		{
			for (auto user : this->user_list)
			{
				if (user != except)
				{
					obj->OnInstantiate(user);
				}
			}
		}

		if (except != nullptr)
		{
			obj->OnInstantiate(except);
		}
	}
	
	return obj;
}

void MinNetRoom::Destroy(std::string prefabName, int id, bool casting, MinNetUser * except)
{
	std::shared_ptr<MinNetGameObject> obj = nullptr;

	auto set = object_map.find(id);

	if (set == object_map.end())
	{
		std::cout << "동기화 실패 감지" << std::endl;
		return;
	}

	obj = set->second;

	if (casting && obj->isSyncingObject)
	{
		MinNetPacket * packet = MinNetPool::packetPool->pop();
		packet->create_packet(Defines::MinNetPacketType::OBJECT_DESTROY);
		packet->push(prefabName);
		packet->push(id);
		packet->create_header();

		manager->Send(this, packet, except);

		MinNetPool::packetPool->push(packet);
	}

	obj->OnDestroy();

	RemoveObject(obj);
}

void MinNetRoom::Destroy()
{
	if (object_list.size() == 0)
		return;

	std::queue<std::shared_ptr<MinNetGameObject>> deleteQ;

	for (auto obj : object_list)
	{
		deleteQ.push(obj);
	}

	while (deleteQ.size() > 0)
	{
		auto obj = deleteQ.front();
		Destroy(obj->GetName(), obj->GetID(), true);
		deleteQ.pop();
	}
}

void MinNetRoom::SetManager(MinNetRoomManager * manager)
{
	this->manager = manager;
}

MinNetRoomManager * MinNetRoom::GetManager()
{
	return this->manager;
}

void MinNetRoom::ObjectSyncing(MinNetUser * user)
{
	if (user == nullptr)
		return;

	user->loadingEnd = true;

	for (std::shared_ptr<MinNetGameObject> obj : object_list)
	{
		if (obj->isSyncingObject)
		{
			MinNetPacket * packet = MinNetPool::packetPool->pop();
			packet->create_packet(Defines::MinNetPacketType::OBJECT_INSTANTIATE);
			packet->push(obj->GetName());
			packet->push(obj->position);
			packet->push(obj->rotation);
			packet->push(obj->GetID());
			packet->create_header();

			manager->Send(user, packet);

			MinNetPool::packetPool->push(packet);

			obj->OnInstantiate(user);
		}
	}

	MinNetPacket * enter = MinNetPool::packetPool->pop();// 새롭게 들어온 유저에게 정상적으로 룸에 들어왔다는 것을 알림
	enter->create_packet((int)Defines::MinNetPacketType::USER_ENTER_ROOM);
	enter->push(user->GetRoom()->GetNumber());
	enter->push(user->GetRoom()->GetName());
	// 대충 룸 정보와 다른 정보를 같이 보낼 예정
	enter->create_header();

	manager->Send(user, enter);

	MinNetPool::packetPool->push(enter);

	MinNetPacket * other_enter = MinNetPool::packetPool->pop();// 다른 유저들 에게 새로운 유저가 들어왔다는 것을 알림
	other_enter->create_packet((int)Defines::MinNetPacketType::OTHER_USER_ENTER_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	other_enter->create_header();

	manager->Send(this, other_enter, user);

	MinNetPool::packetPool->push(other_enter);
}

void MinNetRoom::AddUser(MinNetUser * user)
{
	if (user == nullptr)
		return;
	
	if (nowSceneName != "")// 변경할 씬이 있다면
	{
		MinNetPacket * sceneChange = MinNetPool::packetPool->pop();
		sceneChange->create_packet((int)Defines::MinNetPacketType::CHANGE_SCENE);
		sceneChange->push(nowSceneName);
		sceneChange->create_header();
			
		manager->Send(user, sceneChange);

		MinNetPool::packetPool->push(sceneChange);

		user->loadingEnd = false;// 클라이언트로 부터 로딩이 끝났다는것을 받은 후부터 해당 유저에게 캐스팅을 시작함
	}
	else
	{// 씬 변경이 없기 때문에 로딩도 없음
		ObjectSyncing(user);
	}

	user_list.push_back(user);// 유저 리스트에 새로운 유저 추가 유저가 로딩되는 로딩이 빠른 유저가 새치기를 하면 안되기 때문에 로딩이 끝나기 전에 미리 넣어 둠
	user_map.insert(std::make_pair(user->ID, user));
}

void MinNetRoom::RemoveUser(MinNetUser * user)
{
	if (user == nullptr)
		return;

	if (user->nowp2pGroup != nullptr)
	{
		user->nowp2pGroup->DelMember(user);
	}

	user_map.erase(user->ID);
	user_list.remove(user); 

	std::queue<std::shared_ptr<MinNetGameObject>> deleteQ;

	for (auto it = user->autoDeleteObjectList.begin(); it != user->autoDeleteObjectList.end(); it++)
	{
		deleteQ.push(*it);
	}

	while (!deleteQ.empty())
	{
		auto obj = deleteQ.front();
		deleteQ.pop();

		Destroy(obj->GetName(), obj->GetID(), true, user);
	}

	MinNetPacket * other_leave = MinNetPool::packetPool->pop();// 다른 유저들 에게 어떤 유저가 나갔다는것을 알림
	other_leave->create_packet((int)Defines::MinNetPacketType::OTHER_USER_LEAVE_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	other_leave->create_header();

	manager->Send(this, other_leave);

	MinNetPool::packetPool->push(other_leave);

	MinNetPacket * leave = MinNetPool::packetPool->pop();// 나간 유저에게 정상적으로 룸에서 나갔다는 것을 알림
	leave->create_packet((int)Defines::MinNetPacketType::USER_LEAVE_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	leave->create_header();

	manager->Send(user, leave);

	MinNetPool::packetPool->push(leave);
}

void MinNetRoom::RemoveUsers()
{
	std::queue<MinNetUser *> deleteQ;
	for (MinNetUser * user : user_list)
	{
		deleteQ.push(user);
	}

	while (!deleteQ.empty())
	{
		RemoveUser(deleteQ.front());
		deleteQ.pop();
	}
}

void MinNetRoom::AddObject(std::shared_ptr<MinNetGameObject> object)
{
	object_list.push_front(object);
	object_map.insert(std::make_pair(object->GetID(), object));
	object->ChangeRoom(this);

 	if (object->owner != nullptr)
	{
		object->owner->autoDeleteObjectList.push_front(object);// 주인이 정해져 있는 오브젝트는 주인이 게임에서 나갈때 함께 제거됨
	}
}

void MinNetRoom::RemoveObject(std::shared_ptr<MinNetGameObject> object)
{
	object_list.remove(object);
	object_map.erase(object->GetID());

	if (object->owner != nullptr)
	{
		object->owner->autoDeleteObjectList.remove(object);
	}
}

void MinNetRoom::RemoveObject(int id)
{
	auto set = object_map.find(id);
	if (set == object_map.end())
	{

	}
	else
	{
		RemoveObject(set->second);
	}
}

void MinNetRoom::RemoveObjects()
{
	std::queue<std::shared_ptr<MinNetGameObject> > deleteQ;

	for (std::shared_ptr<MinNetGameObject> obj : object_list)
	{
		deleteQ.push(obj);
	}

	while (!deleteQ.empty())
	{
		std::shared_ptr<MinNetGameObject>  obj = deleteQ.front();
		Destroy(obj->GetName(), obj->GetID(), true);
		deleteQ.pop();
	}
}

std::shared_ptr<MinNetGameObject>  MinNetRoom::GetGameObject(int id)
{
	auto set = object_map.find(id);
	if (set == object_map.end())
	{
		return nullptr;
	}
	else
	{
		return set->second;
	}
}

std::shared_ptr<MinNetGameObject> MinNetRoom::GetGameObject(std::string prefabName)
{
	for (auto obj : object_list)
	{
		if (obj->GetName() == prefabName)
			return obj;
	}

	return nullptr;
}

std::vector<std::shared_ptr<MinNetGameObject>> & MinNetRoom::GetGameObjects(std::string prefabName)
{
	std::list<std::shared_ptr<MinNetGameObject>> objectList;

	for (auto obj : object_list)// 먼저 std::list에 오브젝트들을 넣고
	{
		if (obj->GetName() == prefabName)
			objectList.push_back(obj);
	}

	std::vector<std::shared_ptr<MinNetGameObject>> objectVector(objectList.size());

	int i = 0;
	for (auto obj : objectList)// std::vector로 바꿈
	{
		objectVector[i++] = obj;
	}

	return objectVector;
}

void MinNetRoom::ChangeRoom(std::string roomName)
{
	if (roomName == "")
		return;

	changeRoomName = roomName;
	changeRoom = true;
}

void MinNetRoom::Update()
{
	for (auto object : object_list)
	{
		object->Update();
	}

	if (changeRoom)
	{// 룸을 변경해야 한다면
		Destroy();// 룸에 있는 모든 객체 파괴

		auto userList = GetUserList();

		std::queue<MinNetUser *> tempQ;
		std::list<MinNetUser *> tempList;

		for (auto user : *userList)
		{
			tempList.push_back(user);
			tempQ.push(user);
		}

		for (auto user : tempList)
			user->ChangeRoom(nullptr);

		SetName(changeRoomName);
		MinNetCache::AddRoom(this, nullptr);

		while (!tempQ.empty())
		{
			auto front = tempQ.front();
			front->ChangeRoom(this);
			tempQ.pop();
		}

		changeRoomName = "";
		changeRoom = false;
	}
}

void MinNetRoom::LateUpdate()
{
	for (auto object : object_list)
	{
		object->LateUpdate();
	}
}

int MinNetRoom::GetNewID()
{
	return id_count++;
}

MinNetUser * MinNetRoom::GetUser(int id)
{
	auto pair =  user_map.find(id);

	if (pair == user_map.end())
		return nullptr;// 해당 id를 가지는 유저가 없음
	else
		return pair->second;
}

void MinNetRoom::ObjectRPC(MinNetUser * user, MinNetPacket * packet)
{
	int id = packet->pop_int();
	std::string componentName = packet->pop_string();
	std::string methodName = packet->pop_string();
	int target = packet->pop_int();

	auto obj = GetGameObject(id);
	if (obj == nullptr)
	{
		return;
	}
	obj->ObjectRPC(componentName, methodName, packet);

	if (!obj->isSyncingObject)// 다른 클라이언트와 동기화 되지 않는 오브젝트는 여기서 RPC호출을 끝냄
		return;

	switch (MinNetRpcTarget(target))
	{
	case MinNetRpcTarget::All:
	case MinNetRpcTarget::Others:
		manager->Send(this, packet, user);
		break;

	case MinNetRpcTarget::AllViaServer:
		manager->Send(this, packet);
		break;

	case MinNetRpcTarget::Server:

		break;
	}
}

void MinNetRoom::SendRPC(int objectId, std::string componentName, std::string methodName, MinNetRpcTarget target, MinNetPacket * parameters, bool isTcp)
{
	MinNetPacket * rpcPacket = MinNetPool::packetPool->pop();
	rpcPacket->create_packet((int)Defines::MinNetPacketType::RPC);

	rpcPacket->push(objectId);
	rpcPacket->push(componentName);
	rpcPacket->push(methodName);
	rpcPacket->push(static_cast<int>(target));

	MinNetUser * except = nullptr;
	auto obj = GetGameObject(objectId);

	if (parameters != nullptr)
	{
		int packetSize = rpcPacket->buffer_position;
		int parameterSize = parameters->size() - Defines::HEADERSIZE;
		
		memcpy(&rpcPacket->buffer[packetSize], &parameters->buffer[Defines::HEADERSIZE], parameterSize);// 보낼 rpc패킷 뒤에 파라미터로 받은 인자들을 넣음
		rpcPacket->buffer_position += (parameterSize);

		//if (target == MinNetRpcTarget::All || target == MinNetRpcTarget::AllViaServer)
		//{
		//	parameters->set_buffer_position(Defines::HEADERSIZE);
		//	obj->ObjectRPC(componentName, methodName, parameters);
		//	except = nullptr;
		//}

		MinNetPool::packetPool->push(parameters);
	}

	rpcPacket->create_header();

	rpcPacket->isTcpCasting;

	if(target == MinNetRpcTarget::Others)
	{
		if (obj->owner != nullptr)
		{
			except = obj->owner;
		}
	}

	manager->Send(this, rpcPacket, except);

	MinNetPool::packetPool->push(rpcPacket);
}

void MinNetRoom::SendRPC(int objectId, std::string componentName, std::string methodName, MinNetUser * target, MinNetPacket * parameters, bool isTcp)
{
	MinNetPacket * rpcPacket = MinNetPool::packetPool->pop();
	rpcPacket->create_packet((int)Defines::MinNetPacketType::RPC);

	rpcPacket->push(objectId);
	rpcPacket->push(componentName);
	rpcPacket->push(methodName);
	rpcPacket->push(static_cast<int>(MinNetRpcTarget::One));

	if (parameters != nullptr)
	{
		int packetSize = rpcPacket->buffer_position;
		int parameterSize = parameters->size() - Defines::HEADERSIZE;

		memcpy(&rpcPacket->buffer[packetSize], &parameters->buffer[Defines::HEADERSIZE], parameterSize);// 보낼 rpc패킷 뒤에 파라미터로 받은 인자들을 넣음
		rpcPacket->buffer_position += (parameterSize);

		MinNetPool::packetPool->push(parameters);
	}

	rpcPacket->create_header();

	rpcPacket->isTcpCasting;

	manager->Send(target, rpcPacket);

	MinNetPool::packetPool->push(rpcPacket);
}

void MinNetRoom::SetSceneName(std::string sceneName)
{
	nowSceneName = sceneName;
}

void MinNetRoom::ObjectInstantiate(MinNetUser * user, MinNetPacket * packet)
{
	std::string prefabName = packet->pop_string();
	Vector3 position = packet->pop_vector3();
	Vector3 rotation = packet->pop_vector3();
	bool autoDelete = packet->pop_bool();

	auto obj = Instantiate(prefabName, position, rotation, GetNewID(), true, user, autoDelete);
}

void MinNetRoom::ObjectDestroy(MinNetUser * user, MinNetPacket * packet)
{
	auto prefabName = packet->pop_string();
	auto objectId = packet->pop_int();
	Destroy(prefabName, objectId, true);
}

MinNetRoomManager::MinNetRoomManager() : work(service)
{
	for (int i = 0; i < Defines::BOOSTTHREADCOUNT; i++)
	{
		io_threads.create_thread(boost::bind(&boost::asio::io_service::run, &service));
	}
}

MinNetRoomManager::~MinNetRoomManager()
{
	service.stop();
	io_threads.join_all();
	service.reset();
}

MinNetRoom * MinNetRoomManager::GetPeacefulRoom(std::string roomName)
{
	if (room_list.empty())// 룸이 존재하지 않으면 새로운 룸을 만듦
	{
		std::cout << "룸이 존재하지 않아 새로움 룸을 만들었습니다" << std::endl;
		return CreateRoom(roomName, nullptr);
	}

	for (MinNetRoom * room : room_list)// 여유로운 룸을 체크함
	{
		if (room->GetName() == roomName && room->IsPeaceful())
		{
			return room;
		}
	}

	return CreateRoom(roomName, nullptr);
}

MinNetRoom * MinNetRoomManager::GetRoom(int roomId)
{
	for (auto room : room_list)
	{
		if (room->GetNumber() == roomId)
			return room;
	}

	return nullptr;
}

void MinNetRoomManager::Send(MinNetRoom * room, MinNetPacket * packet, MinNetUser * except)
{
	for (auto user : *room->GetUserList())
	{
		if(except != user)
			Send(user, packet);
	}
}

void MinNetRoomManager::Send(MinNetUser * user, MinNetPacket * packet)
{
	MinNetIOCP::StartSend(user, packet, packet->isTcpCasting);
}

void MinNetRoomManager::PacketHandler(MinNetUser * user, MinNetPacket * packet)
{
	packet->set_buffer_position(6);
	auto nowRoom = user->GetRoom();

	switch ((Defines::MinNetPacketType)packet->packet_type)
	{
	case Defines::MinNetPacketType::CREATE_ROOM:
		user->ChangeRoom(CreateRoom(packet->pop_string(), packet));
		break;

	case Defines::MinNetPacketType::USER_ENTER_ROOM:
	{
		int roomId = packet->pop_int();// 나중에 방 번호로 입장 기능을 만들때 사용할것
		if (roomId == -2)
		{
			user->ChangeRoom(GetPeacefulRoom(packet->pop_string()));
		}
		else
		{
			auto room = GetRoom(roomId);
			bool isNull = (room == nullptr);
			bool isNotPeaceful = true;

			if(!isNull)
				 isNotPeaceful = !room->IsPeaceful();

			if (isNull || isNotPeaceful)
			{
				MinNetPacket * failPacket = MinNetPool::packetPool->pop();
				failPacket->create_packet(static_cast<int>(Defines::MinNetPacketType::USER_ENTER_ROOM_FAIL));
				failPacket->push(roomId);
				if (isNull)
				{
					failPacket->push("존재하지 않는 방입니다");
				}
				if(isNotPeaceful)
				{
					failPacket->push("참여할 수 없는 방입니다");
				}
				failPacket->create_header();
				Send(user, failPacket);
				MinNetPool::packetPool->push(failPacket);
			}
			else
			{
				user->ChangeRoom(room);
			}
		}
		break;
	}

	case Defines::MinNetPacketType::USER_LEAVE_ROOM:
		user->ChangeRoom(nullptr);
		break;

	case Defines::MinNetPacketType::OBJECT_INSTANTIATE:
		if (nowRoom != nullptr)
			nowRoom->ObjectInstantiate(user, packet);
		break;

	case Defines::MinNetPacketType::OBJECT_DESTROY:
		if (nowRoom != nullptr)
			nowRoom->ObjectDestroy(user, packet);
		break;

	case Defines::MinNetPacketType::RPC:
		if (nowRoom != nullptr)
			nowRoom->ObjectRPC(user, packet);
		break;

	case Defines::MinNetPacketType::CHANGE_SCENE_COMPLETE:
		if (nowRoom != nullptr)
			nowRoom->ObjectSyncing(user);
		break;

	case Defines::MinNetPacketType::SET_USER_VALUE:
	{
		auto key = packet->pop_string();
		auto value = packet->pop_string();

		user->userValue.SetValue(key, value);
		break;
	}
	case Defines::MinNetPacketType::GET_USER_VALUE:
	{
		auto key = packet->pop_string();
		auto value = user->userValue.GetValueString(key);

		auto valuePacket = MinNetPool::packetPool->pop();

		valuePacket->create_packet(static_cast<int>(Defines::MinNetPacketType::GET_USER_VALUE));

		valuePacket->push(key);
		valuePacket->push(value);

		valuePacket->create_header();

		Send(user, valuePacket);

		MinNetPool::packetPool->push(valuePacket);
		break;
	}

	default:
		break;
	}
}

void MinNetRoomManager::Update()
{
	std::queue<MinNetRoom *> deleteQ;

	for (auto room : room_list)
	{
		if (room->destroyWhenEmpty)
		{
			if (room->UserCount() < 1)
			{
				deleteQ.push(room);
			}
		}

		service.post(room->strand.wrap(boost::bind(&MinNetRoom::Update, room)));
	}

	while (deleteQ.size() > 0)
	{
		DestroyRoom(deleteQ.front());
		deleteQ.pop();
	}
}

void MinNetRoomManager::LateUpdate()
{
	for (auto room : room_list)
	{
		service.post(room->strand.wrap(boost::bind(&MinNetRoom::LateUpdate, room)));
	}
}

std::list<MinNetRoom*>& MinNetRoomManager::GetRoomList()
{
	return room_list;
}

MinNetRoom * MinNetRoomManager::CreateRoom(std::string roomName, MinNetPacket * packet)
{
	auto room = new MinNetRoom(service);

	room->SetManager(this);
	room->SetName(roomName);	
	room->SetNumber(roomNumberCount++);

	MinNetCache::AddRoom(room, packet);

	room_list.push_back(room);

	return room;
}

void MinNetRoomManager::DestroyRoom(MinNetRoom * room)
{
	room->Destroy();
	room_list.remove(room);
	delete room;
}