template <typename T>
class singleton{
public:
	T &instantiate(){
		// ensure only one instance is built;
		pthread_once(&ponce_, &singleton::new_);
		return *value_;
	}


private:
	singleton();
	~singleton();

	void new_(){
		value = new T ();
	}

	static pthread_once_t ponce_;
	static T *value_;
};

template <typename T>
pthread_once_t singleton<T>::ponce_ = PTHREAD_ONCE_INIT;

template <typename T>
T * singleton<T>::value_ = nullptr;
