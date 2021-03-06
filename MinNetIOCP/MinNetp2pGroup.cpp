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
}

void MinNetp2pGroup::SendMemberList(MinNetUser * member)
{
	for (auto originMember : memberList)
	{
		if (originMember == member)
			continue;

		member->relayUserList.push_back(originMember);

		auto originPacket = MinNetPool::packetPool->pop();
		originPacket->create_packet(Defines::MinNetPacketType::P2P_MEMBER_CAST);

		originPacket->push(originMember->ID);
		originPacket->push(originMember->remoteIP);
		originPacket->push(originMember->remotePort);

		originPacket->create_header();
		room->GetManager()->Send(member, originPacket);
		MinNetPool::packetPool->push(originPacket);
	}
}

void MinNetp2pGroup::SendOtherJoin(MinNetUser * member)
{
	auto otherJoinPacket = MinNetPool::packetPool->pop();
	otherJoinPacket->create_packet(Defines::MinNetPacketType::OTHER_JOIN_P2P_GROUP);

	otherJoinPacket->push(member->ID);
	otherJoinPacket->push(member->remoteIP);
	otherJoinPacket->push(member->remotePort);

	otherJoinPacket->create_header();

	for (auto originMember : memberList)
	{
		if (originMember != member)
		{
			room->GetManager()->Send(originMember, otherJoinPacket);
			originMember->relayUserList.push_back(member);
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
	member->relayUserList.clear();
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
		if (originMember != member)
		{
			room->GetManager()->Send(originMember, otherLeavePacket);
			originMember->relayUserList.remove(member);
		}
	}

	MinNetPool::packetPool->push(otherLeavePacket);
}