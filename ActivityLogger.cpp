#include "ActivityLogger.h"
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

using bsoncxx::builder::stream::document;
using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::stream::finalize;

ActivityLogger::ActivityLogger(mongocxx::collection collection)
    : collection_(std::move(collection))
{
    worker_ = std::thread(&ActivityLogger::workerThread, this);
}

ActivityLogger::~ActivityLogger()
{
    {
        std::lock_guard<std::mutex> lock(mtx_);
        running_ = false;
    }
    condition_.notify_all();

    if (worker_.joinable()) {
        worker_.join();
    }
}

void ActivityLogger::log(LogRecord record)
{
    {
        std::lock_guard<std::mutex> lock(mtx_);
        logQueue_.push(std::move(record));
    }
    condition_.notify_one();
}

void ActivityLogger::workerThread(){
    while (running_) {
        std::unique_lock<std::mutex> lock(mtx_);

        condition_.wait(lock, [this] {
            return !logQueue_.empty() || !running_;
        });

        while (!logQueue_.empty() && running_) {
            LogRecord record = std::move(logQueue_.front());
            logQueue_.pop();

            lock.unlock();   // unlock while doing I/O

            try {
                auto now=std::chrono::system_clock::now();
                auto expire_time=now+std::chrono::hours(24*30);
                auto doc = make_document(
                    kvp("school_id", record.schoolId),
                    kvp("user_id", record.userId),
                    kvp("method_name", record.methodName),
                    kvp("description", record.description),
                    kvp("duration_ms", bsoncxx::types::b_int64{record.duration}),
                    kvp("status_code", record.statusCode),
                    kvp("status_message", record.statusMessage),
                    kvp("timestamp", bsoncxx::types::b_date{record.timestamp}),
                    kvp("expireAt",bsoncxx::types::b_date{expire_time})
                );

                collection_.insert_one(doc.view());
            }
            catch (const std::exception& e) {
                // TODO: Log to file or stderr
            }

            lock.lock();
        }
    }
}