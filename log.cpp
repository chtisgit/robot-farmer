#include <mutex>

#include "log.h"
#include "util.h"

/** initialization */
log::logptr log::log_instance = nullptr;

/**
 * Create a new instance of the log class.
 * Only used in the main thread.
 * Use instead the get_instance() function.
 * @param io
 */
void log::create(std::ostream& out) {
    log::log_instance = new log(out);
}

/**
 * Shutdown the main log worker. Do this before exiting program.
 */
void log::shutdown() {
    delete(log::log_instance);
    log::log_instance = nullptr;
}

/**
 * Private constructor.
 * Do not use. Use create instead.
 * @param out
 */
log::log(std::ostream& out) : log_output(out) {
    log_running = true;
    log_worker = new std::thread(std::bind(&log::run, this));
}

log::~log() {
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
log::logptr log::get_instance() {
    return log::log_instance;
}

/**
 * Log a message.
 * Threadsafe.
 * @param message
 */
void log::msg(const std::string &message) {
    std::lock_guard<std::mutex> lk(log_lock);
    message_queue.push(message);
    signal.notify_all();
}

void log::msg(const char *message) {
    std::lock_guard<std::mutex> lk(log_lock);
    message_queue.push(std::string(message));
    signal.notify_all();
}

/**
 * Internal callback for the worker thread.
 */
void log::run() {

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

void log::output(std::string msg) {

    log_output << msg;
}

std::stringstream& logger::stream() {

    return log_stream;
}

logger::~logger() {
    log::logptr p = log::get_instance();
    if (p) {
        p->msg(log_stream.str());
    } else {
        std::cout << "Note: logging to stdout because log object is gone." << std::endl;
        std::cout << log_stream.str();
    }
}

