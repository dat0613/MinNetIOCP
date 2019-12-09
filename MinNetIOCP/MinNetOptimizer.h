#pragma once
#include <atomic>
#include <functional>
#include <queue>

class MinNetSpinLock
{
public:
	MinNetSpinLock();
	~MinNetSpinLock();
	void lock();
	void unlock();

private:
	std::atomic_flag locker = ATOMIC_FLAG_INIT;
};

template <class T>
class MinNetMemoryPool
{
public:

private:

};

template <class T>
class MinNetObjectPool
{
public:
	MinNetObjectPool()
	{

	}

	~MinNetObjectPool()
	{
		while (!pool.empty())
		{
			T* obj = pool.front();
			pool.pop();
			if (destructor != nullptr)
				destructor(obj);
			delete obj;
		}
	}

	void SetConstructor(std::function<void(T *)> constructor)
	{
		this->constructor = constructor;
	}

	void SetDestructor(std::function<void(T *)> destructor)
	{
		this->destructor = destructor;
	}

	void SetOnPush(std::function<void(T *)> onPush)
	{
		this->onPush = onPush;
	}

	void AddObject(int size)
	{
		for (int i = 0; i < size; i++)
			push(CreateNewObject());
	}

	int GetSize()
	{
		spinLock.lock();
		int size = pool.size();
		spinLock.unlock();

		return size;
	}

	T* pop()
	{
		T* retval = nullptr;

		spinLock.lock();
		if (pool.empty())
		{// 풀이 비어있음
			std::cout << typeid(T).name() << " 풀의 객체가 고갈되어 새로운 객체를 생성합니다" << std::endl;
			retval = CreateNewObject();
		}
		else
		{
			T* obj = pool.front();
			pool.pop();
			retval = obj;
		}
		spinLock.unlock();
		
		return retval;
	}

	void push(T* obj)
	{
		if (onPush != nullptr)
		{
			onPush(obj);
		}

		spinLock.lock();
		pool.push(obj);
		spinLock.unlock();
	}

private:
	std::queue<T*> pool;
	std::function<void(T*)> constructor = nullptr;
	std::function<void(T*)> destructor = nullptr;
	std::function<void(T*)> onPush = nullptr;

	MinNetSpinLock spinLock;

	T* CreateNewObject()
	{
		T* obj = new T();

		if (constructor != nullptr)
			constructor(obj);

		return obj;
	}
};
