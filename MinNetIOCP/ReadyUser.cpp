#include "ReadyUser.h"

#include "MinNetGameObject.h"

void ReadyUser::InitRPC()
{
	DefRPC("SendChat", [this](MinNetPacket * packet) 
	{
		auto chat = packet->pop_string();

		this->SendChat(chat);
	});
}

void ReadyUser::Awake()
{
	nickName = gameObject->owner->userValue.GetValueString("NickName");
}

void ReadyUser::OnInstantiate(MinNetUser * user)
{
	RPC("SyncOption", user);
}

ReadyUser::ReadyUser()
{
}

ReadyUser::~ReadyUser()
{
}

void ReadyUser::SendChat(std::string chat)
{
	auto str2 = nickName + " : " + chat;

	RPC("ChatCast", MinNetRpcTarget::AllNotServer, str2);
}