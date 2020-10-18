#include <iostream>
#include <cstring>
#include <thread>
#include <fstream>
#include <chrono>
#include <vector>
#include <sstream>

using namespace std;

void StringSeparation(int StartIndex, int endIndex,const vector<string> &input, vector<vector<int>> &result){
	string delimiter = "|";
	string token,s;
	size_t pos;
	int j = 0;
	for(int i = StartIndex;i < endIndex;++i){
		s = input.at(i);
		pos = 0;
		j = 0;
		while ((pos = s.find(delimiter)) != string::npos) {
    		token = s.substr(0, pos);//find substring
			result.at(i).at(j) = stoi(token);
    		s.erase(0, pos + delimiter.length());//delete substring and delimiter 
			++j;
		}
		result.at(i).at(j) = stoi(s);//last token
	}	
}


int main(int argc,char *argv[]){
	chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();//timer start    	
	vector<string> inputVector;
	int numberOfThread = atoi(argv[1]);
	vector<thread> mythread(numberOfThread); //memory for threads,not initialize yet
	
	cout << "start reading data" << endl;	
	//read csv file
    ifstream infile("./input.csv");
    if(!infile){
        cerr << "fail to open file : input.csv"  <<endl;
        exit(1);
    }

	string readline;
	while(getline(infile,readline)){
		inputVector.push_back(readline);
	}
	infile.close();
	chrono::steady_clock::time_point rdata = std::chrono::steady_clock::now();   	
	cout << "finish reading data at: "<< chrono::duration_cast<chrono::milliseconds>(rdata - begin).count() << "ms"<< endl;
	cout << "start string tokenizing" << endl;
	//mutithread string tokenize
	const int len = inputVector.size();	
	
	int averageThreadCap = len / numberOfThread;//average number of string for one thread
	int remainer = len % numberOfThread;
	int startIndex = 0,EndIndex,threadCap;
	vector<vector<int> > result(len,vector<int>(20));//result table after threads separation

	for(int i = 0;i < numberOfThread;++i){
		//load balacing
		threadCap = averageThreadCap;
		if(remainer > 0){
			++threadCap;
			--remainer;
		}

		EndIndex = startIndex + threadCap;
		mythread.at(i) = thread(StringSeparation,startIndex,EndIndex,ref(inputVector),ref(result));
		startIndex = EndIndex;//update for next iteration
	}
	
	for(int i = 0;i < numberOfThread;++i){
		mythread.at(i).join();//join all thread before write file
	}
	chrono::steady_clock::time_point stoken = std::chrono::steady_clock::now();   	
	cout << "finish string tokenizing at: "<< chrono::duration_cast<chrono::milliseconds>(stoken - begin).count() << "ms"<< endl;
	cout << "start writing data" << endl;
	//write file in json form
	ofstream outfile("./output.json");
	outfile <<"[\n";
	for(int i = 0;i < len - 1;++i){
		outfile <<"  {\n";
		for(int j = 0;j < 19;++j){
			outfile <<"    \"col_" << j + 1 << "\":" << result.at(i).at(j) << ",\n";
		}
		outfile <<"    \"col_20\": " << result.at(i).at(19) << "\n";//last element don't need comma
		outfile <<"  },\n";
	}
	outfile <<"  {\n";
	for(int j = 0;j < 19;++j){
		outfile <<"    \"col_" << j + 1 << "\":" << result.at(len - 1).at(j) << ",\n";
	}
	outfile <<"    \"col_20\": " << result.at(len - 1).at(19) << "\n";//last element don't need comma
	outfile <<"  }\n";//last list don't need comma
	outfile <<"]\n";

	outfile.close();	
	cout << "finish writing data" << endl;
	chrono::steady_clock::time_point end = std::chrono::steady_clock::now();	
	cout << "Elapsed time in milliseconds : "<< chrono::duration_cast<chrono::milliseconds>(end - begin).count()<< " ms" << endl;  
	return 0;
}
