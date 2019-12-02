#include "ReadyRoomManager.h"



ReadyRoomManager::ReadyRoomManager()
{
}


ReadyRoomManager::~ReadyRoomManager()
{
}

void ReadyRoomManager::OnInstantiate(MinNetUser * user)
{
	RPC("LoadRoomSetting", user, roomName, TeamNumber, CanBargeIn, OnlyHeadShot, TicketCount, RespawnTime, DefaultDamage, HeadShotDamageMultiple, PlayerMaxHP);
}

void ReadyRoomManager::SetOption(std::string roomName, int TeamNumber, bool CanBargeIn, bool OnlyHeadShot, int TicketCount, float RespawnTime, int DefaultDamage, float HeadShotDamageMultiple, int PlayerMaxHP)
{

	std::cout << roomName.c_str() << " �� �̸��� ���� ���ӹ��� ����" << std::endl;

	this->roomName = roomName;
	this->TeamNumber = TeamNumber;
	this->CanBargeIn = CanBargeIn;
	this->OnlyHeadShot = OnlyHeadShot;
	this->RespawnTime = RespawnTime;
	this->DefaultDamage = DefaultDamage;
	this->HeadShotDamageMultiple = HeadShotDamageMultiple;
	this->PlayerMaxHP = PlayerMaxHP;
}