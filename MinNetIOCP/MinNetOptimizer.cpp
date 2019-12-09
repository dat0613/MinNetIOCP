#include "MinNetOptimizer.h"

MinNetSpinLock::MinNetSpinLock()
{

}

MinNetSpinLock::~MinNetSpinLock()
{
}

void MinNetSpinLock::lock()
{
	while (locker.test_and_set(std::memory_order_acquire));
}

void MinNetSpinLock::unlock()
{
	locker.clear(std::memory_order_release);
}