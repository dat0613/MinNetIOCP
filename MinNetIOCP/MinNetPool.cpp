#include "MinNetPool.h"
#include "MinNetRoom.h"


MinNetObjectPool<MinNetRoom> * MinNetPool::roomPool = nullptr;
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
	roomPool = new MinNetObjectPool<MinNetRoom>();
	acceptOverlappedPool = new MinNetObjectPool<MinNetAcceptOverlapped>();
	closeOverlappedPool = new MinNetObjectPool<MinNetCloseOverlapped>();
	sendOverlappedPool = new MinNetObjectPool<MinNetSendOverlapped>();
	recvOverlappedPool = new MinNetObjectPool<MinNetRecvOverlapped>();
	userPool = new MinNetObjectPool<MinNetUser>();
	packetPool = new MinNetObjectPool<MinNetPacket>();

	roomPool->SetOnPush([](MinNetRoom * room) {
		room->RemoveUsers();
		room->RemoveObjects();
		room->SetManager(nullptr);

	});
	roomPool->AddObject(10);

	userPool->SetOnPush([](MinNetUser * user) {
		user->ID = -1;
		user->isConnected = false;
		user->last_ping = user->last_pong = user->ping = -1;
		user->sock = INVALID_SOCKET;
		user->ChangeRoom(nullptr);
		user->autoDeleteObjectList.clear();
		user->buffer_position = 0;
		ZeroMemory(user->temporary_buffer, 2048);
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
		overlap->wsabuf.buf = new char[1024];
	});
	sendOverlappedPool->SetOnPush([](MinNetSendOverlapped * overlap) {
		ZeroMemory(overlap, sizeof(MinNetOverlapped));
		overlap->type = MinNetOverlapped::TYPE::SEND;
		ZeroMemory(overlap->wsabuf.buf, 1024);
	});
	sendOverlappedPool->AddObject(100);

	recvOverlappedPool->SetConstructor([](MinNetRecvOverlapped * overlap) {
		overlap->wsabuf.buf = new char[1024];
		overlap->wsabuf.len = 1024;
	});
	recvOverlappedPool->SetDestructor([](MinNetRecvOverlapped * overlap) {
		delete[] overlap->wsabuf.buf;
	});
	recvOverlappedPool->SetOnPush([](MinNetRecvOverlapped * overlap) {
		ZeroMemory(overlap, sizeof(MinNetOverlapped));
		overlap->type = MinNetOverlapped::TYPE::RECV;

		ZeroMemory(overlap->wsabuf.buf, 1024);
		overlap->wsabuf.len = 1024;
	});
	recvOverlappedPool->AddObject(100);

	packetPool->SetOnPush([](MinNetPacket * packet) {
		packet->buffer_position = 0;
		packet->body_size = 0;
		ZeroMemory(packet->buffer, 1024);
	});
	packetPool->AddObject(10);
}
