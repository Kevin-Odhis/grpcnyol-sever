#pragma once
#include<grpcpp/grpcpp.h>
#include"School.grpc.pb.h"
#include"School.pb.h"
#include"mongo.h"
#include"ActivityLogger.h"
#include"ActivityInterceptor.h"
#include<iostream>

using grpc::ServerContext;
using grpc::Status;
using grpc::Server;
using grpc::ServerBuilder;

using School::AddStudentRequest;
using School::Response;
using School::Student;
using School::SchoolService;
using School::RemoveStudentRequest;
using School::StudentList;
using School::AddStudentRequest;
using School::Teacher;
using School::GradeSub;
using School::AddTeacherRequest;
using School::TeachersList;
using School::RemoveTeacherRequest;
using School::AddExamRequest;
using School::AddGradeRequest;
using School::GradeList;
using School::RemoveGradeRequest;
using School::Grade;
using School::UpdateGradeRequest;
using School::AddScoreRequest;
using School::ExamReportRequest;
using School::ExamReport;
using School::SubjectScore;
using School::add_subject_score;
using School::UpdateSubjectsRequest;
using School::GetStudentsperSubjectRequest;
using School::LearnersListperGrade;
using School::studentforteacher;
using School::Return_Grade;
using School::ExamList;
using School::Exam;
using School::GradeStudentsRequest;
using School::SetExamAnalysedRequest;
using School::StudentMerit;
using School::MeritList;
using School::MeritListRequest;
using School::getloadedsubjectsrequest;
using School::setloadedsubjectrequest;
using School::SubjectList;

class SchoolServiceImpl final:public SchoolService::Service{
    public:
        explicit SchoolServiceImpl(std::shared_ptr<ActivityLogger> logger) : logger_(std::move(logger)) {};
        Status AddStudent(ServerContext* context,const AddStudentRequest* request,Response* reply) override;
        Status RemoveStudent(ServerContext* context,const RemoveStudentRequest* request,Response* reply) override;
        Status GetStudents(ServerContext* context,const School::FindStudentsRequest* request,StudentList* reply) override;
        Status AddSubjectsToStudent(ServerContext* context,const School::AddSubjectsRequest* request,Response* reply);
        Status EditStudentDetails(ServerContext* context,const School::EditStudentDetailsRequest* request,Response* reply);
        Status DeleteStudentSubject(ServerContext* context,const School::DeleteSubjectRequest* request,Response* reply); 

        Status AddTeacher(ServerContext* context,const AddTeacherRequest* request,Response* reply) override;
        Status GetTeachers(ServerContext* context,const School::GetTeachersRequest* request,TeachersList* reply)override;
        Status RemoveTeacher(ServerContext* context,const RemoveTeacherRequest* request,Response* reply)override;
        Status UpdateSubjects(ServerContext* context,const UpdateSubjectsRequest* request,Response* reply)override;
        Status DeleteTeacherSubject(ServerContext* context,const School::DeleteTeacherSubjectRequest* request,Response* reply)override;

        Status AddExam(ServerContext* context,const AddExamRequest* request,Response* reply)override;

        Status AddGrade(ServerContext* context, const AddGradeRequest* request, Response* reply)override;
        Status RemoveGrade(ServerContext* context,const RemoveGradeRequest* request, Response* reply)override;
        Status FindGrades(ServerContext* context,const School::FindGradesRequest* request, GradeList* reply)override;
        Status UpdateGrade(ServerContext* context,const UpdateGradeRequest* request,Response* r)override;

        Status AddStudentScore(ServerContext* context,const AddScoreRequest* re,Response* reply)override;
        Status StudentExamReport(ServerContext* context,const ExamReportRequest* r,ExamReport* reply)override;
        Status ListofLearnersbyGrade_Subject(ServerContext* context,const GetStudentsperSubjectRequest* request,
            LearnersListperGrade* response)override;
        Status FindExams(ServerContext* context,const School::FindExamRequest* request,ExamList* response)override;
        Status GradeStudents(ServerContext* context,const GradeStudentsRequest* request,StudentList* reply)override;
        Status SetExamAnalysed(ServerContext* context,const SetExamAnalysedRequest* request,Response* reply)override;

        Status GradeMeritList(ServerContext* context,const MeritListRequest* request,MeritList* reply)override;
        Status setLoadedSubjects(ServerContext* context,const setloadedsubjectrequest* request,Response* reply)override;
        Status fetchLoadedSubjects(ServerContext* context,const getloadedsubjectsrequest* request,SubjectList* reply)override;
        Status TeacherLogin(ServerContext* context,const School::LoginRequest* request,Teacher* response)override;
        Status ResetPassword(ServerContext* context,const School::ResetPasswordRequest* request,Response* reply)override;

        Status RemoveGradeSubject(ServerContext* context,const School::RemoveSubjectRequest* request,Response* reply)override;

        Status CreateSchool(ServerContext* context,const School::AddSchoolRequest* request,Response* reply)override;
        Status GetSchools(ServerContext* context,const School::SchoolsRequest* request,School::SchoolList* reply)override;
        Status GetSchoolDetails(ServerContext* context,const School::SchoolDetailsRequest* request,School::SchoolDetails* reply)override;

        Status Invoice(ServerContext* context,const School::AddInvoiceRequest* request,Response* reply)override;
        Status GetInvoice(ServerContext* context,const School::GetInvoiceRequest* request,School::InvoiceList* reply)override;
        Status Receipt(ServerContext* context,const School::AddReceiptRequest* request,Response* reply)override;
        Status GetReceipt(ServerContext* context,const School::GetReceiptRequest* request,School::ReceiptList* reply)override;

    private:
            //database object
        SchoolDB school_db;
        std::shared_ptr<ActivityLogger> logger_;
    };