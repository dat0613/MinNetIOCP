#include "MinNetp2pGroup.h"
#include "MinNet.h"
#include "MinNetPool.h"
#include "MinNetRoom.h"
#include "Debug.h"

MinNetp2pGroup::MinNetp2pGroup(MinNetRoom * room) : room(room)
{

}

MinNetp2pGroup::~MinNetp2pGroup()
{

}

void MinNetp2pGroup::AddMember(MinNetUser * member)
{
	if (IsMember(member))
		return;

	memberList.push_back(member);
	memberMap.insert(std::make_pair(member->ID, member));

	Debug::Log(member, "가 p2p 그룹에 들어옴");

	SendJoin(member);
	SendMemberList(member);
	SendOtherJoin(member);

	member->nowp2pGroup = this;
}

void MinNetp2pGroup::DelMember(MinNetUser * member)
{
	if (!IsMember(member))
		return;

	memberList.remove(member);
	memberMap.erase(member->ID);

	SendLeave(member);
	SendOtherLeave(member);

	member->nowp2pGroup = nullptr;
}

bool MinNetp2pGroup::IsMember(MinNetUser * member)
{
	auto set = memberMap.find(member->ID);
	return !(set == memberMap.end());
}

void MinNetp2pGroup::SendJoin(MinNetUser * member)
{
	auto joinPacket = MinNetPool::packetPool->pop();
	joinPacket->create_packet(Defines::MinNetPacketType::JOIN_P2P_GROUP);
	joinPacket->create_header();
	room->GetManager()->Send(member, joinPacket);
	MinNetPool::packetPool->push(joinPacket);
	Debug::Log(member, "에게 join패킷을 보냄");
}

void MinNetp2pGroup::SendMemberList(MinNetUser * member)
{
	Debug::Log(member, "에게 멤버 리스트 보내기 시작");

	for (auto originMember : memberList)
	{
		if (originMember == member)
			continue;

		Debug::Log(member, "에게", originMember, "의 정보를 보냄");

		std::string remoteAddr = inet_ntoa(originMember->addr->sin_addr);
		int port = ntohs(originMember->addr->sin_port);

		auto originPacket = MinNetPool::packetPool->pop();
		originPacket->create_packet(Defines::MinNetPacketType::P2P_MEMBER_CAST);

		originPacket->push(originMember->ID);
		originPacket->push(remoteAddr);
		originPacket->push(port);

		originPacket->create_header();
		room->GetManager()->Send(member, originPacket);
		MinNetPool::packetPool->push(originPacket);
	}

	Debug::Log(member, "에게 멤버 리스트 보내기 끝");
}

void MinNetp2pGroup::SendOtherJoin(MinNetUser * member)
{
	std::string remoteAddr = inet_ntoa(member->addr->sin_addr);
	int port = ntohs(member->addr->sin_port);

	auto otherJoinPacket = MinNetPool::packetPool->pop();
	otherJoinPacket->create_packet(Defines::MinNetPacketType::OTHER_JOIN_P2P_GROUP);

	otherJoinPacket->push(member->ID);
	otherJoinPacket->push(remoteAddr);
	otherJoinPacket->push(port);

	otherJoinPacket->create_header();

	for (auto originMember : memberList)
	{
		if (originMember != member)
		{
			room->GetManager()->Send(originMember, otherJoinPacket);
			Debug::Log(originMember, "에게", member, "의 정보를 보냄");
		}
	}

	MinNetPool::packetPool->push(otherJoinPacket);
}

void MinNetp2pGroup::SendLeave(MinNetUser * member)
{
	auto leavePacket = MinNetPool::packetPool->pop();
	leavePacket->create_packet(Defines::MinNetPacketType::LEAVE_P2P_GROUP);
	leavePacket->create_header();
	room->GetManager()->Send(member, leavePacket);
	MinNetPool::packetPool->push(leavePacket);
}

void MinNetp2pGroup::SendOtherLeave(MinNetUser * member)
{
	auto otherLeavePacket = MinNetPool::packetPool->pop();
	otherLeavePacket->create_packet(Defines::MinNetPacketType::OTHER_LEAVE_P2P_GROUP);
	otherLeavePacket->push(member->ID);
	otherLeavePacket->create_header();

	for (auto originMember : memberList)
	{
		if(originMember != member)
			room->GetManager()->Send(originMember, otherLeavePacket);
	}

	MinNetPool::packetPool->push(otherLeavePacket);
}