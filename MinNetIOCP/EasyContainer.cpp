#include "EasyContainer.h"

EasyContainer::EasyContainer()
{
}


EasyContainer::~EasyContainer()
{
}

void EasyContainer::SetValue(std::string key, int value)
{
	SetValue(key, std::to_string(value));
}

void EasyContainer::SetValue(std::string key, float value)
{
	SetValue(key, std::to_string(value));
}

void EasyContainer::SetValue(std::string key, bool value)
{
	std::string str = "";

	if (value)
		str = "true";
	else
		str = "false";

	SetValue(key, str);
}

void EasyContainer::SetValue(std::string key, std::string value)
{
	if (key == "")
	{
		std::cout << "Ű���� ���� �ɼ��� �߰��� �� �����ϴ�." << std::endl;
		return;
	}

	auto set = map.find(key);

	if (set == map.end())
	{// ���� ����
		map.insert(std::make_pair(key, value));
	}
	else
	{// ���� ����
		set->second = value;
	}
}

int EasyContainer::GetValueInt(std::string key)
{
	int retval = 0;
	auto value = GetValueString(key);

	if (value == "")
	{
		std::cout << "�ش� Ű�� �����ϴ� ���� �����ϴ�." << std::endl;
		return retval;
	}

	retval = atoi(value.c_str());

	return retval;
}

float EasyContainer::GetValueFloat(std::string key)
{

	float retval = 0;
	auto value = GetValueString(key);

	if (value == "")
	{
		std::cout << "�ش� Ű�� �����ϴ� ���� �����ϴ�." << std::endl;
		return retval;
	}

	retval = atof(value.c_str());

	return retval;
}

bool EasyContainer::GetValueBool(std::string key)
{
	bool retval = false;
	auto value = GetValueString(key);

	if (value == "")
	{
		std::cout << "�ش� Ű�� �����ϴ� ���� �����ϴ�." << std::endl;
		return retval;
	}


	if (value == "true")
		retval = true;
	else
		retval = false;

	return retval;
}

std::string EasyContainer::GetValueString(std::string key)
{
	std::string retval = "";

	if (key == "")
	{
		std::cout << "Ű���� ���� �ɼ��� ã�� �� �����ϴ�." << std::endl;
		return retval;
	}

	auto set = map.find(key);

	if (set == map.end())
	{// ���� ����
	}
	else
	{// ���� ����
		retval = set->second;
	}

	return retval;
}
