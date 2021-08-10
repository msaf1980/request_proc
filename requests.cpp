#include <chrono>

using namespace std::chrono_literals;

#include "requests.hpp"

void TaskScheduler::Start() {
    bool running = false;
    if (this->running.compare_exchange_strong(running, true)) {
        for (int i = 0; i < this->nThreads; ++i) {
            std::thread thread(&TaskScheduler::worker, this, nullptr);
            threads.push_back(std::move(thread));
        }
    }
}

void TaskScheduler::Stop() {
    bool running = true;
    if (this->running.compare_exchange_strong(running, false)) {
        {
            std::unique_lock<std::mutex> lock(this->requests_lock);
            this->requests_cv.notify_all();
        }
        {
            std::unique_lock<std::mutex> lock(this->done_lock);
            this->done_cv.notify_all();
        }
        for (auto & thread : this->threads) {
            thread.join();
        }
        this->threads.resize(0);
    }
}

Request* TaskScheduler::GetRequest() throw() {
    while (1) {
        if (!this->running.load()) {
            return nullptr; // shutdown initiated
        }
        std::unique_lock<std::mutex> lock(this->done_lock);
        //this->done_cv.notify_all();
        auto req = this->done.front();
        if (req == nullptr) {
            if (this->done_cv.wait_for(lock,  100ms) == std::cv_status::timeout) {
                continue;
            }
            if (!this->running.load()) {
               return nullptr; // shutdown initiated
            }
            req = this->done.front();
        }
        this->done.pop();
        return req;
    }
}

void TaskScheduler::ProcessRequest(Request* request) throw() {
    if (!this->running.load()) {
        return;
    }
    std::unique_lock<std::mutex> lock(this->requests_lock);
    this->requests.push(request);
    this->requests_cv.notify_one();
}

void TaskScheduler::worker(void *p) {
    while (this->running.load()) {
        Request *req = nullptr;
        {
            std::unique_lock<std::mutex> lock(this->requests_lock);

            req = this->requests.front();
            if (req == nullptr) {
                this->requests_cv.wait(lock);
                if (!this->running.load()) {
                    return; // shutdown ititiated
                }
                req = this->requests.front();
                if (req == nullptr) {
                    continue;
                }
            }
            this->requests.pop();
        }

        req->func(req);

        {
            std::unique_lock<std::mutex> lock(this->done_lock);
            this->done.push(req);
            this->done_cv.notify_one();
        }
    } 
}

int TaskScheduler::RequestsQueued() {
    std::unique_lock<std::mutex> lock(this->requests_lock);
    return this->requests.size();
}