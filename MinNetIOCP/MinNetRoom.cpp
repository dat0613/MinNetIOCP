#include "MinNetRoom.h"

#include "MinNet.h"
#include "MinNetIOCP.h"
#include "MinNetOptimizer.h"

MinNetRoom::MinNetRoom()
{
	SetMaxUser(10);
	gameobject_pool.SetOnPush([](MinNetGameObject * obj) {
		obj->rotation = obj->position = { 0.0f, 0.0f, 0.0f };
		obj->scale = { 1.0f, 1.0f, 1.0f };
		obj->SetID(-1);
		obj->SetName("");
		obj->owner = nullptr;
	});

	gameobject_pool.AddObject(30);
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
	MinNetGameObject * obj = gameobject_pool.pop();

	obj->SetName(prefabName);
	obj->position = position;
	obj->rotation = euler;
	obj->SetID(id);

	if (casting)
	{
		MinNetPacket * packet = manager->PopPacket();
		packet->create_packet(Defines::MinNetPacketType::OBJECT_INSTANTIATE);
		packet->push(prefabName);
		packet->push(position);
		packet->push(euler);
		packet->push(id);
		packet->create_header();

		manager->Send(this, packet, except);

		manager->PushPacket(packet);

		if (except != nullptr)
		{
			MinNetPacket * packet = manager->PopPacket();
			packet->create_packet(Defines::MinNetPacketType::ID_CAST);
			packet->push(obj->GetName());
			packet->push(obj->GetID());
			packet->create_header();

			manager->Send(except, packet);
		
			manager->PushPacket(packet);
	
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
		MinNetPacket * packet = manager->PopPacket();
		packet->create_packet(Defines::MinNetPacketType::OBJECT_DESTROY);
		packet->push(prefabName);
		packet->push(id);
		packet->create_header();

		manager->Send(this, packet, except);

		manager->PushPacket(packet);
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
	
	MinNetPacket * enter = manager->PopPacket();// 새롭게 들어온 유저에게 정상적으로 룸에 들어왔다는 것을 알림
	enter->create_packet((int)Defines::MinNetPacketType::USER_ENTER_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	enter->create_header();

	manager->Send(user, enter);

	manager->PushPacket(enter);

	cout << user << " 유저가 방으로 들어옴" << endl;

	for (MinNetGameObject * obj : object_list)
	{
		MinNetPacket * packet = manager->PopPacket();
		packet->create_packet(Defines::MinNetPacketType::OBJECT_INSTANTIATE);
		packet->push(obj->GetName());
		packet->push(obj->position);
		packet->push(obj->rotation);
		packet->push(obj->GetID());
		packet->create_header();

		manager->Send(user, packet);

		manager->PushPacket(packet);
	}

	MinNetPacket * other_enter = manager->PopPacket();// 다른 유저들 에게 새로운 유저가 들어왔다는 것을 알림
	other_enter->create_packet((int)Defines::MinNetPacketType::OTHER_USER_ENTER_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	other_enter->create_header();

	manager->Send(this, other_enter);

	user_list.push_back(user);// 유저 리스트에 새로운 유저 추가

	manager->PushPacket(other_enter);
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
		RemoveObject(deleteQ.front());
		deleteQ.pop();
	}

	MinNetPacket * other_leave = manager->PopPacket();// 다른 유저들 에게 어떤 유저가 나갔다는것을 알림
	other_leave->create_packet((int)Defines::MinNetPacketType::OTHER_USER_LEAVE_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	other_leave->create_header();

	manager->Send(this, other_leave);

	manager->PushPacket(other_leave);

	MinNetPacket * leave = manager->PopPacket();// 나간 유저에게 정상적으로 룸에서 나갔다는 것을 알림
	leave->create_packet((int)Defines::MinNetPacketType::USER_LEAVE_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	leave->create_header();

	manager->Send(user, leave);

	manager->PushPacket(leave);
}

void MinNetRoom::AddObject(MinNetGameObject * object)
{
	object_list.push_back(object);
	object_map.insert(make_pair(object->GetID(), object));
	if (object->owner != nullptr)
	{
		cout << object->GetName() + " 는 자동삭제 오브젝트 임" << endl;
		object->owner->autoDeleteObjectList.push_back(object);
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
}

void MinNetRoom::RemoveObject(int id)
{
	RemoveObject(object_map[id]);
}

int MinNetRoom::GetNewID()
{
	return id_count++;
}

void MinNetRoom::ObjectRPC(MinNetUser * user, MinNetPacket * packet)
{
	cout << "RPC 전송" << endl;
	manager->Send(this, packet);
}

void MinNetRoom::ObjectInstantiate(MinNetUser * user, MinNetPacket * packet)
{
	string prefabName = packet->pop_string();
	Vector3 position = packet->pop_vector3();
	Vector3 rotation = packet->pop_vector3();
	bool autoDelete = packet->pop_bool();

	cout << prefabName + "생성 요청" << endl;

	Instantiate(prefabName, position, rotation, GetNewID(), true, user, autoDelete);
}

void MinNetRoom::ObjectDestroy(MinNetUser * user, MinNetPacket * packet)
{
	Destroy(packet->pop_string(), packet->pop_int(), true);
}

MinNetRoomManager::MinNetRoomManager(MinNetIOCP * minnet)
{
	this->minnet = minnet;
	room_pool.SetOnPush([](MinNetRoom * room){
		room->SetManager(nullptr);
	});
	room_pool.AddObject(10);
}

MinNetRoom * MinNetRoomManager::GetPeacefulRoom()
{
	if (room_list.empty())// 룸이 존재하지 않으면 새로운 룸을 만듦
	{
		cout << "룸이 존재하지 않아 새로움 룸을 만들었습니다" << endl;
		room_list.push_back(room_pool.pop());
	}

	for (MinNetRoom * room : room_list)// 여유로운 룸을 체크함
	{
		if (room->IsPeaceful())
		{
			room->SetManager(this);
			return room;
		}
	}

	MinNetRoom * room = room_pool.pop();// 여유로움 룸이 없다면 새로운 룸을 만듦
	room->SetManager(this);
	room_list.push_back(room);
	cout << "여유로운 룸이 존재하지 않아 새로움 룸을 만들었습니다" << endl;
	return room;
}

MinNetPacket * MinNetRoomManager::PopPacket()
{
	return minnet->PopPacket();
}

void MinNetRoomManager::PushPacket(MinNetPacket * packet)
{
	minnet->PushPacket(packet);
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

void MinNetGameObject::SetID(int id)
{
	this->id = id;
}

int MinNetGameObject::GetID()
{
	return id;
}

void MinNetGameObject::SetName(string name)
{
	this->name = name;
}

string MinNetGameObject::GetName()
{
	return name;
}
