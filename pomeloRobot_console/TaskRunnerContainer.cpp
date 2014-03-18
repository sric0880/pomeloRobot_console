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
int TaskRunnerContainer::_totalReqs = 0;
TaskRunnerContainer::TaskRunnerContainer(const char* addr, int port, int clients_nums)
:_addr(addr),_port(port),_client_nums(clients_nums),_current_client_id(0)
{
    char buf[50];
    sprintf(buf, "stat_%d.txt", getpid());
    _fout.open(buf);
    sprintf(buf, "stat_%d_sum.txt", getpid());
    _fout_summary.open(buf);
//    TaskRunner::printHeader(std::cout);
    TaskRunner::printHeader(_fout);
}
TaskRunnerContainer::~TaskRunnerContainer()
{
    _fout.close();
    _fout_summary.close();
}

void TaskRunnerContainer::release()
{
    for (auto tr : _toRelease) {
        if (tr) {
            delete tr;
        }
    }
//    _toRelease.clear();
}

std::mutex id_increase_mutex;
std::mutex queue_mutex;
#include <chrono>
using namespace std::chrono;

void TaskRunnerContainer::runTask()
{
    do {
        TaskRunner* tr = NULL;
        {
//            id_increase_mutex.lock();
            std::unique_lock<std::mutex> lm(id_increase_mutex);
            if (_current_client_id > _client_nums) {
                break;
            }
            tr = _taskGenerator(_current_client_id);
            _toRelease.push_back(tr);
            ++_current_client_id;
//            id_increase_mutex.unlock();
        }
        
        if (tr) {
            tr->run(_addr.c_str(), _port);
            {
//                queue_mutex.lock();
                std::unique_lock<std::mutex> lm(queue_mutex);
                tr->printStatistics(std::cout);
                tr->printStatistics(_fout);
//                queue_mutex.unlock();
            }
        }
    } while (1);
}


void TaskRunnerContainer::statEverySec()
{
    int lastReqs = 0;
    do {
        if (_current_client_id > _client_nums) {
            break;
        }
        sleep(1);
        _fout_summary<<(_totalReqs - lastReqs)<<endl;
        lastReqs = _totalReqs;
    } while (1);
}

void TaskRunnerContainer::setGenerateFunc(std::function<TaskRunner*(int)> f)
{
    _taskGenerator = f;
}