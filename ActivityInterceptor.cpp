#include "ActivityInterceptor.h"
#include "ActivityLogger.h"
#include "LogRecord.h"
#include <grpcpp/impl/codegen/interceptor.h>
#include<unordered_map>

ActivityInterceptor::ActivityInterceptor(grpc::experimental::ServerRpcInfo* info,
                                         std::shared_ptr<ActivityLogger> logger)
    : logger_(std::move(logger))
{
    activityData_.methodName = info ? info->method() : "Unknown";
}

void ActivityInterceptor::Intercept(grpc::experimental::InterceptorBatchMethods* methods)
{
    using clock = std::chrono::steady_clock;

    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::POST_RECV_INITIAL_METADATA)){
        activityData_.startTime = clock::now();
        activityData_.userId = "Anonymous";
        activityData_.schoolId.clear();

        auto* metadata = methods->GetRecvInitialMetadata();
        if (metadata) {
            auto userIt = metadata->find("userid");
            if (userIt != metadata->end()) {
                activityData_.userId = std::string(userIt->second.data(), userIt->second.length());
            }

            auto schoolIt = metadata->find("schoolid");
            if (schoolIt != metadata->end()) {
                activityData_.schoolId = std::string(schoolIt->second.data(), schoolIt->second.length());
            }
        }
    }

    if (methods->QueryInterceptionHookPoint(
            grpc::experimental::InterceptionHookPoints::PRE_SEND_STATUS))
    {
        if (activityData_.startTime.time_since_epoch().count() == 0) {
            // Safety: in case startTime was never set
            activityData_.startTime = clock::now();
        }

        auto endTime = clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            endTime - activityData_.startTime).count();

        if (activityData_.schoolId.empty()) {
            activityData_.schoolId = "Unknown";
        }

        grpc::Status status = methods->GetSendStatus();
        logActivity(duration, status);
    }

    methods->Proceed();
}

void ActivityInterceptor::logActivity(long long duration, const grpc::Status& status)
{
    LogRecord record{
        activityData_.schoolId,
        activityData_.userId,
        activityData_.methodName,
        getDescription(activityData_.methodName),
        duration,
        status.error_code(),
        status.error_message(),
        std::chrono::system_clock::now()
    };

    logger_->log(std::move(record));
}

std::string ActivityInterceptor::getDescription(const std::string& methodName){
    std::unordered_map<std::string,std::string> methodDescriptions = {
        {"/School.SchoolService/AddStudent", "Added a student"},
        {"/School.SchoolService/RemoveStudent", "Removed a student"},
        {"/School.SchoolService/GetStudents", "Retrieved students"},
        {"/School.SchoolService/AddSubjectsToStudent", "Added subjects to a student"},
        {"/School.SchoolService/AddTeacher", "Added a teacher"},
        {"/School.SchoolService/GetTeachers", "Retrieved teachers"},
        {"/School.SchoolService/RemoveTeacher", "Removed a teacher"},
        {"/School.SchoolService/AddExam", "Added an exam"},
        {"/School.SchoolService/AddGrade", "Added a grade"},
        {"/School.SchoolService/GetGrades", "Retrieved grades"},
        {"/School.SchoolService/RemoveGrade", "Removed a grade"},
        {"/School.SchoolService/UpdateGrade", "Updated a grade"},
        {"/School.SchoolService/AddStudentScore", "Added a score"},
        {"/School.SchoolService/StudentExamReport", "Retrieved an exam report"},
        {"/School.SchoolService/UpdateSubjects", "Updated subjects for a student"},
        {"/School.SchoolService/ListofLearnersbyGrade_Subject", "Retrieved students per subject"},
        {"/School.SchoolService/GetStudentForTeacher", "Retrieved students for a teacher"},
        {"/School.SchoolService/TeacherLogin", "Logged in a as a teacher"},
        {"/School.SchoolService/RemoveGradeSubject", "Deleted a subject from a grade"},
        {"/School.SchoolService/SetExamAnalysed", "Set an exam as analysed"},
        {"/School.SchoolService/GradeStudents", "Retrieved students for a grade"},
        {"/School.SchoolService/fetchLoadedSubjects", "Retrieved the list of loaded subjects"},
        {"/School.SchoolService/setLoadedSubject", "Set a subject as loaded"},
        {"/School.SchoolService/GradeMeritList", "Retrieved the merit list for a grade"},
        {"/School.SchoolService/FindExams","Retrieved assessment list"},
        {"/School.SchoolService/FindGrades","Retrieved grade list"},
        {"/School.SchoolService/ResetPassword","Reset a teacher's password"},
        {"/School.SchoolService/GetSchoolDetails","Retrieved school details"},
        {"/School.SchoolService/CreateSchool","Created a school"}
    };
   auto it=methodDescriptions.find(methodName);
   if(it!=methodDescriptions.end()){
        return it->second;
    }
    return "Performed an unknown action: " + methodName;
}