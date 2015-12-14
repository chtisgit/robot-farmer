/* 
 * File:   log.h
 * Author: alexander
 *
 * Created on March 31, 2014, 8:09 PM
 */

#include <queue>
#include <mutex>
#include <condition_variable>
#include <sstream>
#include <thread>
#include <iostream>

#ifndef LOG_H
#define LOG_H
class log {
public:
    typedef log* logptr;

    static logptr get_instance();
    static void shutdown();

    void msg(const std::string &message);
    void msg(const char *message);

    static void create(std::ostream &out);
    log(std::ostream& out);
    virtual ~log();

private:
    void run();
    void output(std::string msg);

    std::queue<std::string> message_queue;
    std::mutex log_lock;
    std::condition_variable signal;
    std::thread *log_worker;
    std::ostream &log_output;
    bool log_running;

    static logptr log_instance;
};

/*
 * Logs when destructor is called.
 */
class logger {
public:
    std::stringstream& stream();
    ~logger();

private:
    std::stringstream log_stream;
};
#endif  /* LOG_H */
