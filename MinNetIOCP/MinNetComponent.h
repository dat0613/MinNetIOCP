#pragma once

#include <memory>
#include <iostream>
#include <functional>
#include <map>

class MinNetGameObject;
class MinNetPacket;

enum class MinNetRpcTarget;

class MinNetComponent
{
public:

	MinNetComponent();
	~MinNetComponent();

	void DefRPC(std::string functionName, std::function<void(void)> function);

	void SetName(std::string name);
	std::string GetName();

	virtual void InitRPC();
	virtual void Awake();
	virtual void Update();
	virtual void LateUpdate();

	void SetParent(MinNetGameObject * parent);

	MinNetGameObject * gameObject;

	void PushRpcPacket(MinNetPacket * packet);// 리플렉션도 안되고 자동 다운 캐스팅도 안되서 그냥 패킷 자체를 넣고 쓰는 식으로 함

	void RPC(std::string methodName, MinNetRpcTarget target, MinNetPacket * parameters);// RPC를 호출할때
	void CallRPC(std::string functionName, MinNetPacket * parameters);

protected:

	MinNetPacket * rpcPacket = nullptr;

private:

	std::map<std::string, std::function<void(void)>> RpcMap;
	std::string name;

};