#include <mutex>
#include <condition_variable>
#include <atomic>

class BlockMutex {
	
	// state for if our lock is free or not 
	std::atomic<bool> l{false};
	std::condition_variable cv;
	std::mutex mtx; 

	public:
		void lock() {
			bool exp = false;

			// If this this section is false then another thread owns our lock
			if(l.compare_exhange_strong(exp,true,std::memory_order_acquire)){
				//
				return;
			}

			std::unique_lock<std::mutex> ul(mtx);

			// Checking if we did get the lock 
			while(l.exchange(true,std::memory_order_acquire)){
				// Put current thread to sleep and make it stand still
				cv.wait(ul);
			}
		}

		void unlock() {
			l.store(false,std::memory_order_release);
			std::scoped_lock<std::mutex> ul(mtx);
			// Wake up the thread that is currently asleep at the moment
			cv.notify_one();
		}

};