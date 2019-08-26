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

void MinNetRoom::AddUser(MinNetUser * user)
{
	if (user == nullptr)
		return;

	

	user_list.push_back(user);
}

void MinNetRoom::RemoveUser(MinNetUser * user)
{
	if (user == nullptr)
		return;



	user_list.remove(user);
}

MinNetRoomManager::MinNetRoomManager(MinNetIOCP * minnet)
{
	this->minnet = minnet;
	room_pool.AddObject(10);
}

MinNetRoom * MinNetRoomManager::GetPeacefulRoom()
{
	if (room_list.empty())// ���� �������� ������ ���ο� ���� ����
	{
		cout << "���� �������� �ʾ� ���ο� ���� ��������ϴ�" << endl;
		room_list.push_back(room_pool.pop());
	}

	for (MinNetRoom * room : room_list)// �����ο� ���� üũ��
	{
		if (room->IsPeaceful())
			return room;
	}

	MinNetRoom * room = room_pool.pop();// �����ο� ���� ���ٸ� ���ο� ���� ����
	room_list.push_back(room);
	cout << "�����ο� ���� �������� �ʾ� ���ο� ���� ��������ϴ�" << endl;
	return room;
}