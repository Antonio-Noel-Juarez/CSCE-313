#include "pool.h"
#include <mutex>
#include <iostream>

Task::Task() = default;
Task::~Task() = default;

ThreadPool::ThreadPool(int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(new std::thread(&ThreadPool::run_thread, this));
    }
}

ThreadPool::~ThreadPool() {
    for (std::thread *t: threads) {
        delete t;
    }
    threads.clear();

    for (Task *q: queue) {
        delete q;
    }
    queue.clear();
}

void ThreadPool::SubmitTask(const std::string &name, Task *task) {
    //TODO: Add task to queue, make sure to lock the queue
    mtx.lock();
    
    if (done) {
        std::cout << "Cannot added task to queue\n";
        mtx.unlock();
        return;
    }
    
    task->name = name;
    queue.push_back(task);
    num_tasks_unserviced++;
    std::cout << "Added task: " << name << std::endl;
    mtx.unlock();
}

void ThreadPool::run_thread() {
    while (true) {
        Task* task = nullptr;
        
        mtx.lock();
            
        //TODO1: if done and no tasks left, break
        if (done && num_tasks_unserviced == 0) {
            mtx.unlock();
            break;
        }
        
        //TODO2: if no tasks left, continue
        if (num_tasks_unserviced == 0 || queue.empty()) {
            mtx.unlock();
            continue;
        }

        //TODO3: get task from queue, remove it from queue, and run it
        task = queue.front();
        queue.erase(queue.begin());
        mtx.unlock();
        

        mtx.lock();
        if (task != nullptr && !task->is_running()) {
            task->running = true;
            std::cout << "Started task: " << task->name << "\n";
            mtx.unlock();
            // Run the task outside the lock
            task->Run();
            mtx.lock();
            task->running = false;
            std::cout << "Finished task: " << task->name << "\n";
            
            num_tasks_unserviced--;
            
            //TODO4: delete task
            delete task;
        }
        mtx.unlock();
    }
}

// Remove Task t from queue if it's there
void ThreadPool::remove_task(Task *t) {
    mtx.lock();
    for (auto it = queue.begin(); it != queue.end();) {
        if (*it == t) {
            queue.erase(it);
            mtx.unlock();
            return;
        }
        ++it;
    }
    mtx.unlock();
}

void ThreadPool::Stop() {
    //TODO: Delete threads, but remember to wait for them to finish first
    std::cout << "Called Stop()" << std::endl;
    while (num_tasks_unserviced > 0);
    std::cout << "Stopping thread" << std::endl;
    mtx.lock();
    this->done = true;
    mtx.unlock();
    
    for (std::thread *t : threads) {
        if (t->joinable()) {
            t->join();
        }
        delete t;
    }

    threads.clear();
}
