#include "ComponentTest.h"
#include "MinNetGameObject.h"


void ComponentTest::InitRPC()
{
	DefRPC("SendTest", [this](MinNetPacket * packet) {
		auto num = packet->pop_int();
		auto num2 = packet->pop_float();
		auto sanz = packet->pop_string();

		SendTest(num, num2, sanz);
	});
}

void ComponentTest::SendTest(int num, float num2, std::string sanz)
{
	std::cout << num << std::endl;
	std::cout << num2 << std::endl;
	std::cout << sanz.c_str() << std::endl;

	RPC("RecvTest", gameObject->owner, "Àß µÇ³Ä??1@#123A$S@F!12df");
}

ComponentTest::ComponentTest()
{
}


ComponentTest::~ComponentTest()
{
}
