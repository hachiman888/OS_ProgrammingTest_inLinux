#include <unistd.h>
#include <iostream>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <string.h>

void sys_err(const char* msg){
    perror(msg);
    exit(-1);
}

int main(){
    int fd = open("ForTest.txt",O_RDWR|O_CREAT|O_TRUNC,0664);

    if(fd == -1) {
        sys_err("open error!");
    }

    ftruncate64(fd,20);
    int len = lseek64(fd,0,SEEK_END);
    char* p = static_cast<char*>(mmap(NULL,len,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0));
    close(fd);

    if(p == MAP_FAILED){
        sys_err("mmap error!");
    }

    memcpy(p,"hello world",12);
    std::cout << "已写入共享内存映射中" << std::endl;

    int ret = munmap(p,11);
    if(ret == -1){
        sys_err("munmap error!");
    }
}