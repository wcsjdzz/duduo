using Foo = int;
using FooList = std::vector<Foo>;
using FooListPtr = std::shared_ptr<FooList>;

muduo::MutexLock mutex_;
FooListPtr g_ptr;

void traverse(){
	FooListPtr tmp;
	{
		MutexLockGuard lock_(mutex_);
		tmp = g_ptr; // counter added;
	}
	for(auto itr = tmp->begin(); itr != tmp->end(); ++itr){
		// traverse it...
	}
}

void post(Foo &item_){
	MutexLockGuard lock_(mutex_);
	if(!g_ptr.unique()){
		g_ptr.reset(new FooList (*g_ptr));
		assert(g_ptr.unique());
	}
	g_ptr->push_back(item_);
}


void post_wrong_version1(Foo &item_){
	MutexLockGuard lock_(mutex_);
	g_ptr->push_back(item_); // what is some other threads are travers()ING ?
}

void post_wrong_version2(Foo &item_){
	FooListPtr new_ptr(new FooList (*g_ptr));
	new_ptr->push_back(item_); // what if there are multiple threads posting?
				  // That means there are multiple 'new_ptr' exists;
	MutexLockGuard lock_(mutex_);
	g_ptr = new_ptr;
}
