#pragma once
#include "MinNetComponent.h"
class ReadyRoomManager :
	public MinNetComponent
{
public:
	ReadyRoomManager();
	~ReadyRoomManager();
	
	void OnInstantiate(MinNetUser * user) override;

	void SetOption(std::string roomName, int TeamNumber, bool CanBargeIn, bool OnlyHeadShot, int TicketCount, float RespawnTime, int DefaultDamage, float HeadShotDamageMultiple, int PlayerMaxHP);

private:

	std::string roomName = "";
	int TeamNumber = 0;
	bool CanBargeIn = false;
	bool OnlyHeadShot = false;
	int TicketCount = 0;
	float RespawnTime = 0.0f;
	int DefaultDamage = 0;
	float HeadShotDamageMultiple = 0.0f;
	int PlayerMaxHP = 0;
};