#pragma once
#include "MinNetComponent.h"
class ComponentTest :
	public MinNetComponent
{
public:

	void InitRPC() override;

	void SendTest(int num, float num2, std::string sanz);

	ComponentTest();
	~ComponentTest();
};

