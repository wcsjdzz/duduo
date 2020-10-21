muduo::MutexLock mutex_;
muduo::condition cond(mutex_);
std::queue<int> Q;

int dequeue(){
	// out of multiple threads waiting here, 
	// only one thread could be waked up;
	muduo::MutexLockGuard lock_(mutex_); // RAII mode
	while (Q.empty()){
		cond.wait(); // unlock the mutex automatically;
	}
	assert(!Q.empty());
	int ret = Q.front();
	Q.pop();
	return ret;
}

void enqueue(int num){
	muduo::MutexLockGuard lock_(mutex_); // RAII mode
	Q.push(num);
	cond.notify(); // resoure is available now;
}

