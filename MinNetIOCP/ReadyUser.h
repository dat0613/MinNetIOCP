#pragma once
#include "MinNetComponent.h"
#include "PlayerMove.h"

class ReadyRoomManager;

class ReadyUser : public MinNetComponent
{
public:

	void InitRPC() override;
	void Awake() override;
	void OnDestroy() override;

	bool isMaster = false;

	void OnInstantiate(MinNetUser * user) override;

	ReadyUser();
	~ReadyUser();
	std::string nickName = "";

	PlayerMove::Team GetTeam() const;
	void SetTeam(PlayerMove::Team team);

	bool GetIsMaster();
	
	int orderCount = 0;

private:

	ReadyRoomManager * manager = nullptr;
	void SendChat(std::string chat);
	void SetMaster(bool isMaster);

	void GameStart();
	void ChangeTeam();

	PlayerMove::Team team = PlayerMove::Team::None;
};