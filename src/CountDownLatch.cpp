class CountDownLatch{
public:
	explicit CountDownLatch (const int &count); // constructor
	void wait_();
	void countDown();

private:
	mutable MutexLock mutex_;
	condition cond_;
	int cnt;
};

explicit CountDownLatch::CountDownLatch(const int &count)
	: mutex_(), cond_(mutex_), cnt(count){ // keep in order!

}

void CountDownLatch::wait_(){
	MutexLockGuard lock_(mutex_);
	while(cnt > 0){
		cond_.wait();
	}
}

void CountDownLatch::countDown(){
	MutexLockGuard lock_(mutex_);
	if(--cnt == 0){
		cond_.notifyAll(); // why notifyAll() instead of notify() ?
	}
}
