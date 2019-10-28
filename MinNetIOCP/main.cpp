#include <iostream>
#include <conio.h>

#include "MinNetIOCP.h"
#include "MinNetCache.h"
#include "MinNetGameObject.h"
#include "FirstPersonController.h"

void main()
{
	MinNetCache::componentCache.insert(std::make_pair("FPSController", [](MinNetGameObject * object)
	{
		std::cout << "���ٿ����� �ּ� : " << object << std::endl;
		object->AddComponent<FirstPersonController>();
	}));

	MinNetIOCP * iocp = new MinNetIOCP();
	iocp->SetTickrate(20);

	iocp->StartServer();
	iocp->ServerLoop();


	_getch();
}