#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <string>

class HandleProcessIO{
public:
    HandleProcessIO(std::string file_name,int size):
    data_(nullptr),fd_(-1){
        fd_ = open64(file_name.c_str(),O_RDWR|O_CREAT|O_TRUNC,0664);
        if(fd_ == -1){
            throw std::runtime_error("open error!");
        }
        
        ftruncate64(fd_,size);
        size_ = lseek64(fd_,0,SEEK_END);
        data_ = static_cast<char*>(mmap64(NULL,size_,PROT_READ|PROT_WRITE,MAP_SHARED,fd_,0));
        if(data_ == nullptr){
            throw std::runtime_error("mmap error!");
        }
    }

    void standardWrite(const std::string msg){
        if(msg.size() >= size_){
            throw std::out_of_range("Write error!");
        }
        std::copy(msg.begin(),msg.end(),data_);
        data_[msg.size()] = '\0';
    }

    std::string standardRead() const{
        return std::string(data_);
    }

    char* data(){return data_;}
    int size(){return size_;}

    ~HandleProcessIO(){
        if(fd_ != -1){
            close(fd_);
        }
        if(data_ != nullptr && data_ != MAP_FAILED ){
            munmap(data_,size_);
        }
    }

private:
    char* data_;
    int size_;
    int fd_;
};

int main(){
    try{
        HandleProcessIO test("testForModern",4096);
        while(1) test.standardWrite("hello world!");
    }catch(const std::exception& e){
        std::cerr << e.what() << std::endl;
        return -1;
    }
}