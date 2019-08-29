#include "MinNetRoom.h"

#include "MinNet.h"
#include "MinNetIOCP.h"
#include "MinNetOptimizer.h"

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

void MinNetRoom::SetManager(MinNetRoomManager * manager)
{
	this->manager = manager;
}

void MinNetRoom::AddUser(MinNetUser * user)
{
	if (user == nullptr)
		return;
	
	MinNetPacket * other_enter = manager->PopPacket();// 다른 유저들 에게 새로운 유저가 들어왔다는 것을 알림
	other_enter->create_packet((int)Defines::MinNetPacketType::OTHER_USER_ENTER_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	other_enter->create_header();

	manager->Send(this, other_enter);

	lock();
	user_list.push_back(user);// 유저 리스트에 새로운 유저 추가
	unlock();

	MinNetPacket * enter = manager->PopPacket();// 새롭게 들어온 유저에게 정상적으로 룸에 들어왔다는 것을 알림
	enter->create_packet((int)Defines::MinNetPacketType::USER_ENTER_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	enter->create_header();

	manager->Send(user, enter);


	cout << user << " 유저가 방으로 들어옴" << endl;
}

void MinNetRoom::RemoveUser(MinNetUser * user)
{
	if (user == nullptr)
		return;

	cout << user << " 유저가 방에서 나감" << endl;

	lock();
	user_list.remove(user); 
	unlock();

	MinNetPacket * other_leave = manager->PopPacket();// 다른 유저들 에게 어떤 유저가 나갔다는것을 알림
	other_leave->create_packet((int)Defines::MinNetPacketType::OTHER_USER_LEAVE_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	other_leave->create_header();

	manager->Send(this, other_leave);

	MinNetPacket * leave = manager->PopPacket();// 나간 유저에게 정상적으로 룸에서 나갔다는 것을 알림
	leave->create_packet((int)Defines::MinNetPacketType::USER_LEAVE_ROOM);
	// 대충 룸 정보와 다른 정보를 넣을 같이 보낼 예정
	leave->create_header();

	manager->Send(user, leave);
}

void MinNetRoom::AddObject(MinNetGameObject * object, string name, int id, MinNetUser * spawner)
{
	object->SetID(id);
	object->SetName(name);

	MinNetPacket * packet = manager->PopPacket();
	packet->create_packet(Defines::MinNetPacketType::OBJECT_INSTANTIATE);
	packet->push(name);
	packet->push(id);
	manager->Send(this, packet);
}

void MinNetRoom::RemoveObject(MinNetGameObject * object)
{

}

void MinNetRoom::RemoveObject(int id)
{

}

int MinNetRoom::GetNewID()
{
	return id_count++;
}

void MinNetRoom::lock()
{
	user_lock.lock();
}

void MinNetRoom::unlock()
{
	user_lock.unlock();
}

MinNetRoomManager::MinNetRoomManager(MinNetIOCP * minnet)
{
	this->minnet = minnet;
	room_pool.SetOnPush
	(
		[](MinNetRoom * room)
		{
			room->SetManager(nullptr);
		}
	);
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
	room->lock();
	packet->send_count = room->GetUserList()->size();

	if (except != nullptr)
		packet->send_count--;

	for (auto user : *room->GetUserList())
	{
		if(except != user)
			Send(user, packet);
	}
	room->unlock();
}

void MinNetRoomManager::Send(MinNetUser * user, MinNetPacket * packet)
{
	minnet->StartSend(user, packet);
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
