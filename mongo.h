#pragma once
#include<mongocxx/instance.hpp>
#include<mongocxx/uri.hpp>
#include<mongocxx/client.hpp>
#include<mongocxx/exception/operation_exception.hpp>
#include<mongocxx/exception/exception.hpp>
#include<bsoncxx/document/view.hpp>
#include<bsoncxx/builder/basic/document.hpp>
#include<bsoncxx/builder/basic/array.hpp>
#include<bsoncxx/document/value.hpp>
#include"School.pb.h"
#include<vector>
#include<string>
#include<stdexcept>
#include"Tutor.h"
#include"poolhandle.h"
#include<mongocxx/options/index.hpp>
#include"Student.h"
#include<mongocxx/options/update.hpp>
#include<bsoncxx/builder/basic/document.hpp>
#include<bsoncxx/builder/basic/array.hpp>
#include<bsoncxx/builder/basic/kvp.hpp>
#include<mongocxx/pipeline.hpp>

using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::kvp;
using bsoncxx::types::b_date;
using School::Teacher;
using School::Student;
using School::Grade;
using School::Return_Grade;
using School::Sex;
using School::Role;

using student_exam=std::variant<StudentExam,std::string>;
using l_areas_container=std::vector<std::pair<std::string,std::string>>;
using login_response= std::variant<std::string,Teacher>;


class SchoolDB{
    public:
        SchoolDB();
        std::string CreateStudent(const std::string& name,const std::string& sex,const std::string& upi,const std::string& grade,
        const std::string& school_id);
        std::string DeleteStudent(const std::string& upi,const std::string& school_id);
        void EditStudentDetails(const School::Student& learner,const std::string& school_id);
        std::vector<Student> FindStudents(const std::string& school_id);
        std::string DeleteStudentSubject(const std::string& upi,std::vector<std::string>&& subjects,const std::string& schoolid);

        poolhandle getcollection(const std::string& name);
        std::string CreateTeacher(Teacher& teach,std::vector<std::pair<std::string,std::string>>&& grade_sub,
        const std::string& username,const std::string& password,const std::string& school_id);
        std::string DeleteTeacher(int code,const std::string& school_id);
        std::vector<Tutor> FindTeachers(const std::string& school_id);
        std::string Teacher_Name(int code,const std::string& school_id);
        std::string UpdateLearningAreas(int code,const std::string& school_id,l_areas_container && grade_sub);
        void Delete_TeacherSubjects(int code,const std::string& school_id,const l_areas_container& gradsubs={});
        std::vector<StudentExam> GetStudentsperSubject(const std::string& grade,const std::string& subj,
            int t_code,const std::string& examname,const std::string& school_id);
        std::string DeleteTeacherSubject(const std::vector<School::GradeSub>&subjects,int teacher_code,const std::string& school_id);
        
        std::vector<StudentExam>GetStudentScoreInSubject(const std::string& grade,const std::string& subj,
            int t_code,const std::string& examname,const std::string& school_id,int paper_number);

        void create_indexes();
        std::string CreateGrade(Grade&& grade,std::vector<std::pair<int,std::string>>&& subs,const std::string& school_id,
            const std::string& level="");
        std::string DeleteGrade(const std::string& grade_name);
        std::vector<Return_Grade> FindGrades(const std::string& school_id);
        std::string UpdateGrade(const std::string& name,const std::string& school_id,const std::string& name2="",int code=-1,
        const std::vector<std::pair<int,std::string>>& added_sub={});

       void AddScore(const std::string& exam,const std::string& upi,const std::string& subject, int score,const std::string& school_id);
       void AddScore(const std::string& exam,const std::string& upi,const std::pair<std::string,int>&score,const std::string& subject,
        const std::string& school_id,int out_of);
       student_exam StudentExamReport(const std::string& upi,const std::string& examname,const std::string& school_id);

       std::string CreateExam(const std::string& exam_name,int term,std::vector<std::pair<std::string,bool>>&& grades,const std::string& school_id);
       std::vector<School::Exam>GetExams(const std::string& school_id);
       std::string AddExamToGrade(const std::string& exam,const std::string& grade,const std::string&school_id,int term,bool full=false);
       bool ExamAnalyzed(const std::string& grade_name,const std::string& exam_name,bool analysis,const std::string& school_id);
       std::vector<Student>FindGradeStudents(const std::string& grade,const std::string& school_id);
       School::MeritList FetchGradeMeritList(const std::string& grade_name,const std::string& exam_name,const std::string& school_id);
       School::MeritList GetGradeMeritList(const std::string& grade_name,const std::string& exam_name,const std::string& school_id);
       void LoadedSubjects(const std::string& grade,const std::string& exam,const std::string& subject,
            const std::string& school_id,bool full_paper=false,int paper_number=0);
       std::vector<std::string>get_loadedsubjects(const std::string& grade,const std::string& exam,const std::string& school_id);
       std::string getsubject_level(int score,const std::string& school_id);
       std::string Science_Math_Grading(int marks,const std::string&s_id);
       void AddExamPapers(const std::string& grade,const std::string& school_id,const std::string& exam);
       login_response UserLogin(const std::string& username,const std::string& password);
       std::string ResetUserPassword(const std::string& username,const std::string& newpassword);
       Return_Grade GetSubjects_Teacher(const std::string& grade,const std::string& school_id);
       std::string Delete_GradeSubjects(const std::string& grade,const std::string& school_id,const std::vector<std::pair<int,std::string>>& subjects);

       std::string CreateSchool(const std::string& name,const std::string& addr,const std::string& id,
        const std::string& email,const School::Category& cate,const std::string& motto);
       School::school GetSchoolDetails(const std::string& school_id);
       School::SchoolList GetSchools();
       std::string UpdateSchoolDetails(const School::UpdateSchoolRequest& request);
       void AddSubjects_to_Student(const std::string& upi,const std::string& school_id,
            const std::vector<std::string>& subjects);
       std::vector<StudentExam>GetStudents_Mark(const std::string& grade,const std::string& subj,int t_code,
                const std::string& examname,const std::string& school_id);

        std::string UpdateAdminRole(int code,const std::string& school_id,const School::Role& new_role);
        std::string Add_SchoolHead(const School::Teacher& head,const std::string& school_id);
        std::string Delete_SchoolHead(const std::string& school_id);
        
        std::string insertInvoice(const School::Invoice& inv);
        bool insertReceipt(const School::Receipt& r);
        std::vector<School::Invoice>getInvoicesBySchool(const std::string& schoolId);
        std::vector<School::Receipt>getReceiptsBySchool(const std::string& schoolId);
    private:
    mongocxx::database db;
    //mongocxx::client client;
    mongocxx::pool pool;
};