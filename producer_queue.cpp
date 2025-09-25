#include <thread>
#include <queue>
#include <condition_variable>
#include <iostream>
#include <chrono>
#include <mutex>

// Producer consumer queue
class PC_q {
	public:
		std::queue<int> q;
		mutable std::mutex mtx;
		std::condition_variable cv;

		int pop_q() {
			std::unique_lock<std::mutex> l(this->mtx);
			// Signal if its empty or not we only pop from the top if the predicate returns false 
			// Use predicates to prevent spurious wake ups(Thread will procceed no matter what)
			cv.wait(l,[this]{return !q.empty();});
			int value = std::move(q.front());
			q.pop();
			return value;
		};

		void push_q(int v){	
			{
				// Pushing onto the queue
				std::lock_guard<std::mutex> l(mtx);
				q.push(v);
			}

			// Notify the queue to wake up the thread 
			cv.notify_one();
		};

		bool empty() {
			std::lock_guard<std::mutex> l(this->mtx);
			return q.empty();
		};

	private:
};

void producer(PC_q& pq) {
	for(int i = 0; i < 3; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(400));
		pq.push_q(i);
	}
}

void consumer(PC_q& pq,int v){
	for(int i = 0; i <= 3; i++) {
		int v = pq.pop_q();
		std::cout << "Consumer: " << v << " from the queue" << '\n';
	}
}


int main() {

	PC_q q;

	std::thread p(producer,std::ref(q));
	std::thread c(consumer,std::ref(q),1);

	p.join();
	c.join();

	return 0;
}