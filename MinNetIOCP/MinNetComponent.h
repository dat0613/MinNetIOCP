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

	void PushRpcPacket(MinNetPacket * packet);// ���÷��ǵ� �ȵǰ� �ڵ� �ٿ� ĳ���õ� �ȵǼ� �׳� ��Ŷ ��ü�� �ְ� ���� ������ ��

	void RPC(std::string methodName, MinNetRpcTarget target, MinNetPacket * parameters);// RPC�� ȣ���Ҷ�
	void CallRPC(std::string functionName, MinNetPacket * parameters);

protected:

	MinNetPacket * rpcPacket = nullptr;

private:

	std::map<std::string, std::function<void(void)>> RpcMap;
	std::string name;

};