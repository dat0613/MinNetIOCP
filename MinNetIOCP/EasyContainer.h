#pragma once

#include <map>
#include <string>
#include <iostream>

class EasyContainer
{
public:
	EasyContainer();
	~EasyContainer();

	void SetValue(std::string key, int value);
	void SetValue(std::string key, float value);
	void SetValue(std::string key, bool value);
	void SetValue(std::string key, std::string value);

	int GetValueInt(std::string key);
	float GetValueFloat(std::string key);
	bool GetValueBool(std::string key);
	std::string GetValueString(std::string key);

private:

	std::map<std::string, std::string> map;
};

