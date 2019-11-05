#include "MinNetRoom.h"

#include "MinNetIOCP.h"
#include "MinNetOptimizer.h"
#include "MinNetPool.h"
#include "MinNetGameObject.h"

MinNetRoom::MinNetRoom()
{
	SetMaxUser(10);
}

MinNetRoom::~MinNetRoom()
{

}

void MinNetRoom::SetName(std::string name)
{
	this->name = name;
}

std::string MinNetRoom::GetName()
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

std::list<MinNetUser*> * MinNetRoom::GetUserList()
{
	return &user_list;
}

std::shared_ptr<MinNetGameObject>  MinNetRoom::Instantiate(std::string prefabName, Vector3 position, Vector3 euler, int id, bool casting, MinNetUser * except, bool autoDelete)
{
	auto obj = std::make_shared<MinNetGameObject>();

	obj->SetName(prefabName);
	obj->position = position;
	obj->rotation = euler;
	obj->SetID(id);

	if (casting)
	{
		MinNetPacket * packet = MinNetPool::packetPool->pop();
		packet->create_packet(Defines::MinNetPacketType::OBJECT_INSTANTIATE);
		packet->push(prefabName);
		packet->push(position);
		packet->push(euler);
		packet->push(id);
		packet->create_header();

		manager->Send(this, packet, except);

		MinNetPool::packetPool->push(packet);

		if (except != nullptr)
		{
			MinNetPacket * packet = MinNetPool::packetPool->pop();
			packet->create_packet(Defines::MinNetPacketType::ID_CAST);
			packet->push(obj->GetName());
			packet->push(obj->GetID());
			packet->create_header();

			manager->Send(except, packet);
		
			MinNetPool::packetPool->push(packet);
	
			if (autoDelete)
			{
				obj->owner = except;
			}
		}
	}

	AddObject(obj);
	
	return obj;
}

void MinNetRoom::Destroy(std::string prefabName, int id, bool casting, MinNetUser * except)
{
	std::shared_ptr<MinNetGameObject>  obj = nullptr;

	if (object_map.find(id) == object_map.end())
	{
		std::cout << "����ȭ ���� ����" << std::endl;
		return;
	}

	obj = object_map[id];

	if (casting)
	{
		MinNetPacket * packet = MinNetPool::packetPool->pop();
		packet->create_packet(Defines::MinNetPacketType::OBJECT_DESTROY);
		packet->push(prefabName);
		packet->push(id);
		packet->create_header();

		manager->Send(this, packet, except);

		MinNetPool::packetPool->push(packet);
	}

	RemoveObject(obj);
}

void MinNetRoom::SetManager(MinNetRoomManager * manager)
{
	this->manager = manager;
}

void MinNetRoom::AddUser(MinNetUser * user)
{
	if (user == nullptr)
		return;
	
	MinNetPacket * enter = MinNetPool::packetPool->pop();// ���Ӱ� ���� �������� ���������� �뿡 ���Դٴ� ���� �˸�
	enter->create_packet((int)Defines::MinNetPacketType::USER_ENTER_ROOM);
	// ���� �� ������ �ٸ� ������ ���� ���� ���� ����
	enter->create_header();

	manager->Send(user, enter);

	MinNetPool::packetPool->push(enter);

	std::cout << user << " ������ ������ ����" << std::endl;

	for (std::shared_ptr<MinNetGameObject>  obj : object_list)
	{
		MinNetPacket * packet = MinNetPool::packetPool->pop();
		packet->create_packet(Defines::MinNetPacketType::OBJECT_INSTANTIATE);
		packet->push(obj->GetName());
		packet->push(obj->position);
		packet->push(obj->rotation);
		packet->push(obj->GetID());
		packet->create_header();

		manager->Send(user, packet);

		MinNetPool::packetPool->push(packet);
	}

	MinNetPacket * other_enter = MinNetPool::packetPool->pop();// �ٸ� ������ ���� ���ο� ������ ���Դٴ� ���� �˸�
	other_enter->create_packet((int)Defines::MinNetPacketType::OTHER_USER_ENTER_ROOM);
	// ���� �� ������ �ٸ� ������ ���� ���� ���� ����
	other_enter->create_header();

	manager->Send(this, other_enter);

	user_list.push_back(user);// ���� ����Ʈ�� ���ο� ���� �߰�
	user_map.insert(std::make_pair(user->ID, user));

	MinNetPool::packetPool->push(other_enter);
}

void MinNetRoom::RemoveUser(MinNetUser * user)
{
	if (user == nullptr)
		return;

	std::cout << user << " ������ �濡�� ����" << std::endl;

	user_map.erase(user->ID);
	user_list.remove(user); 

	std::queue<std::shared_ptr<MinNetGameObject>> deleteQ;

	for (auto it = user->autoDeleteObjectList.begin(); it != user->autoDeleteObjectList.end(); it++)
	{
		deleteQ.push(*it);
	}

	while (!deleteQ.empty())
	{
		auto obj = deleteQ.front();
		Destroy(obj->GetName(), obj->GetID(), true, user);
		deleteQ.pop();
	}

	MinNetPacket * other_leave = MinNetPool::packetPool->pop();// �ٸ� ������ ���� � ������ �����ٴ°��� �˸�
	other_leave->create_packet((int)Defines::MinNetPacketType::OTHER_USER_LEAVE_ROOM);
	// ���� �� ������ �ٸ� ������ ���� ���� ���� ����
	other_leave->create_header();

	manager->Send(this, other_leave);

	MinNetPool::packetPool->push(other_leave);

	MinNetPacket * leave = MinNetPool::packetPool->pop();// ���� �������� ���������� �뿡�� �����ٴ� ���� �˸�
	leave->create_packet((int)Defines::MinNetPacketType::USER_LEAVE_ROOM);
	// ���� �� ������ �ٸ� ������ ���� ���� ���� ����
	leave->create_header();

	manager->Send(user, leave);

	MinNetPool::packetPool->push(leave);
}

void MinNetRoom::RemoveUsers()
{
	std::queue<MinNetUser *> deleteQ;
	for (MinNetUser * user : user_list)
	{
		deleteQ.push(user);
	}

	while (!deleteQ.empty())
	{

		RemoveUser(deleteQ.front());
		deleteQ.pop();
	}
}

void MinNetRoom::AddObject(std::shared_ptr<MinNetGameObject> object)
{
	object_list.push_back(object);
	object_map.insert(std::make_pair(object->GetID(), object));
	object->ChangeRoom(this);
	if (object->owner != nullptr)
	{
		object->owner->autoDeleteObjectList.push_back(object);// ������ ������ �ִ� ������Ʈ�� ������ ���ӿ��� ������ �Բ� ���ŵ�
	}
}

void MinNetRoom::RemoveObject(std::shared_ptr<MinNetGameObject> object)
{
	object_list.remove(object);
	object_map.erase(object->GetID());

	if (object->owner != nullptr)
	{
		object->owner->autoDeleteObjectList.remove(object);
	}
}

void MinNetRoom::RemoveObject(int id)
{
	RemoveObject(object_map[id]);
}

void MinNetRoom::RemoveObjects()
{
	std::queue<std::shared_ptr<MinNetGameObject> > deleteQ;

	for (std::shared_ptr<MinNetGameObject> obj : object_list)
	{
		deleteQ.push(obj);
	}

	while (!deleteQ.empty())
	{
		std::shared_ptr<MinNetGameObject>  obj = deleteQ.front();
		Destroy(obj->GetName(), obj->GetID(), true);
		deleteQ.pop();
	}
}

std::shared_ptr<MinNetGameObject>  MinNetRoom::GetGameObject(int id)
{
	return object_map[id];
}

int MinNetRoom::GetUserCount()
{
	return user_list.size();
}

int MinNetRoom::GetNewID()
{
	return id_count++;
}

MinNetUser * MinNetRoom::GetUser(int id)
{
	auto pair =  user_map.find(id);

	if (pair == user_map.end())
		return nullptr;// �ش� id�� ������ ������ ����
	else
		return pair->second;
}

void MinNetRoom::ObjectRPC(MinNetUser * user, MinNetPacket * packet)
{
	int id = packet->pop_int();
	std::string componentName = packet->pop_string();
	std::string methodName = packet->pop_string();
	int target = packet->pop_int();

	auto obj = GetGameObject(id);
	if (obj == nullptr)
	{
		std::cout << __FUNCTION__ << std::endl;
		return;
	}
	obj->ObjectRPC(componentName, methodName, packet);

	switch (MinNetRpcTarget(target))
	{
		case MinNetRpcTarget::All:
		case MinNetRpcTarget::Others:
			manager->Send(this, packet, user);
			break;

		case MinNetRpcTarget::AllViaServer:
			manager->Send(this, packet);
			break;

		case MinNetRpcTarget::Server:

			break;
	}

	//MinNetPool::packetPool->push(packet);
}

void MinNetRoom::SendRPC(int objectId, std::string componentName, std::string methodName, MinNetRpcTarget target, MinNetPacket * parameters)
{
	MinNetPacket * rpcPacket = MinNetPool::packetPool->pop();
	rpcPacket->create_packet((int)Defines::MinNetPacketType::RPC);

	rpcPacket->push(objectId);
	rpcPacket->push(componentName);
	rpcPacket->push(methodName);
	rpcPacket->push(int(target));

	MinNetUser * except = nullptr;
	auto obj = GetGameObject(objectId);


	if (parameters != nullptr)
	{
		int packetSize = rpcPacket->buffer_position;
		int parameterSize = parameters->size() - Defines::HEADERSIZE;
		
		memcpy(&rpcPacket->buffer[packetSize], &parameters->buffer[Defines::HEADERSIZE], parameterSize);// ���� rpc��Ŷ �ڿ� �Ķ���ͷ� ���� ���ڵ��� ����
		rpcPacket->buffer_position += (parameterSize);

		if (target == MinNetRpcTarget::All || target == MinNetRpcTarget::AllViaServer)
		{
			parameters->set_buffer_position(Defines::HEADERSIZE);
			obj->ObjectRPC(componentName, methodName, parameters);
			except = nullptr;
		}

		MinNetPool::packetPool->push(parameters);
	}

	rpcPacket->create_header();

	if(target == MinNetRpcTarget::Others)
	{
		if (obj->owner != nullptr)
		{
			except = obj->owner;
		}
	}

	manager->Send(this, rpcPacket, except);

	MinNetPool::packetPool->push(rpcPacket);
}

void MinNetRoom::SendRPC(int objectId, std::string componentName, std::string methodName, MinNetUser * target, MinNetPacket * parameters)
{
	MinNetPacket * rpcPacket = MinNetPool::packetPool->pop();
	rpcPacket->create_packet((int)Defines::MinNetPacketType::RPC);

	rpcPacket->push(objectId);
	rpcPacket->push(componentName);
	rpcPacket->push(methodName);
	rpcPacket->push((int)MinNetRpcTarget::One);

	if (parameters != nullptr)
	{
		int packetSize = rpcPacket->buffer_position;
		int parameterSize = parameters->size() - Defines::HEADERSIZE;

		memcpy(&rpcPacket->buffer[packetSize], &parameters->buffer[Defines::HEADERSIZE], parameterSize);// ���� rpc��Ŷ �ڿ� �Ķ���ͷ� ���� ���ڵ��� ����
		rpcPacket->buffer_position += (parameterSize);

		MinNetPool::packetPool->push(parameters);
	}

	rpcPacket->create_header();

	manager->Send(target, rpcPacket);

	MinNetPool::packetPool->push(rpcPacket);
}

void MinNetRoom::ObjectInstantiate(MinNetUser * user, MinNetPacket * packet)
{
	std::string prefabName = packet->pop_string();
	Vector3 position = packet->pop_vector3();
	Vector3 rotation = packet->pop_vector3();
	bool autoDelete = packet->pop_bool();

	auto obj = Instantiate(prefabName, position, rotation, GetNewID(), true, user, autoDelete);
}

void MinNetRoom::ObjectDestroy(MinNetUser * user, MinNetPacket * packet)
{
	Destroy(packet->pop_string(), packet->pop_int(), true);
}

MinNetRoomManager::MinNetRoomManager(MinNetIOCP * minnet)
{
	this->minnet = minnet;
}

MinNetRoom * MinNetRoomManager::GetPeacefulRoom()
{
	if (room_list.empty())// ���� �������� ������ ���ο� ���� ����
	{
		std::cout << "���� �������� �ʾ� ���ο� ���� ��������ϴ�" << std::endl;
		room_list.push_back(MinNetPool::roomPool->pop());
	}

	for (MinNetRoom * room : room_list)// �����ο� ���� üũ��
	{
		if (room->IsPeaceful())
		{
			room->SetManager(this);
			return room;
		}
	}

	MinNetRoom * room = MinNetPool::roomPool->pop();// �����ο� ���� ���ٸ� ���ο� ���� ����
	room->SetManager(this);
	room_list.push_back(room);
	std::cout << "�����ο� ���� �������� �ʾ� ���ο� ���� ��������ϴ�" << std::endl;
	return room;
}

void MinNetRoomManager::Send(MinNetRoom * room, MinNetPacket * packet, MinNetUser * except)
{
	for (auto user : *room->GetUserList())
	{
		if(except != user)
			Send(user, packet);
	}
}

void MinNetRoomManager::Send(MinNetUser * user, MinNetPacket * packet)
{
	minnet->StartSend(user, packet);
}

void MinNetRoomManager::PacketHandler(MinNetUser * user, MinNetPacket * packet)
{
	switch ((Defines::MinNetPacketType)packet->packet_type)
	{
	case Defines::MinNetPacketType::USER_ENTER_ROOM:
		user->ChangeRoom(GetPeacefulRoom());
		break;

	case Defines::MinNetPacketType::USER_LEAVE_ROOM:
		user->ChangeRoom(nullptr);
		break;

	case Defines::MinNetPacketType::OBJECT_INSTANTIATE:
		user->GetRoom()->ObjectInstantiate(user, packet);
		break;

	case Defines::MinNetPacketType::OBJECT_DESTROY:
		user->GetRoom()->ObjectDestroy(user, packet);
		break;

	case Defines::MinNetPacketType::RPC:
		user->GetRoom()->ObjectRPC(user, packet);
		break;

	default:
		break;
	}
}