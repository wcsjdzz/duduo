#include <vector>
using namespace std;
bool running = false;

void excuteTask(){
	while(running){
		// excuting task ...
	}
}

int main(){
	muduo::thread t(excuteTask);
	t.start(); // wrong!!
	running  = true;
	vector<int> V{1, 0};
	V.push_back(2);
}


