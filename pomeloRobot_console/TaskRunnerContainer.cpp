//
//  TaskRunnerContainer.cpp
//  pomeloRobot_console
//
//  Created by qiong on 14-2-24.
//  Copyright (c) 2014年 qiong. All rights reserved.
//

#include "TaskRunnerContainer.h"
#include "TaskRunner.h"
#include <iostream>
#include <mutex>
#include <unistd.h>
#include <thread>
#include <random>
std::default_random_engine rg;
std::uniform_int_distribution<int> uid(1,4);
int TaskRunnerContainer::_totalReqs = 0;
TaskRunnerContainer::TaskRunnerContainer(const char* addr, int port, int clients_nums)
:_addr(addr),_port(port),_client_nums(clients_nums),_current_client_id(0),_container()
{
    char buf[50];
    sprintf(buf, "stat_%d.txt", getpid());
    _fout.open(buf);
//    sprintf(buf, "stat_%d_sum.txt", getpid());
//    _fout_summary.open(buf);
//    TaskRunner::printHeader(std::cout);
    TaskRunner::printHeader(_fout);
}
TaskRunnerContainer::~TaskRunnerContainer()
{
    _fout.close();
//    _fout_summary.close();
}

void TaskRunnerContainer::release()
{
}

std::mutex id_increase_mutex;
std::mutex queue_mutex;
#include <chrono>
using namespace std::chrono;

void TaskRunnerContainer::startRun(int numOfThreads)
{
    product();
    vector<thread> threads(_client_nums);
    for (int i = 0; i < _client_nums; ++i) {
        threads[i] = thread(&TaskRunnerContainer::consume, this);
    }
//    thread statThread(&TaskRunnerContainer::statEverySec, &trc);
    
    for (int i = 0; i < _client_nums; ++i) {
        threads[i].join();
    }
//    statThread.join();
}

void TaskRunnerContainer::product()
{
    for (int i = 0; i < _client_nums; ++i) {
        _container.push(_taskGenerator(i));
    }
}

void TaskRunnerContainer::consume()
{
    TaskRunner* task;
    {
        std::unique_lock<std::mutex> lm(id_increase_mutex);
        task = _container.top();
        _container.pop();
        task->connect(_addr.c_str(), _port);
    }
    unsigned int time = 0;
    do {
        task->run(_addr.c_str(), _port);
        {
            int rtime = uid(rg);
            time += rtime;
            sleep(rtime);
            std::unique_lock<std::mutex> lm(queue_mutex);
            task->printStatistics(std::cout);
            task->printStatistics(_fout);
        }
    } while (time < 60);
    task->stop();
}

void TaskRunnerContainer::statEverySec()
{
//    int lastReqs = 0;
//    do {
//        if (_current_client_id > _client_nums) {
//            break;
//        }
//        sleep(1);
//        _fout_summary<<(_totalReqs - lastReqs)<<endl;
//        lastReqs = _totalReqs;
//    } while (1);
}

void TaskRunnerContainer::setGenerateFunc(std::function<TaskRunner*(int)>&& f)
{
    _taskGenerator = f;
}