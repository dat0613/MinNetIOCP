#pragma once
#include "MinNetComponent.h"
class ReadyUser :
	public MinNetComponent
{
public:

	void InitRPC() override;
	void Awake() override;

	void OnInstantiate(MinNetUser * user) override;

	ReadyUser();
	~ReadyUser();
	std::string nickName;

private:

	void SendChat(std::string chat);
};

