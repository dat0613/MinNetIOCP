#pragma once

#include <list>
#include <map>

class MinNetUser;
class MinNetRoom;

class MinNetp2pGroup
{
public:
	
	MinNetp2pGroup(MinNetRoom * room);
	~MinNetp2pGroup();

	void AddMember(MinNetUser * member);
	void DelMember(MinNetUser * member);
	bool IsMember(MinNetUser * member);

private:

	std::list<MinNetUser *> memberList;
	std::map<int, MinNetUser *> memberMap;
	MinNetRoom * room = nullptr;

	void SendJoin(MinNetUser * member);
	void SendMemberList(MinNetUser * member);
	void SendOtherJoin(MinNetUser * member);

	void SendLeave(MinNetUser * member);
	void SendOtherLeave(MinNetUser * member);
};