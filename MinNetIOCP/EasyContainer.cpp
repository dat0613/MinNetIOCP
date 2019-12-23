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
		std::cout << "키값이 없는 옵션은 추가할 수 없습니다." << std::endl;
		return;
	}

	auto set = map.find(key);

	if (set == map.end())
	{// 값이 없음
		map.insert(std::make_pair(key, value));
	}
	else
	{// 값이 있음
		set->second = value;
	}
}

int EasyContainer::GetValueInt(std::string key)
{
	int retval = 0;
	auto value = GetValueString(key);

	if (value == "")
	{
		std::cout << "해당 키에 대응하는 값이 없습니다." << std::endl;
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
		std::cout << "해당 키에 대응하는 값이 없습니다." << std::endl;
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
		std::cout << "해당 키에 대응하는 값이 없습니다." << std::endl;
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
		std::cout << "키값이 없는 옵션은 찾을 수 없습니다." << std::endl;
		return retval;
	}

	auto set = map.find(key);

	if (set == map.end())
	{// 값이 없음
	}
	else
	{// 값이 있음
		retval = set->second;
	}

	return retval;
}
