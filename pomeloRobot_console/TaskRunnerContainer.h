//
//  TaskRunnerContainer.h
//  pomeloRobot_console
//
//  Created by qiong on 14-2-24.
//  Copyright (c) 2014年 qiong. All rights reserved.
//

#ifndef __pomeloRobot_console__TaskRunnerContainer__
#define __pomeloRobot_console__TaskRunnerContainer__

#include <functional>
#include <string>
//#include <vector>
#include <stack>
#include <fstream>

class TaskRunner;

class TaskRunnerContainer{
public:
    TaskRunnerContainer(const char* addr, int port, int clients_nums);
    virtual ~TaskRunnerContainer();
    void startRun(int numOfThreads);
    void setGenerateFunc(std::function<TaskRunner*(int)>&&);
    void release();
    /*statistic*/
    static int _totalReqs;        //当前已发送的请求总数
private:
    std::function<TaskRunner*(int)> _taskGenerator;
    std::string _addr;
    int _port;
    int _client_nums;
    int _current_client_id;
    std::stack<TaskRunner*> _container;
    
    std::ofstream _fout;
//    std::ofstream _fout_summary;
    
    void consume();
    void product();
    void statEverySec();          //FIXME: 暂时不正确
};

#endif /* defined(__pomeloRobot_console__TaskRunnerContainer__) */
