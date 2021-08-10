#include <cstdio>
#include <chrono>

using namespace std::chrono_literals;

#include "requests.hpp"

const int NumberOfThreads = 2;

// Основной поток должен:

// 1) Запустить несколько рабочих потоков (NumberOfThreads).
// 2) Класть в одну очередь заданий задачи до тех пор, пока GetRequest() не вернёт nullptr.
// 3) Корректно остановить рабочие потоки. Они должны доделать текущий ProcessRequest, если он имеется, и остановиться. 
//    Если имеются необработанные задания, не обращать на них внимания.
// 4) Завершить программу.

// Рабочий поток должен:

// 1) Обрабатывать поступающие через очередь запросы с помощью ProcessRequest.
// 2) Завершиться, как только основной поток ему это скомандует.

// Вызовы GetRequest() и ProcessRequest() могут работать долго.

// возвращает nullptr если нужно завершить процесс, либо указатель на память,
// которую в дальнейшем требуется удалить
// !! Перенесенно в класс TaskScheduler
// Request* GetRequest() throw();

// обрабатывает запрос, но память не удаляет
// !! Перенесенно в класс TaskScheduler
// void ProcessRequest(Request* request) throw();

void task(Request *req) {
    // Use C-style printf for consistent output, for C++20 replace with std::format
    //std::cout << "PROCESS request " << req->id << std::endl;
    std::printf("PROCESS request %d\n", req->id);
    std::this_thread::sleep_for(1s);
    req->status = SUCCESS;
}

int main() {
    TaskScheduler scheduler(NumberOfThreads);
    scheduler.Start();
    for (int i = 1; i <= 10; i++) {
        Request *req = new Request(task, i);
        scheduler.ProcessRequest(req);
    }

    for (int i = 0; i < 2; i++) {
        Request *req = scheduler.GetRequest();
        if (req != nullptr) {
            //std::cout << "GET request " << req->id << std::endl;
            std::printf("GET request %d WITH status %d\n", req->id, req->status);
            delete req;
        }
    }

    scheduler.Stop();
    std::printf("UNPROCESSED REQUESTS: %d\n", scheduler.RequestsQueued());
}
