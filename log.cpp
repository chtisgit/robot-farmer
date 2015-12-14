#include <mutex>

#include "log.h"

/** initialization */
Log::logptr Log::log_instance = nullptr;

/**
 * Create a new instance of the log class.
 * Only used in the main thread.
 * Use instead the get_instance() function.
 * @param io
 */
void Log::create(std::ostream& out) {
    Log::log_instance = new Log(out);
}

/**
 * Shutdown the main log worker. Do this before exiting program.
 */
void Log::shutdown() {
    delete(Log::log_instance);
    Log::log_instance = nullptr;
}

/**
 * Private constructor.
 * Do not use. Use create instead.
 * @param out
 */
Log::Log(std::ostream& out) : log_output(out) {
    log_running = true;
    log_worker = new std::thread(std::bind(&Log::run, this));
}

Log::~Log() {
    log_running = false;
    signal.notify_all();

    // wait for the worker to exit
    log_worker->join();

    delete(log_worker);
}

/**
 * Get a pointer to the logger object.
 * 
 * Returns a null pointer when before create is called!
 * @return
 */
Log::logptr Log::get_instance() {
    return Log::log_instance;
}

/**
 * Log a message.
 * Threadsafe.
 * @param message
 */
void Log::msg(const std::string &message) {
    std::lock_guard<std::mutex> lk(log_lock);
    message_queue.push(message);
    signal.notify_all();
}

void Log::msg(const char *message) {
    std::lock_guard<std::mutex> lk(log_lock);
    message_queue.push(std::string(message));
    signal.notify_all();
}

/**
 * Internal callback for the worker thread.
 */
void Log::run() {

    while (log_running || (message_queue.size() > 0)) {
        std::unique_lock<std::mutex> guard(log_lock);

        while (message_queue.size() > 0) {

            output(message_queue.front());
            message_queue.pop();
        }

        if (log_running) {
            // wait for a signal
            signal.wait(guard);
        }
    }
}

void Log::output(std::string msg) {

    log_output << msg;
}

std::stringstream& Logger::stream() {

    return log_stream;
}

Logger::~Logger() {
    Log::logptr p = Log::get_instance();
    if (p) {
        p->msg(log_stream.str());
    } else {
        std::cout << "Note: logging to stdout because log object is gone." << std::endl;
        std::cout << log_stream.str();
    }
}

