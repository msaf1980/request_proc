cmake_minimum_required(VERSION 3.5)
project(request_proc)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(request_proc main.cpp requests.cpp)
target_link_libraries(request_proc Threads::Threads)
