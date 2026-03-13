#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<cerrno>
#include<cstring>
using namespace std;

int main(){
	int fd = 0;
	fd = open("./dict1.txt",O_RDONLY);
	cout << "fd = " << fd <<"\t " << "errno:" << errno << ":"
	       	<< strerror(errno) << endl;
	close(fd);
}

