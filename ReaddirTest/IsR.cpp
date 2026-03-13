#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dirent.h>
#include <string>

void isFile(const char* name);
void read_dir(const char* dir);

void isFile(const char* name){
	int ret = 0;
	struct stat buf;
	
	ret = stat(name,&buf);
	if(ret == -1){
		perror("stat error!");
		return ;
	}
	if(S_ISDIR(buf.st_mode)){
		read_dir(name);
	}//如果不是目录，那么直接输出文件名字和大小
	std::cout << name << "\t" << buf.st_size << std::endl;
}

void read_dir(const char* dir){
	DIR* dp;
	struct dirent* sdp;
	char path[256];

	dp = opendir(dir);
	if(dp == nullptr){
		perror("opendir error!");
		return;
	}

	while(sdp = readdir(dp)){
		std::string filename(sdp->d_name);
		if(filename == "." || filename  == "..") {
			continue;}
		
		//目录项本身只有名字，不是路径，所以要拼接路径
		sprintf(path,"%s/%s",dir,sdp->d_name);
		isFile(path);
	}
	closedir(dp);
	return ;
}

int main(int argc,char* argv[]){
	if(argc == 1){
		isFile(".");
	}else{
		isFile(argv[1]);
	}
}
