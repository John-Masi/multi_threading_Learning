#include <future>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

class Foo {
	public:
		int* data;

		Foo() {
			this->data = new int(0);
		};

		Foo(const Foo &f) {
			this->data = new int(*f.data);
		};

		Foo(Foo &&f) noexcept {
			this->data = f.data;
			f.data = nullptr;
		};

		~Foo() {
			delete data;
		};
};

int count = 0;
std::mutex mtx;

int add() {
	for(int i = 0; i < 5; i++) {
		std::lock_guard<std::mutex> l(mtx);
		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << count << '\n';
		count++;
	}
	return count;
};

// Basic worker function
void addPromise(std::promise<int> p) {
	auto start = std::chrono::high_resolution_clock::now(); // Timer start 
	for(int i = 0; i < 100; i++) {
		std::lock_guard<std::mutex> l(mtx);
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		std::cout << count << '\n'; 
		count++;
	}
	auto end = std::chrono::high_resolution_clock::now(); // Timer end 
	std::chrono::duration<double> timer = end - start;
	std::cout << "Time elapsed: " << timer.count() << '\n';
	p.set_value(count);
};

int main() {

	// Acts as a producer and stores results that will be ready later
	std::promise<int> pr;
	// This will get the result set by the promise( or other providers ), it acts as a consumer and waits for async result
	std::future<int> fut_3 = pr.get_future();
	std::thread t(addPromise,std::move(pr)); // Converting the promise to an rvalue to take owner ship
	std::shared_future<int> s_fut = fut_3.share(); // Call get multiple times without consuming our value we get from future, takes owner ship of the og future


	// Creating threads for the above result we get from the promise/thread 
	std::vector<std::thread> t_vec; 
	for(int i = 0; i < 3; i++) {
		t_vec.emplace_back([i,s_fut] {std::cout << s_fut.get() + i << '\n'; });
	}

	// Join our producer thread
	t.join();
	for(auto &c: t_vec) {
		// Joining all consumerthreads that are storing the result from the producer
		c.join();
	}

	// Future has two parts the std::future on the stack, that stores a pointer to the shared state(8 bytes,
	// and a heap-allocated share state which holds the result of T, a mutex/condition var for waiting, Flags.
	//std::future<Foo> fut = std::async([] {return Foo(); });
	std::future<int> fut_2 = std::async(std::launch::deferred, add); // this will automatically create a thread for add 

	return 0;
}