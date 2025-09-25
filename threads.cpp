#include <thread>
#include <iostream>
#include <stdlib.h>
#include <string>
#include <mutex>
#include <condition_variable>
#include <future>

// std::thread acts like std::unique_ptr can only use copy semantics on it 

// Shared memory 
int count = 0;
int count_2 = 0;

// Two mutexs for each shared count
std::mutex mtx;
std::condition_variable cv;
bool ready = false; 

void add_1() {
	std::unique_lock<std::mutex> l(mtx); // Exclusive ownership can use move sematics 
	for(int i=0; i < 10000; i++){
		count++;
	}
};

void add_2() {
	std::unique_lock<std::mutex> l(mtx);
	for(int i = 0; i < 1000; i++) {
		count_2++;
	}
};
	
void worker(){
	std::unique_lock<std::mutex> l(mtx);
	cv.wait(l,[] {return ready; }); // unlocks mutex and blocks the current thread, then relocks it when ready
	count += count_2;
	std::cout << count;
};

// When using mutexs with objects, we only need one mutex to guard the object and lock when we work with data memebers inside of the object
class BankAccount {
	private:
		int balance;
		std::string name;
		std::string acc_num;

		std::lock_guard<std::mutex> doLock() {
			return std::lock_guard<std::mutex> (mtx); // have to return lock if we just set the lock noramlly when function is out of scope it will unlock the lock before we can access our shared data
		};

	public:
		mutable std::mutex mtx;

		BankAccount(int &balance,std::string username,std::string account) {
			auto l = doLock();
		};

		void setBalance(int &b) {
			auto l = doLock();
			this->balance = b;
		};

		int getBalance() {
			auto l = doLock();
			return this->balance;
		};

		// using scoped transfer to have a thread safe transfer between bank accounts
		// Avoids dead locking if we accidently lock in the wrong order 
		void transfer(BankAccount &ba,int amt) {
			std::scoped_lock(this->mtx,ba.mtx);
			this->balance -= ba.balance;
			ba.balance += amt;

		};
};


int main(){

	std::thread t(add_1);
	std::thread t1(add_2);
	std::thread t2(worker);


	t.join();
	t1.join();

	// Block that will set the bool to ready for cv.wait
	{	
		std::lock_guard<std::mutex> l(mtx);
		ready = true;	
	}

	cv.notify_one(); // Notifies one worker thread 

	t2.join();

	return 0;
}