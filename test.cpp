#include <cstdio>
#include <iostream>
#include <fstream>
using namespace std;


int main(){
	ofstream outfile("testout.json");
	outfile << "[" << endl;
	outfile << "  {"<<endl;
	outfile << "    \"col_1\": 5,"<<endl;
	outfile << "    \"col_2\": 10"<<endl;
	outfile << "  },"<<endl;
	outfile << "  {"<<endl;
	outfile << "    \"col_1\": 5,"<<endl;
	outfile << "    \"col_2\": 10"<<endl;//last element don't need comma
	outfile << "  }"<<endl;//last element don't need comma
	outfile << "]" << endl;
	outfile.close();
	return 0;
}



