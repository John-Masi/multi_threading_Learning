#include <atomic> 

class SpinLock{
	std::atomic_flag flag = ATOMIC_FLAG_INIT;

	public:
		void lock() {
			// Flag is set to lock
			while(flag.test_and_set(std::memory_order_acquire)){
				// busy-wait so the thread keeps trying the atomic LOCK operation while the flag is still false 
			}
		}

		void unlock() {
			// Flag is set to unlock 
			flag.clear(std::memory_order_release);
		}
};