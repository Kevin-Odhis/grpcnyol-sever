#pragma once

#include "LogRecord.h"
#include <mongocxx/collection.hpp>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>

class ActivityLogger {
public:
    explicit ActivityLogger(mongocxx::collection collection);
    ~ActivityLogger();
    void log(LogRecord record);

private:
    void workerThread();

    std::queue<LogRecord> logQueue_;
    mutable std::mutex mtx_;
    std::condition_variable condition_;
    std::thread worker_;
    std::atomic<bool> running_{true};
    mongocxx::collection collection_;
};