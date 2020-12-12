class invenctory{
public:
	void add_(request *r);
	void remove_(request *r);

	void print_all() const ;

private:
	mutable MutexLock mutex_; // mutex
	std::unordered_set<request*> requests_; // all requests are saved in here;
};

void invenctory::add_(request *r){
	MutexLockGuard lock_(mutex_); // RAII mode
	requests_.insert(r);
}

void invenctory::remove_(request *r){
	MutexLockGuard lock_(mutex_);
	requests_.erase(r);
}

void invenctory::print_all() const {
	MutexLockGuard lock_(mutex_);
	for(auto itr = requests_.begin(); itr != requests_.end(); ++itr){
		(*itr)->print_();
	}
}

class request{
public:
	void print_() const ;
	void process();
	~request();

private:
	mutable MutexLock mutex_;
};

invenctory g_invenctory;

void request::process() {
	MutexLockGuard lock_(mutex_); // could cause dead lock!
	g_invenctory.add_(this);
	// processing ...
}

void request::print_() const {
	MutexLockGuard lock_(mutex_); // 
	/* to be added ... */
}

requst::~request() {
	MutexLock lock_(mutex);
	g_invenctory.remove_(this); // could cause dead lock!
}

