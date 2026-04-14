cmake_minimum_required(VERSION 3.10)
project(AsioTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 1. 找 Boost
find_package(Boost REQUIRED COMPONENTS system)
# 2. 找线程库 (这一行是解决你报错的关键！)
find_package(Threads REQUIRED)

add_executable(server echo_Server.cpp)

# 3. 链接
# 注意这里是 Boost::system 和 Threads::Threads
target_link_libraries(server PRIVATE 
    Boost::system 
    Threads::Threads
)

add_executable(client client.cpp)
target_link_libraries(client PRIVATE 
    Boost::system 
    Threads::Threads
)