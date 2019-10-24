#include <iostream>
#include <conio.h>

#include "MinNetIOCP.h"

void main()
{
	MinNetIOCP * iocp = new MinNetIOCP();
	iocp->SetTickrate(20);

	iocp->StartServer();
	iocp->ServerLoop();

	_getch();
}