#pragma once
#include "MinNetComponent.h"
class LobbyUser :
	public MinNetComponent
{
public:
	
	LobbyUser();
	~LobbyUser();

	void InitRPC() override;
	void Awake() override;

private:

	void GetRoomList();
};

