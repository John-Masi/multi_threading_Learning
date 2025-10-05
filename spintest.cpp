#include <thread>
#include <vector>
#include <iostream>
#include "SpinLock.h"

SpinLock sp;

static int g{0};

void add() {
	sp.lock();
	g++;
	sp.unlock();
	std::cout << g << '\n';
}

int main() {
	std::vector<std::thread> tv;

	for(int i = 0; i < 100; i++) {
		tv.emplace_back(std::thread(add));
	}

	for(auto& t: tv){
		t.join();
	}
}