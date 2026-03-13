#include<iostream>
#include<sys/stat.h>
#include<unistd.h>

int main(){
	struct stat buf;
	int ret = stat("test.txt",&buf);
	if(ret == -1){
		perror("error!");
		exit(1);
	}
	if(S_ISDIR(buf.st_mode)){
		std::cout << "is a dictionary." << std::endl;
	}
	else std::cout << "not a dictionary" << std::endl;
}
