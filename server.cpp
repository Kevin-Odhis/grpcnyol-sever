#include "server.h"
#include <grpcpp/security/server_credentials.h>
#include<fstream>
#include <absl/log/initialize.h>
//#include <grpcpp/ext/proto_server_reflection_plugin.h>
Status SchoolServiceImpl::AddStudent(ServerContext* context,const AddStudentRequest* request,Response* reply){
   try{
        Student learner=request->student();
        if(learner.name().empty()){
            reply->set_success(false);
            reply->set_message("All fields are required.");
            return Status(grpc::StatusCode::INTERNAL,"Studende name is required");
        }
        if(learner.upi_no().empty()){
            reply->set_success(false);
            reply->set_message("All fields are required.");
            return Status(grpc::StatusCode::INTERNAL,"Admission number is required!");
        }
        if(learner.grade().empty()){
            reply->set_success(false);
            reply->set_message("All fields are required.");
            return Status(grpc::StatusCode::INTERNAL,"Student's grade/class is required");
        }
        std::string s_id=request->school_id();
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("The School is not recorgnised!");
            return Status(grpc::StatusCode::NOT_FOUND,reply->message());
        }
        
        std::string name=learner.name();
        std::string upi=learner.upi_no();
        std::string grade_name=learner.grade();
        std::string sex=Sex_Name(learner.gender());
        std::string db_response=school_db.CreateStudent(name,sex,upi,grade_name,s_id);
        if(!db_response.empty()&&db_response.find("Could not add")!=std::string::npos){
            reply->set_success(false);
            reply->set_message(db_response);
            return Status(grpc::StatusCode::INTERNAL,db_response);
        }
        reply->set_success(true);
        reply->set_message(db_response);
        return Status::OK;
   }
   catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
   }
   catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
   }
    catch(...){
          return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::EditStudentDetails(ServerContext* context,const School::EditStudentDetailsRequest* request,Response* reply){
    try{
         std::string school_id=request->school_id();
         School::Student learner=request->student();
         school_db.EditStudentDetails(learner,school_id);

         reply->set_success(true);
         reply->set_message("Changes have been made successfully.");
         return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}

Status SchoolServiceImpl::RemoveStudent(ServerContext* context,const RemoveStudentRequest* request,Response* reply){
    try{
        if(request->school_id().empty()){
            reply->set_success(false);
            reply->set_message("The School is not recorgnised!");
            return Status(grpc::StatusCode::NOT_FOUND,reply->message());
        }
        if(request->upi_no().empty()){
            reply->set_success(false);
            return Status(grpc::StatusCode::INTERNAL,"UPI NO is required!");
        }
        std::string s_id=request->school_id();
        std::string upi=request->upi_no();
        std::string db_response=school_db.DeleteStudent(upi,s_id);
        if(!db_response.empty()&&db_response.find("Learner with upi")!=std::string::npos){
            reply->set_success(false);
            reply->set_message(db_response);
            return Status(grpc::StatusCode::INTERNAL,db_response);
        }
        reply->set_success(true);
        reply->set_message(db_response);
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::DeleteStudentSubject(ServerContext* context,const School::DeleteSubjectRequest* request,Response* reply){
    try{
        std::string schoolid=request->school_id();
        std::string upi=request->student_upi();
        if(schoolid.empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::ABORTED,reply->message());
        }
        if(upi.empty()){
            reply->set_success(false);
            reply->set_message("No student selected!");
            return Status(grpc::StatusCode::ABORTED,reply->message());
        }

        std::vector<std::string>subjects;
        for(const auto& item:request->subjects()){
            subjects.push_back(item);
        }
        if(subjects.empty()){
            reply->set_success(false);
            reply->set_message("Select subject(s) to remove!");
            return Status(grpc::StatusCode::ABORTED,reply->message());
        }
        auto response=school_db.DeleteStudentSubject(upi,std::move(subjects),schoolid);
        reply->set_message(response);
        reply->set_success(true);
        return Status::OK;
    }
      catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::GetStudents(ServerContext* context,const School::FindStudentsRequest* request,StudentList* reply){
    try{
        if(request->school_id().empty()){
            return Status(grpc::StatusCode::INTERNAL,"The School is not recorgnised!");
        }
        std::string s_id=request->school_id();
        auto students=school_db.FindStudents(s_id);
        if(students.empty()){
            return Status(grpc::StatusCode::NOT_FOUND,"No students found in the database.");
        }
        for(const auto& stu:students){
            Student* learner=reply->add_students();
            learner->CopyFrom(stu);
        }
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::AddSubjectsToStudent(ServerContext* context,const School::AddSubjectsRequest* request,Response* reply){
    try{
        std::string s_id=request->school_id();
        std::string upi=request->upi();
        std::vector<std::string>v_subjects;
        for(const auto& sub:request->subjects()){
            v_subjects.push_back(sub.subject());
        }
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("The School is not recorgnized.");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        if(v_subjects.empty()){
            reply->set_success(false);
            reply->set_message("No subject selected.");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        school_db.AddSubjects_to_Student(upi,s_id,v_subjects);
        reply->set_success(true);
        reply->set_message("Subjects added successfully");

        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}

Status SchoolServiceImpl::AddGrade(ServerContext* context,const AddGradeRequest* request,Response* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        std::string level=School::Category_Name(request->level());
        Grade grade=request->grade();
        std::vector<std::pair<int,std::string>> subjects;subjects.reserve(9);

        for(const auto& sub:request->subjects()){
            subjects.emplace_back(sub.code(),sub.subject());
        }
        if(grade.name().empty()||subjects.empty()){
            reply->set_success(false);
            return Status(grpc::StatusCode::INTERNAL,"All fields are required!");
        }
        auto db_response=school_db.CreateGrade(std::move(grade),std::move(subjects),s_id,level);
        reply->set_success(true);
        reply->set_message(db_response);
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}

Status SchoolServiceImpl::FindGrades(ServerContext* context,const School::FindGradesRequest* request,GradeList* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            return Status(grpc::StatusCode::INTERNAL,"The School is not recorgnized!");
        }
        auto all_grades=school_db.FindGrades(s_id);
        if(all_grades.empty()){
            return Status(grpc::StatusCode::NOT_FOUND,"No grades found in the database.");
        }
        reply->clear_grades();
        for(const auto& grd:all_grades){
            Return_Grade* grade=reply->add_grades();
            *grade=grd;
        }
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::RemoveGrade(ServerContext* context,const RemoveGradeRequest* request, Response* reply){
    try{
        if(request->name().empty()){
        reply->set_success(false);
        return Status(grpc::StatusCode::INTERNAL,"Grade not selected!");
       }
       std::string grade_name=request->name();
       auto db_response=school_db.DeleteGrade(grade_name);
       if(db_response.find("Could not find")!=std::string::npos){
              reply->set_success(false);
              reply->set_message(db_response);
              return Status(grpc::StatusCode::INTERNAL,db_response);
       }
       reply->set_success(true);
       reply->set_message(db_response);
       return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::UpdateGrade(ServerContext* context,const UpdateGradeRequest* request,
    Response* reply){
        try{
            std::string s_id=request->school_id();
            if(s_id.empty()){
                reply->set_success(false);
                reply->set_message("The school is not recorgnized!");
                return Status(grpc::StatusCode::INTERNAL,reply->message());
            }
            int teacher_code=request->code();
            std::string newgrade=request->newname();
            std::string currentgrade=request->name();

            std::vector<std::pair<int,std::string>> subject;

            if(currentgrade.empty()){
                reply->set_success(false);
                return Status(grpc::StatusCode::INTERNAL,"at least a grade must be selected!");
            }
            std::string db_response{};
            if(request->subjects_size()>0){
                for(const auto& sub:request->subjects()){
                    subject.emplace_back(sub.code(),sub.subject());
                }
            }
            if(!subject.empty()){
               db_response=school_db.UpdateGrade(currentgrade,s_id,newgrade,teacher_code,subject);
            }
            else{
                db_response=school_db.UpdateGrade(currentgrade,s_id,newgrade,teacher_code);
            }
            
            if(db_response.find("could not find")!=std::string::npos||db_response.find("At least")!=std::string::npos){
                reply->set_success(false);
                reply->set_message(db_response);
                return Status(grpc::StatusCode::INTERNAL,db_response);
            }
            reply->set_success(true);
            reply->set_message(db_response);
            return Status::OK;
        }
        catch(const std::runtime_error& e){
            return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
        }
        catch(const std::exception& e){
            return Status(grpc::StatusCode::INTERNAL,"Server error "+std::string(e.what()));
        }
        catch(...){
            return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
        }

    }
 Status SchoolServiceImpl::AddStudentScore(ServerContext* context,const AddScoreRequest* request,
        Response* reply){
    try{
        std::string s_id=request->school_id();
        int out_of=request->out_of();
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        add_subject_score score=request->mark();
        if(score.upi().empty()||score.subject().empty()||score.exam_name().empty()){
            reply->set_success(false);
            return Status(grpc::StatusCode::INTERNAL,"All fields are required!");
        }
        std::string upi=score.upi();
        auto subject=score.subject();
        auto marks=score.scores();
        bool full_paper=score.full_paper();
        std::string exam=score.exam_name();
        int mark=marks.score();
        int pp=marks.pp();
        

        if(full_paper){
            std::string p="pp"+std::to_string(pp);
            std::pair<std::string,int> score{p,mark};
            school_db.AddScore(exam,upi,score,subject,s_id,out_of);
        }
        else{
            school_db.AddScore(exam,upi,subject,mark,s_id);
        }
        
        reply->set_success(true);
        reply->set_message("Scores added successfully.");
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception&e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::StudentExamReport(ServerContext* context,const ExamReportRequest* request,ExamReport* reply){

 try{
    std::string s_id=request->school_id();
    if(s_id.empty()){
        return Status(grpc::StatusCode::INTERNAL,"The school is not recorgnized!");
    }
    if(request->upi().empty()||request->exam_name().empty()){
    return Status(grpc::StatusCode::INTERNAL,"UPI NO for the student is required!");
   }
   auto upi=request->upi();
   auto exam=request->exam_name();
   auto result=school_db.StudentExamReport(upi,exam,s_id);
   if(std::holds_alternative<std::string>(result)){
    return Status(grpc::StatusCode::INTERNAL,std::get<std::string>(result));
   }
   if(std::holds_alternative<StudentExam>(result)){
     StudentExam report=std::get<StudentExam>(result);
     reply->set_total_marks(report.total_marks);
     reply->set_overall_level(report.level);
     auto all_scores=report.get_subject_scores();
     for(const auto& [sub,score]:all_scores){
        SubjectScore* subject=reply->add_scores();
        subject->set_subject(sub);
        subject->set_score(score.second);
        subject->set_level(score.first);
     }
   }
    return Status::OK;
 }
catch(const std::runtime_error& e){
    return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
}
catch(const std::exception& e){
    return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
}
catch(...){
    return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}

Status SchoolServiceImpl::UpdateSubjects(ServerContext* context,const UpdateSubjectsRequest* request,
    Response* reply){
        auto sub=request->new_subjects();
        try{
            std::string s_id=request->school_id();
            if(s_id.empty()){
                reply->set_success(false);
                reply->set_message("The school is not recorgnized!");
                return Status(grpc::StatusCode::INTERNAL,reply->message());
            }

            std::vector<std::pair<std::string,std::string>> l_areas;
            for(const auto& ele:sub){
                auto grad=ele.grade();
                auto learning=ele.subject();
                l_areas.emplace_back(grad,learning);
            }
            int code=request->teacher_code();
            Role role=request->role();
            
            std::string db_response=school_db.UpdateLearningAreas(code,s_id,role,std::move(l_areas));
            if(db_response.find("Could not")!=std::string::npos||db_response.find("Update failed")!=std::string::npos){
                reply->set_success(false);
                reply->set_message(db_response);
                return Status(grpc::StatusCode::INTERNAL,db_response);
            }
            reply->set_success(true);
            reply->set_message(db_response);
            return Status::OK;
        }
        catch(const std::runtime_error& e){
            return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
        }
        catch(const std::exception& e){
            return Status(grpc::StatusCode::INTERNAL,"Server error: "+std::string(e.what()));
        }
        catch(...){
            return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
        }
    }
Status SchoolServiceImpl::AddExam(ServerContext* context,const AddExamRequest* request,Response* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        std::string exam_name=request->name();
        int term=request->term();
        std::vector<std::pair<std::string,bool>>grades;
        for(const auto& ele:request->grades()){
            grades.emplace_back(ele.grade(),ele.full_paper());
        }
        if(exam_name.empty()||grades.empty()||term<=0){
            reply->set_success(false);
            return Status(grpc::StatusCode::INTERNAL,"All fields are required!");
        }
        std::string db_response=school_db.CreateExam(exam_name,term,std::move(grades),s_id);
        if(db_response.find("Could not")!=std::string::npos){
            reply->set_success(false);
            reply->set_message(db_response);
            return Status(grpc::StatusCode::INTERNAL,db_response);
        }
        reply->set_success(true);
        reply->set_message(db_response);
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::ListofLearnersbyGrade_Subject(ServerContext* context,const GetStudentsperSubjectRequest* 
    request,LearnersListperGrade* response){
        try{
            int pp=request->paper_number();
            std::string s_id=request->school_id();
            if(s_id.empty()){
                return Status(grpc::StatusCode::INTERNAL,"The School is not recorgnized");
            }
            School::school s_detail=school_db.GetSchoolDetails(s_id);
            std::string subject=request->subject();
            std::string grade=request->grade_name();
            std::string exam=request->exam_name();
            int t_code=request->code();
            if(subject.empty()||grade.empty()||exam.empty()||t_code<=0){
                return Status(grpc::StatusCode::INTERNAL,"all fields are required!");
            }
            if(request->full_paper()){
                
                std::vector<StudentExam>learners=school_db.GetStudentScoreInSubject(grade,subject,t_code,exam,s_id,pp);
                if(learners.empty()){
                return Status(grpc::StatusCode::NOT_FOUND,"No learners found in that grade for the learning area.");
                }
                for(const auto& learner:learners){
                    studentforteacher* student=response->add_students();
                    student->set_student_name(learner.student_name);
                    student->set_upi(learner.upi);
                    student->set_grade_name(grade);
                    student->set_score(learner.pp_score);
                }
            }else{
                std::vector<StudentExam>learners;
                if(s_detail.category()==School::Category::SECONDARY){
                    learners=school_db.GetStudents_Mark(grade,subject,t_code,exam,s_id);
                }else{
                    learners=school_db.GetStudentsperSubject(grade,subject,t_code,exam,s_id);
                }
                if(learners.empty()){
                    return Status(grpc::StatusCode::NOT_FOUND,"No learners found in that grade for the learning area.");
                }
                for(const auto& learner:learners){
                    studentforteacher* student=response->add_students();
                    student->set_student_name(learner.student_name);
                    student->set_upi(learner.upi);
                    student->set_score(learner.total_marks);
                    student->set_grade_name(grade);
                }
            }
            return Status::OK;
        }
        catch(const std::runtime_error& e){
            return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
        }
        catch(const std::exception& e){
            return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
        }
        catch(...){
            return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
        }
    }

Status SchoolServiceImpl::GetTeachers(ServerContext* context,const School::GetTeachersRequest* request,TeachersList* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            return Status(grpc::StatusCode::INTERNAL,"The School is not recorgnized");
        }
        std::vector<Tutor> teachers=school_db.FindTeachers(s_id);
        if(teachers.empty()){
            return Status(grpc::StatusCode::NOT_FOUND,"No teachers found in the database.");
        }
        for(const auto& teach:teachers){
            Teacher* tutor=reply->add_teachers();

            tutor->set_name(teach.name);
            tutor->set_code(teach.tutor_code);
            std::string role_str=teach.role;
            Role role_t=Role::TEACHER;Role role_h=Role::HOI;Role role_a=Role::ADMIN;
            if(Role_Parse(role_str,&role_t))tutor->set_role(role_t);
            if(Role_Parse(role_str,&role_h))tutor->set_role(role_h);
            if(Role_Parse(role_str,&role_a))tutor->set_role(role_a);

            for(const auto& [grd,subj]:teach.get_grade_subjects()){
                GradeSub* sub_grd=tutor->add_grade_subs();
                sub_grd->set_grade(grd);
                sub_grd->set_subject(subj);
            }
        }
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::AddTeacher(ServerContext* context,const AddTeacherRequest* request,Response* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        Teacher tutor=request->teacher();
        School::loginDetails userlogin=request->login();
        std::vector<std::pair<std::string,std::string>> l_areas;
        if(userlogin.username().empty()||userlogin.password().empty()){
            reply->set_success(false);
            return Status(grpc::StatusCode::INTERNAL,"Password and username are required");
        }
        std::string user=userlogin.username();
        std::string passw=userlogin.password();
        for(const auto& ele:request->grade_subs()){
            std::string grad=ele.grade();
            std::string subj=ele.subject();
            l_areas.emplace_back(grad,subj);
        }
        if(tutor.name().empty()||l_areas.empty()){
            reply->set_success(false);
            return Status(grpc::StatusCode::INTERNAL,"All fields are required!");
        }
        std::string db_response=school_db.CreateTeacher(tutor,std::move(l_areas),user,passw,s_id);
        reply->set_success(true);
        reply->set_message(db_response);
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::RemoveTeacher(ServerContext* context,const RemoveTeacherRequest* request,Response* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        int t_code=request->code();
        std::string t_name=request->name();
        if(t_code<=0){
            reply->set_success(false);
            return Status(grpc::StatusCode::INTERNAL,"Teacher code is required!");
        }
        std::string db_response=school_db.DeleteTeacher(t_code,s_id);
        if(db_response.find("Teacher with")!=std::string::npos){
            reply->set_success(false);
            reply->set_message(db_response);
            return Status(grpc::StatusCode::INTERNAL,db_response);
        }
        reply->set_success(true);
        reply->set_message(db_response);
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::DeleteTeacherSubject(ServerContext* context,const School::DeleteTeacherSubjectRequest* request,Response* reply){
    try{
        int code=request->teachercode();
        std::string s_id=request->school_id();
        std::vector<School::GradeSub>subjects;
        for(const auto& item:request->subjects()){
            subjects.push_back(item);
        }
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("Unrecorgnized school!");
            return Status(grpc::StatusCode::ABORTED,reply->message());
        }
        if(code<=0){
            reply->set_success(false);
            reply->set_message("Select the teacher!");
            return Status(grpc::StatusCode::ABORTED,reply->message());
        }
        if(subjects.empty()){
            reply->set_success(false);
            reply->set_message("You have not selected any subject to be deleted!");
            return Status(grpc::StatusCode::ABORTED,reply->message());
        }
        auto response=school_db.DeleteTeacherSubject(subjects,code,s_id);
        reply->set_success(true);
        reply->set_message(response);
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::FindExams(ServerContext* context,const School::FindExamRequest* request,ExamList* response){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            return Status(grpc::StatusCode::INTERNAL,"The School is not recorgnized");
        }
        auto reply=school_db.GetExams(s_id);
        for(const auto&ass:reply){
            Exam* exam=response->add_exams();
            exam->CopyFrom(ass);
        }
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::GradeStudents(ServerContext* context,const GradeStudentsRequest* request,StudentList* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            return Status(grpc::StatusCode::INTERNAL,"The School is not recorgnized");
        }
        if(request->grade().empty()){
            return Status(grpc::StatusCode::INVALID_ARGUMENT,"Input valid grade.");
        }
        std::string grade=request->grade();
        auto response=school_db.FindGradeStudents(grade,s_id);
        if(response.empty()){
            return Status(grpc::StatusCode::NOT_FOUND,"No students found.");
        }
        for(const auto& learner:response){
            Student* stude=reply->add_students();
            stude->set_name(learner.name());
            stude->set_upi_no(learner.upi_no());
            stude->set_grade(learner.grade());
            stude->set_gender(learner.gender());
        }
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::SetExamAnalysed(ServerContext* context,const SetExamAnalysedRequest* request,Response* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        std::string name=request->exam_name();
        std::string grade_name=request->grade_name();
        bool status=request->analysis_status();
        if(name.empty()||grade_name.empty()){
            reply->set_success(false);
            return Status(grpc::StatusCode::INVALID_ARGUMENT,"Exam name is required.");
        }
        bool response=school_db.ExamAnalyzed(grade_name,name,status,s_id);
        if(!response){
            reply->set_success(false);
            reply->set_message("Could not analyze the exam. Ensure exam exists.");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        reply->set_success(true);
        reply->set_message("Exam analyzed successfully.");
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::GradeMeritList(ServerContext* context,const MeritListRequest* request,MeritList* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            return Status(grpc::StatusCode::INTERNAL,"The School is not recorgnized");
        }
        std::string exam_name=request->examname();
        std::string grade_name=request->gradename();
        if(exam_name.empty()||grade_name.empty()){
            return Status(grpc::StatusCode::INVALID_ARGUMENT,"Exam name and Grade name are required.");
        }
        if(request->full_paper()){
            *reply=school_db.GetGradeMeritList(grade_name,exam_name,s_id);
        }else{
            *reply=school_db.FetchGradeMeritList(grade_name,exam_name,s_id);
        }
       
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::setLoadedSubjects(ServerContext* context,const setloadedsubjectrequest* request,Response* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        std::string examname=request->exam_name();
        std::string gradename=request->grade_name();
        std::string subject=request->subject();
        bool full_pp=request->full_paper();
        int pp_numb=request->paper_number();
        if(examname.empty()||gradename.empty()||subject.empty()){
            reply->set_success(false);
            return Status(grpc::StatusCode::INTERNAL,"Exam name, grade and subject fields are required!");
        }
        school_db.LoadedSubjects(gradename,examname,subject,s_id,full_pp,pp_numb);
        reply->set_success(true);
        reply->set_message(gradename+"scores for "+subject+" loaded successfully.");
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::fetchLoadedSubjects(ServerContext* context,const getloadedsubjectsrequest* request,SubjectList* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            return Status(grpc::StatusCode::INTERNAL,"The School is not recorgnized");
        }
        std::string gradename=request->grade_name();
        std::string examname=request->exam_name();
        if(gradename.empty()||examname.empty()){
            return Status(grpc::StatusCode::INTERNAL,"grade and assessment name is required.");
        }
        auto result=school_db.get_loadedsubjects(gradename,examname,s_id);
        for(const auto& sub:result){
            reply->add_subject(sub);
        }
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::TeacherLogin(ServerContext* context,const School::LoginRequest* request,Teacher* response){
    try{
        School::loginDetails credentials=request->verify();
        std::string user=credentials.username();
        std::string password=credentials.password();
        if(user.empty()||password.empty()){
            return Status(grpc::StatusCode::INTERNAL,"username and password are required");
        }
        auto result=school_db.UserLogin(user,password);
        if(std::holds_alternative<std::string>(result)){
            std::string msg=std::get<std::string>(result);
            return Status(grpc::StatusCode::INTERNAL,msg);
        }
        if(std::holds_alternative<Teacher>(result)){
            Teacher teacher=std::get<School::Teacher>(result);
            response->CopyFrom(teacher);
        }
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::ResetPassword(ServerContext* context,const School::ResetPasswordRequest* request,Response* reply){
    try{
        std::string username=request->username();
        std::string password=request->newpassword();

        if(username.empty()||password.empty()){
            reply->set_success(false);
            return Status(grpc::StatusCode::INTERNAL,"username and new password are required!");
        }
        std::string db_response=school_db.ResetUserPassword(username,password);
        reply->set_success(true);
        reply->set_message(db_response);
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::RemoveGradeSubject(ServerContext* context,const School::RemoveSubjectRequest* request,
    Response* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        std::string grade=request->grade_name();
        std::vector<std::pair<int,std::string>>subjects;
        for(const auto& sub:request->subjects()){
            subjects.emplace_back(sub.code(),sub.subject());
        }
        auto db_response=school_db.Delete_GradeSubjects(grade,s_id,subjects);
        if(db_response.find("successfully")!=std::string::npos){
            reply->set_success(true);
            reply->set_message(db_response);
            return Status::OK;
        }else{
            reply->set_success(false);
            reply->set_message(db_response);
            return Status(grpc::StatusCode::INTERNAL,db_response);
        }

    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::CreateSchool(ServerContext* context,const School::AddSchoolRequest* request,Response* reply){
    try{
        School::school detail=request->details();
        if(detail.name().empty()||detail.school_id().empty()){
            reply->set_success(false);
            reply->set_message("School name and school_id are requried");
            return Status(grpc::StatusCode::FAILED_PRECONDITION,reply->message());
        }
        if(detail.address().empty()||detail.email().empty()){
            reply->set_success(false);
            reply->set_message("Please add the school address and email");
            return Status(grpc::StatusCode::FAILED_PRECONDITION,reply->message());
        }
        std::string s_category=School::Category_Name(detail.category());
        if(s_category.empty()){
            reply->set_success(false);
            reply->set_message("Please add the school category e.g Primary school");
            return Status(grpc::StatusCode::FAILED_PRECONDITION,reply->message());
        }
        std::string s_name=detail.name();
        std::string s_address=detail.address();
        std::string s_id=detail.school_id();
        std::string s_email=detail.email();
        School::Category category=detail.category();
        std::string motto=detail.school_motto();

        std::string db_response=school_db.CreateSchool(s_name,s_address,s_id,s_email,category,motto);
        if(db_response.find("exist")!=std::string::npos||db_response.find("Failed")!=std::string::npos){
            reply->set_success(false);
            reply->set_message(db_response);
            return Status(grpc::StatusCode::NOT_FOUND,db_response);
        }
        reply->set_success(true);
        reply->set_message(db_response);
        return Status::OK;
    }
     catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::GetSchoolDetails(ServerContext* context,const School::SchoolDetailsRequest* request,School::SchoolDetails* reply){
    try{
        std::string s_id=request->school_id();
        if(s_id.empty()){
            return Status(grpc::StatusCode::INTERNAL,"school_id is required");
        }
        School::school c_school=school_db.GetSchoolDetails(s_id);
        School::school* detail=reply->mutable_details();
        detail->CopyFrom(c_school);
       
        return Status::OK;
    }
      catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
 Status SchoolServiceImpl::GetSchools(ServerContext* context,const School::SchoolsRequest* request,School::SchoolList* reply){
    try{
        if(request->role()!=Role::DEVELOPER){
            return Status(grpc::StatusCode::PERMISSION_DENIED,"Only developers can access the list of schools.");
        }
        auto a_schools=school_db.GetSchools();
        if(a_schools.schools().empty()){
            return Status(grpc::StatusCode::NOT_FOUND,"No schools found in the database.");
        }
        for(const auto& sch:a_schools.schools()){
            School::school* school=reply->add_schools();
            school->CopyFrom(sch);
        }
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
 }
Status SchoolServiceImpl::Invoice(ServerContext* context,const School::AddInvoiceRequest* request,Response* reply){
    try{
        School::Invoice invoice=request->invoice();
        if(invoice.amount()<=0||invoice.description().empty()){
            reply->set_success(false);
            reply->set_message("Amount and description fields are required!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        if(invoice.schoolid().empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        auto response=school_db.insertInvoice(invoice);
        reply->set_success(true);
        reply->set_message(response);
        return Status::OK;
    }
     catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::GetInvoice(ServerContext* context,const School::GetInvoiceRequest* request,School::InvoiceList* reply){
    try{
        std::string schoolid=request->schoolid();
        if(schoolid.empty()){
            return Status(grpc::StatusCode::INTERNAL,"school is not recognized");
        }
        auto response=school_db.getInvoicesBySchool(schoolid);
        if(response.empty()){
            return Status(grpc::StatusCode::NOT_FOUND,"No invoice found for the school");
        }
        for(const auto& inv:response){
            School::Invoice* invoice=reply->add_invoices();
            invoice->CopyFrom(inv);
        }
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
};
Status SchoolServiceImpl::Receipt(ServerContext* context,const School::AddReceiptRequest* request,Response* reply){
    try{
        School::Receipt receipt=request->receipt();
        if(receipt.schoolid().empty()){
            reply->set_success(false);
            reply->set_message("The school is not recorgnized!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        if(receipt.amount()<=0||receipt.invoiceid().empty()||receipt.receiptnumber().empty()){
            reply->set_success(false);
            reply->set_message("Amount, invoice ID and receipt number fields are required!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        School::paymentDetails pay=receipt.payment();
        if(pay.method().empty()||pay.transactioncode().empty()||pay.sendername().empty()){
            reply->set_success(false);
            reply->set_message("All payment details are required!");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        bool response=school_db.insertReceipt(receipt);
        if(!response){
            reply->set_success(false);
            reply->set_message("Failed to add receipt. Ensure the school exists.");
            return Status(grpc::StatusCode::INTERNAL,reply->message());
        }
        reply->set_success(true);
        reply->set_message("Receipt added successfully.");
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}
Status SchoolServiceImpl::GetReceipt(ServerContext* context,const School::GetReceiptRequest* request,School::ReceiptList* reply){
    try{
        std::string schoolid=request->schoolid();
        if(schoolid.empty()){
            return Status(grpc::StatusCode::INTERNAL,"school is not recognized");
        }
        auto response=school_db.getReceiptsBySchool(schoolid);
        if(response.empty()){
            return Status(grpc::StatusCode::NOT_FOUND,"No receipts found for the school");
        }
        for(const auto& rec:response){
            School::Receipt* receipt=reply->add_receipts();
            receipt->CopyFrom(rec);
        }
        return Status::OK;
    }
    catch(const std::runtime_error& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(const std::exception& e){
        return Status(grpc::StatusCode::INTERNAL,std::string(e.what()));
    }
    catch(...){
        return Status(grpc::StatusCode::INTERNAL,"Unexpected error occurred.");
    }
}

std::string LoadFile(const std::string& path){
    std::ifstream file(path,std::ios::in);
    std::string content;
    if(!file){
        std::cerr<<"Cannot open PEM file\n";
        return "";
    }
    std::string line;
    while(std::getline(file,line)){
        content+=line+"\n";
    }
    file.close();
    if(!content.empty()&&content.back()!='\n'){
        content+='\n';
    }
    std::cout<<"Loaded "<<path<<": "<<content.size()<<" bytes\n";
    return content;
}
void RunServer(){
    std::string server_address("0.0.0.0:50051");
    SchoolDB schooldb;
    auto handler=schooldb.getcollection("logs");
    auto activityLogger=std::make_shared<ActivityLogger>(handler.coll);
    SchoolServiceImpl service(activityLogger);
    auto interceptor=std::make_unique<ActivityInterceptorFactory>(activityLogger);
    ServerBuilder builder;  
    
    /*std::string cert_chain=LoadFile("/home/kevin-ouma/fullchain.crt");
    std::string cert_chain=LoadFile("/home/kevin-ouma/kevin-ouma.tail3aad27.ts.net.crt");
    std::string private_key=LoadFile("/home/kevin-ouma/kevin-ouma.tail3aad27.ts.net.key");
    if(cert_chain.empty()||private_key.empty()){
        std::cerr << "FATAL: Failed to load certificate or private key!" << std::endl;
        return;
    }
    //grpc::SslServerCredentialsOptions::PemKeyCertPair keycert;
    //keycert.cert_chain  = std::move(cert_chain);
    //keycert.private_key = std::move(private_key);
    //grpc::SslServerCredentialsOptions ssl_opts;
    ssl_opts.pem_key_cert_pairs.push_back(keycert);*/

    builder.AddListeningPort(server_address,grpc::InsecureServerCredentials());
    builder.RegisterService(&service);

    std::vector<std::unique_ptr<grpc::experimental::ServerInterceptorFactoryInterface>>creators;
    creators.push_back(std::move(interceptor));
    builder.experimental().SetInterceptorCreators(std::move(creators));

    //grpc::reflection::InitProtoReflectionServerBuilderPlugin();
    std::unique_ptr<Server> server(builder.BuildAndStart());
    if (!server) {
        std::cerr << "\n=== FATAL ERROR ===\n"
                  << "gRPC failed to start the secure server.\n"
                  << "This means the TLS credentials (cert + key) were rejected.\n"
                  << "Look for 'Invalid cert chain file' or similar messages above.\n"
                  << "===================\n" << std::endl;
        return;
    }
    std::cout<<"Secure gRPC server listening on "<<server_address<<std::endl;
    server->Wait();
}

int main() {
    try {
        absl::InitializeLog();
        mongocxx::instance instance{};
        
        std::cout << "Mongo instance created successfully" << std::endl;
        
        RunServer();
    }
    catch (const std::exception& e) {
        std::cerr << "\n=== UNCAUGHT EXCEPTION ===\n" << e.what() << std::endl;
    }
    catch (...) {
        std::cerr << "\n=== UNKNOWN CRASH ===\n" << std::endl;
    }
    
    std::cout << "Server stopped." << std::endl;
    return 0;
}