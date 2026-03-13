#include<iostream>
#include<dirent.h>

int main(int argc,char* argv[]){
	DIR* p = opendir(argv[1]);
	if(p == nullptr) {
		perror("error!");
		exit(1);
	}

	struct dirent* p2;
	while((p2 = readdir(p)) != NULL ){
		if(p2->d_name[0] == '.') continue;
		std::cout << p2->d_name << "\t";
	}
	std::cout << std::endl;

	int ret = closedir(p);
	if(ret != 0){
		perror("error!");
		exit(1);
	}
}
