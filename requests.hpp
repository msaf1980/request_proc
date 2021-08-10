#ifndef __REQUESTS_HPP__
#define __REQUESTS_HPP__

#include <atomic>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>

enum RequestStatus { SCHEDULED, SUCCESS, FAILED };

class Request
{
public:
    Request(std::function<void(Request*)> func, int id):
        func(func), id(id), status(SCHEDULED) {};
    std::function<void(Request*)> func;
    int id;
    RequestStatus status;
};

class TaskScheduler {
public:
    TaskScheduler(int nThreads): running(false) {
        if (nThreads < 1) {
            throw std::logic_error("invalid number of threads");
        }
        this->nThreads = nThreads;
    };

    void Start();
    void Stop();
    Request* GetRequest() throw();
    void ProcessRequest(Request* request) throw();
    int RequestsQueued();
private:
    int nThreads;
    std::atomic_bool running;
    std::vector<std::thread> threads;

    std::mutex requests_lock;
    std::condition_variable requests_cv;
    std::queue<Request*> requests;

    std::mutex done_lock;
    std::condition_variable done_cv;
    std::queue<Request*> done;

    void worker(void *p);
};

#endif /* __REQUESTS_HPP__ */
