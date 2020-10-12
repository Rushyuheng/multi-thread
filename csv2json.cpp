#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <chrono>
#include <vector>

using namespace std;

int main(int argc,char *argv[]){
	vector<string> inputVector;
	int numberOfThread = atoi(argv[1]);
	vector<thread> mythread(numberOfThread); //memory for threads,no initialization yet

    ifstream infile("./input.csv");
    if(!infile){
        cerr << "fail to open file : input.csv"  <<endl;
        exit(1);
    }
	
	string readline;
	while(getline(infile,readline)){
		inputVector.push_back(readline);
	}
	
	#ifdef debug
		//unit test for read data in
		for(int i = 0;i < inputVector.size() ;++i){
			cout << inputVector.at(i) << endl;
		}
	#endif

	return 0;
}
