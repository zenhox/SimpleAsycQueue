#include<iostream>
#include<thread>
#include<ctime>
#include<cstdlib>
#include<vector>
#include"AsycQueue.h"
using namespace std;

AsycQueue<int> q;
const int NUM_OF_PRODUCER = 3;
const int NUM_OF_CONSUMER = 5;


void produce() {
	while (true) {
		int num_random = 1 + rand() % 10;
		q.push(num_random);
		chrono::milliseconds duration(500 + rand()%2000);
		this_thread::sleep_for(duration);
	}
}

void consume() {
	while (true) {
		int val = q.wait_pop();
		chrono::milliseconds duration(500 + rand() % 2000);
		this_thread::sleep_for(duration);
	}
}

int main(void) {
	srand(time(0));
	vector<thread> producers, consumers;
	for (int i = 0; i < NUM_OF_PRODUCER; ++i) {
		thread th = thread(produce);
		producers.push_back(move(th));  // must move, thread can not copy.
	}
	for (int i = 0; i < NUM_OF_CONSUMER; ++i) {
		thread th = thread(consume);
		consumers.push_back(move(th));
	}
	for (auto &th : producers) {
		th.join();
	}
	for (auto &th : consumers) {
		th.join();
	}
	return 0;
}