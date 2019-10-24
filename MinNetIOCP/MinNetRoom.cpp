#include "MinNetRoom.h"

#include "MinNetIOCP.h"
#include "MinNetOptimizer.h"
#include "MinNetPool.h"
#include "MinNetGameObject.h"
#include "MinNetLua.h"

MinNetRoom::MinNetRoom()
{
	SetMaxUser(10);
}

MinNetRoom::~MinNetRoom()
{

}

void MinNetRoom::SetName(string name)
{
	this->name = name;
}

string MinNetRoom::GetName()
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
	if (UserCount() < max_user)
		return true;

	return false;
}

list<MinNetUser*> * MinNetRoom::GetUserList()
{
	return &user_list;
}

MinNetGameObject * MinNetRoom::Instantiate(string prefabName, Vector3 position, Vector3 euler, int id, bool casting, MinNetUser * except, bool autoDelete)
{
	MinNetGameObject * obj = MinNetPool::gameobjectPool->pop();

	obj->SetName(prefabName);
	obj->position = position;
	obj->rotation = euler;
	obj->SetID(id);

	if (casting)
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

	return obj;
}

void MinNetRoom::Destroy(string prefabName, int id, bool casting, MinNetUser * except)
{
	MinNetGameObject * obj = nullptr;

	if (object_map.find(id) == object_map.end())
	{
		cout << "동기화 실패 감지" << endl;
		return;
	}

	obj = object_map[id];

	if (casting)
	{
		MinNetPacket * packet = MinNetPool::packetPool->pop();
		packet->create_packet(Defines::MinNetPacketType::OBJECT_DESTROY);
		packet->push(prefabName);
		packet->push(id);
		packet->create_header();

		manager->Send(this, packet, except);

		MinNetPool::packetPool->push(packet);
	}

	RemoveObject(obj);
}

void MinNetRoom::SetManager(MinNetRoomManager * manager)
{
	this->manager = manager;
}

void MinNetRoom::AddUser(MinNetUser * user)
{
	if (user == nullptr)
		return;
	
	MinNetPacket * enter = MinNetPool::packetPool->pop();// 새롭게 들어온 유저에게 정상적으로 룸에 들어왔다는 것을 알림
	enter->create_packet((int)Defines::MinNetPacketType::USER_ENTER_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	enter->create_header();

	manager->Send(user, enter);

	MinNetPool::packetPool->push(enter);

	cout << user << " 유저가 방으로 들어옴" << endl;

	for (MinNetGameObject * obj : object_list)
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
	}

	MinNetPacket * other_enter = MinNetPool::packetPool->pop();// 다른 유저들 에게 새로운 유저가 들어왔다는 것을 알림
	other_enter->create_packet((int)Defines::MinNetPacketType::OTHER_USER_ENTER_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	other_enter->create_header();

	manager->Send(this, other_enter);

	user_list.push_back(user);// 유저 리스트에 새로운 유저 추가

	MinNetPool::packetPool->push(other_enter);
}

void MinNetRoom::RemoveUser(MinNetUser * user)
{
	if (user == nullptr)
		return;

	cout << user << " 유저가 방에서 나감" << endl;

	user_list.remove(user); 

	queue<MinNetGameObject *> deleteQ;

	for (auto it = user->autoDeleteObjectList.begin(); it != user->autoDeleteObjectList.end(); it++)
	{
		deleteQ.push(*it);
	}

	while (!deleteQ.empty())
	{
		auto obj = deleteQ.front();
		Destroy(obj->GetName(), obj->GetID(), true, user);
		deleteQ.pop();
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
	queue<MinNetUser *> deleteQ;
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

void MinNetRoom::AddObject(MinNetGameObject * object)
{
	object_list.push_back(object);
	object_map.insert(make_pair(object->GetID(), object));
	object->ChangeRoom(this);
	if (object->owner != nullptr)
	{
		object->owner->autoDeleteObjectList.push_back(object);// 주인이 정해져 있는 오브젝트는 주인이 게임에서 나갈때 함께 제거됨
	}
}

void MinNetRoom::RemoveObject(MinNetGameObject * object)
{
	object_list.remove(object);
	object_map.erase(object->GetID());

	if (object->owner != nullptr)
	{
		object->owner->autoDeleteObjectList.remove(object);
	}

	MinNetPool::gameobjectPool->push(object);
}

void MinNetRoom::RemoveObject(int id)
{
	RemoveObject(object_map[id]);
}

void MinNetRoom::RemoveObjects()
{
	queue<MinNetGameObject *> deleteQ;

	for (MinNetGameObject * obj : object_list)
	{
		deleteQ.push(obj);
	}

	while (!deleteQ.empty())
	{
		MinNetGameObject * obj = deleteQ.front();
		Destroy(obj->GetName(), obj->GetID(), true);
		deleteQ.pop();
	}
}

MinNetGameObject * MinNetRoom::GetGameObject(int id)
{
	return object_map[id];
}

int MinNetRoom::GetUserCount()
{
	return user_list.size();
}

int MinNetRoom::GetNewID()
{
	return id_count++;
}

void MinNetRoom::ObjectRPC(MinNetUser * user, MinNetPacket * packet)
{
	int id = packet->pop_int();
	string componentName = packet->pop_string();
	string methodName = packet->pop_string();
	int target = packet->pop_int();

	auto obj = GetGameObject(id);
	if (obj == nullptr)
	{
		cout << __FUNCTION__ << endl;
		return;
	}
	obj->ObjectRPC(componentName, methodName, packet);

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

void MinNetRoom::SendRPC(int objectId, std::string componentName, std::string methodName, MinNetRpcTarget target, MinNetPacket * parameters)
{
	MinNetPacket * rpcPacket = MinNetPool::packetPool->pop();
	rpcPacket->create_packet((int)Defines::MinNetPacketType::RPC);
	rpcPacket->push(objectId);
	rpcPacket->push(componentName);
	rpcPacket->push(methodName);
	rpcPacket->push(int(target));

	if (parameters != nullptr)
	{
		memcpy(&rpcPacket->buffer[rpcPacket->buffer_position], &parameters[6], parameters->size() - 6);// 보낼 rpc패킷 뒤에 파라미터로 받은 인자들을 넣음
		MinNetPool::packetPool->push(parameters);
	}

	rpcPacket->create_header();
	
	MinNetUser * except = nullptr;
	auto obj = GetGameObject(objectId);

	if (target == MinNetRpcTarget::All || target == MinNetRpcTarget::AllViaServer)
	{
		rpcPacket->buffer_position = 6;
		obj->ObjectRPC(componentName, methodName, rpcPacket);
		except = nullptr;
	}

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

void MinNetRoom::ObjectInstantiate(MinNetUser * user, MinNetPacket * packet)
{
	string prefabName = packet->pop_string();
	Vector3 position = packet->pop_vector3();
	Vector3 rotation = packet->pop_vector3();
	bool autoDelete = packet->pop_bool();

	Instantiate(prefabName, position, rotation, GetNewID(), true, user, autoDelete);
}

void MinNetRoom::ObjectDestroy(MinNetUser * user, MinNetPacket * packet)
{
	Destroy(packet->pop_string(), packet->pop_int(), true);
}

MinNetRoomManager::MinNetRoomManager(MinNetIOCP * minnet)
{
	this->minnet = minnet;
}

MinNetRoom * MinNetRoomManager::GetPeacefulRoom()
{
	if (room_list.empty())// 룸이 존재하지 않으면 새로운 룸을 만듦
	{
		cout << "룸이 존재하지 않아 새로움 룸을 만들었습니다" << endl;
		room_list.push_back(MinNetPool::roomPool->pop());
	}

	for (MinNetRoom * room : room_list)// 여유로운 룸을 체크함
	{
		if (room->IsPeaceful())
		{
			room->SetManager(this);
			return room;
		}
	}

	MinNetRoom * room = MinNetPool::roomPool->pop();// 여유로움 룸이 없다면 새로운 룸을 만듦
	room->SetManager(this);
	room_list.push_back(room);
	cout << "여유로운 룸이 존재하지 않아 새로움 룸을 만들었습니다" << endl;
	return room;
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
	minnet->StartSend(user, packet);
}

void MinNetRoomManager::PacketHandler(MinNetUser * user, MinNetPacket * packet)
{
	switch ((Defines::MinNetPacketType)packet->packet_type)
	{
	case Defines::MinNetPacketType::USER_ENTER_ROOM:
		user->ChangeRoom(GetPeacefulRoom());
		break;

	case Defines::MinNetPacketType::USER_LEAVE_ROOM:
		user->ChangeRoom(nullptr);
		break;

	case Defines::MinNetPacketType::OBJECT_INSTANTIATE:
		user->GetRoom()->ObjectInstantiate(user, packet);
		break;

	case Defines::MinNetPacketType::OBJECT_DESTROY:
		user->GetRoom()->ObjectDestroy(user, packet);
		break;

	case Defines::MinNetPacketType::RPC:
		user->GetRoom()->ObjectRPC(user, packet);
		break;

	default:
		break;
	}
}