#include "MinNetPool.h"
#include "MinNetRoom.h"


MinNetObjectPool<MinNetAcceptOverlapped> * MinNetPool::acceptOverlappedPool = nullptr;
MinNetObjectPool<MinNetCloseOverlapped> * MinNetPool::closeOverlappedPool = nullptr;
MinNetObjectPool<MinNetSendOverlapped> * MinNetPool::sendOverlappedPool = nullptr;
MinNetObjectPool<MinNetRecvOverlapped> * MinNetPool::recvOverlappedPool = nullptr;
MinNetObjectPool<MinNetUser> * MinNetPool::userPool = nullptr;
MinNetObjectPool<MinNetPacket> * MinNetPool::packetPool = nullptr;

MinNetPool::MinNetPool()
{
}

MinNetPool::~MinNetPool()
{
}

void MinNetPool::Init()
{
	acceptOverlappedPool = new MinNetObjectPool<MinNetAcceptOverlapped>();
	closeOverlappedPool = new MinNetObjectPool<MinNetCloseOverlapped>();
	sendOverlappedPool = new MinNetObjectPool<MinNetSendOverlapped>();
	recvOverlappedPool = new MinNetObjectPool<MinNetRecvOverlapped>();
	userPool = new MinNetObjectPool<MinNetUser>();
	packetPool = new MinNetObjectPool<MinNetPacket>();

	userPool->SetOnPush([](MinNetUser * user) {
		user->ID = -1;
		user->isConnected = false;
		user->last_ping = user->last_pong = user->ping = -1;
		user->sock = INVALID_SOCKET;
		user->ChangeRoom(nullptr);
		user->autoDeleteObjectList.clear();
		user->tcpBufferPosition = user->udpBufferPosition = 0;
		ZeroMemory(user->tcpBuffer, Defines::TEMPORARYBUFFERSIZE);
		ZeroMemory(user->udpBuffer, Defines::TEMPORARYBUFFERSIZE);
	});
	userPool->AddObject(100);


	acceptOverlappedPool->SetOnPush([](MinNetAcceptOverlapped * overlap) {
		ZeroMemory(overlap, sizeof(MinNetAcceptOverlapped));
		overlap->type = MinNetOverlapped::TYPE::ACCEPT;
		overlap->socket = INVALID_SOCKET;
		overlap->socket = WSASocket(PF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	});
	acceptOverlappedPool->AddObject(20);

	closeOverlappedPool->SetOnPush([](MinNetCloseOverlapped * overlap) {
		ZeroMemory(overlap, sizeof(MinNetCloseOverlapped));
		overlap->type = MinNetOverlapped::TYPE::CLOSE;
	});
	closeOverlappedPool->AddObject(20);

	sendOverlappedPool->SetConstructor([](MinNetSendOverlapped * overlap) {
		overlap->wsabuf.buf = new char[Defines::BUFFERSIZE];
		overlap->isTcp = true;
	});
	sendOverlappedPool->SetOnPush([](MinNetSendOverlapped * overlap) {
		ZeroMemory(overlap, sizeof(MinNetOverlapped));
		overlap->type = MinNetOverlapped::TYPE::SEND;
		ZeroMemory(overlap->wsabuf.buf, Defines::BUFFERSIZE);
		overlap->isTcp = true;
	});
	sendOverlappedPool->SetDestructor([](MinNetSendOverlapped * overlap) {
		delete[] overlap->wsabuf.buf;
	});

	sendOverlappedPool->AddObject(100);

	recvOverlappedPool->SetConstructor([](MinNetRecvOverlapped * overlap) {
		overlap->wsabuf.buf = new char[Defines::BUFFERSIZE];
		overlap->wsabuf.len = Defines::BUFFERSIZE;
		overlap->isTcp = true;
	});
	recvOverlappedPool->SetDestructor([](MinNetRecvOverlapped * overlap) {
		delete[] overlap->wsabuf.buf;
	});
	recvOverlappedPool->SetOnPush([](MinNetRecvOverlapped * overlap) {
		ZeroMemory(overlap, sizeof(MinNetOverlapped));
		overlap->type = MinNetOverlapped::TYPE::RECV;

		ZeroMemory(overlap->wsabuf.buf, Defines::BUFFERSIZE);
		overlap->wsabuf.len = Defines::BUFFERSIZE;
		overlap->isTcp = true;
	});
	recvOverlappedPool->AddObject(100);

	packetPool->SetOnPush([](MinNetPacket * packet) {
		packet->buffer_position = 0;
		packet->body_size = 0;
		ZeroMemory(packet->buffer, Defines::PACKETSIZE);
	});
	packetPool->AddObject(3000);
}
