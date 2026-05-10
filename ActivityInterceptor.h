#pragma once

#include <grpcpp/grpcpp.h>
#include <grpcpp/support/server_interceptor.h>
#include <chrono>
#include <memory>
#include <string>

class ActivityLogger;
struct LogRecord;

class ActivityInterceptor final : public grpc::experimental::Interceptor {
public:
    ActivityInterceptor(grpc::experimental::ServerRpcInfo* info,
                        std::shared_ptr<ActivityLogger> logger);

    void Intercept(grpc::experimental::InterceptorBatchMethods* methods) override;

private:
    struct ActivityData {
        std::chrono::steady_clock::time_point startTime{};
        std::string userId = "Anonymous";
        std::string schoolId;
        std::string methodName;
    };

    ActivityData activityData_;
    std::shared_ptr<ActivityLogger> logger_;

    void logActivity(long long duration, const grpc::Status& status);
    std::string getDescription(const std::string& methodName);
};

class ActivityInterceptorFactory final:public grpc::experimental::ServerInterceptorFactoryInterface {
    public:
        explicit ActivityInterceptorFactory(std::shared_ptr<ActivityLogger> logger):
            logger_(std::move(logger)){}
        grpc::experimental::Interceptor* CreateServerInterceptor(
            grpc::experimental::ServerRpcInfo* info){
            return new ActivityInterceptor(info, logger_);
            }
    private:
        std::shared_ptr<ActivityLogger> logger_;
};