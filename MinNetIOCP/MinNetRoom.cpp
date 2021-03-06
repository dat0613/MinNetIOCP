#include "MinNetRoom.h"

#include "MinNetIOCP.h"
#include "MinNetOptimizer.h"
#include "MinNetPool.h"
#include "MinNetGameObject.h"

#include "MinNetPrefab.h"
#include "Debug.h"

#include "MinNetNavMeshAgent.h"

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

MinNetp2pGroup * MinNetRoom::Createp2pGroup()
{
	auto group = new MinNetp2pGroup(this);

	p2pGroupList.push_back(group);

	return group;
}


std::list<MinNetUser*> * MinNetRoom::GetUserList()
{
	return &user_list;
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

void MinNetRoom::Update(float tick)
{
	UpdateCrowd(tick);

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
		MinNetPrefab::AddRoom(this, nullptr);

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

	if(MinNetRpcTarget(target) != MinNetRpcTarget::P2Pgroup)
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
	case MinNetRpcTarget::P2PgroupAndServer:

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
	rpcPacket->push(static_cast<int>(MinNetRpcTarget::One));// target으로 지정된 클라이언트에게만 패킷을 보내기 때문에 One임, 이 값은 클라이언트 측 에서 사용되지 않지만 나중을 위해 모든 RPC 패킷에 4바이트를 추가해 둔 것임.

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

void MinNetRoom::ResetNavSystem()
{
	dtFreeNavMeshQuery(navMeshQuery);
	dtFreeCrowd(crowd);

	memset(&filter, 0, sizeof(filter));
}

void MinNetRoom::SetNavSystem(dtNavMesh * navMesh)
{
	if (navMesh == nullptr)	return;

	navMeshQuery = dtAllocNavMeshQuery();

	auto retval = navMeshQuery->init(navMesh, 2048);

	if (dtStatusFailed(retval))
	{
		Debug::Log("navMeshQuery::init 실패");
		return;
	}

	filter.setAreaCost(MinNetPolyAreas::POLYAREA_GROUND, 1.0f);
	filter.setAreaCost(MinNetPolyAreas::POLYAREA_WATER, 10.0f);
	filter.setAreaCost(MinNetPolyAreas::POLYAREA_ROAD, 1.0f);
	filter.setAreaCost(MinNetPolyAreas::POLYAREA_DOOR, 1.0f);
	filter.setAreaCost(MinNetPolyAreas::POLYAREA_GRASS, 2.0f);
	filter.setAreaCost(MinNetPolyAreas::POLYAREA_JUMP, 1.5f);

	filter.setIncludeFlags(MinNetPolyFlags::POLYFLAGS_ALL ^ MinNetPolyFlags::POLYFLAGS_DISABLED);
	filter.setExcludeFlags(0);

	crowd = dtAllocCrowd();
	crowd->init(maxNavAgent, maxNavAgentRadius, navMesh);
	crowd->getEditableFilter(0)->setExcludeFlags(POLYFLAGS_DISABLED);
}

int MinNetRoom::GetMaxNavAgent()
{
	return maxNavAgent;
}

void MinNetRoom::SetMaxNavAgent(int max)
{
	maxNavAgent = max;
}

float MinNetRoom::GetMaxNavAgentRadius()
{
	return maxNavAgentRadius;
}

void MinNetRoom::SetMaxNavAgentRadius(float radius)
{
	this->maxNavAgentRadius = radius;
}

void MinNetRoom::AddNavAgent(MinNetNavMeshAgent * agent)
{
	if (agent == nullptr) return;

	float position[3];
	agent->gameObject->position.ToArray(position);

	float halfExtents[3];
	agent->halfExtends.ToArray(halfExtents);
	float pointOnPoly[3] = { 0.0f, 0.0f, 0.0f };

	dtPolyRef ref;

	bool retval = GetPointOnPoly(position, halfExtents, pointOnPoly, &ref);

	if (!retval) return;

 	int idx = crowd->addAgent(pointOnPoly, &agent->agentParams);

	if (idx < 0) return;

	agent->SetIdx(idx);
}

void MinNetRoom::DelNavAgent(int idx)
{
	crowd->removeAgent(idx);
}

void MinNetRoom::UpdateAgentParameters(int idx, const dtCrowdAgentParams * params)
{
	if (params == nullptr) return;

	crowd->updateAgentParameters(idx, params);
}

void MinNetRoom::RequestMoveTarget(const int idx, Vector3 endPosition, Vector3 halfExtents)
{
	dtPolyRef ref;

	float pointOnPoly[3] = { 0.0f, 0.0f, 0.0f };
	float position[3];
	float half[3];

	endPosition.ToArray(position);
	halfExtents.ToArray(half);

	auto retval = GetPointOnPoly(position, half, pointOnPoly, &ref);
	if (!retval) return;

	crowd->requestMoveTarget(idx, ref, pointOnPoly);
}

void MinNetRoom::SetObstacleAvoidanceParams(const int idx, float velBias, float adaptiveDivs, float adaptiveRigns, float adaptiveDepth)
{
	dtObstacleAvoidanceParams params;
	memcpy(&params, crowd->getObstacleAvoidanceParams(0), sizeof(dtObstacleAvoidanceParams));

	params.velBias = velBias;
	params.adaptiveDivs = adaptiveDivs;
	params.adaptiveRings = adaptiveRigns;
	params.adaptiveDepth = adaptiveDepth;

	crowd->setObstacleAvoidanceParams(0, &params);
}

bool MinNetRoom::GetPointOnPoly(float * position, float * halfExtents, float * pointOnPoly, dtPolyRef * polyRef)
{
	dtPolyRef ref;

	auto status = navMeshQuery->findNearestPoly(position, halfExtents, &filter, &ref, 0);
	if (dtStatusFailed(status))
	{
		return false;
	}

	bool pointOverPoly;
	status = navMeshQuery->closestPointOnPoly(ref, position, pointOnPoly, &pointOverPoly);

	if (dtStatusFailed(status))
	{
		return false;
	}

	*polyRef = ref;
	return true;
}

void MinNetRoom::UpdateCrowd(float tick)
{
	int maxAgentCount = crowd->getAgentCount();

	crowd->update(tick, &agentDebugInfo);

	for (int i = 0; i < maxAgentCount; i++)
	{
		auto agent = crowd->getAgent(i);

		if (!agent->active) continue;

		auto component = static_cast<MinNetNavMeshAgent *>(agent->params.userData);
		auto gameObject = component->gameObject;

		gameObject->position.x = -agent->npos[0] + component->addtion.x;
		gameObject->position.y = agent->npos[1] + component->addtion.y;
		gameObject->position.z = agent->npos[2] + component->addtion.z;

		component->velocity.x = -agent->vel[0];
		component->velocity.y = agent->vel[1];
		component->velocity.z = agent->vel[2];		
	}
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

	switch ((Defines::MinNetPacketType)packet->packet_type)
	{
	case Defines::MinNetPacketType::CREATE_ROOM:
		user->ChangeRoom(CreateRoom(packet->pop_string(), packet));
		break;

	case Defines::MinNetPacketType::USER_ENTER_ROOM:
		UserEnterRoom(user, packet);
		break;

	case Defines::MinNetPacketType::USER_LEAVE_ROOM:
		user->ChangeRoom(nullptr);
		break;

	case Defines::MinNetPacketType::OBJECT_INSTANTIATE:
		if (user->GetRoom() != nullptr)
			user->GetRoom()->ObjectInstantiate(user, packet);
		break;

	case Defines::MinNetPacketType::OBJECT_DESTROY:
		if (user->GetRoom() != nullptr)
			user->GetRoom()->ObjectDestroy(user, packet);
		break;

	case Defines::MinNetPacketType::RPC_P2P:
	case Defines::MinNetPacketType::RPC:
		if (user->GetRoom() != nullptr)
			user->GetRoom()->ObjectRPC(user, packet);
		break;

	case Defines::MinNetPacketType::CHANGE_SCENE_COMPLETE:
		if (user->GetRoom() != nullptr)
			user->GetRoom()->ObjectSyncing(user);
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

void MinNetRoomManager::Update(float tick)
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

		service.post(room->strand.wrap(boost::bind(&MinNetRoom::Update, room, tick)));
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

void MinNetRoomManager::UserEnterRoom(MinNetUser * user, MinNetPacket * packet)
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

		if (!isNull)
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
			if (isNotPeaceful)
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
}

MinNetRoom * MinNetRoomManager::CreateRoom(std::string roomName, MinNetPacket * packet)
{
	auto room = new MinNetRoom(service);

	room->SetManager(this);
	room->SetName(roomName);	
	room->SetNumber(roomNumberCount++);

	MinNetPrefab::AddRoom(room, packet);

	room_list.push_back(room);

	return room;
}

void MinNetRoomManager::DestroyRoom(MinNetRoom * room)
{
	room->Destroy();
	room_list.remove(room);
	delete room;
}