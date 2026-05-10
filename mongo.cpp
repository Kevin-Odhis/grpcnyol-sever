#include"mongo.h"
#include<map>
SchoolDB::SchoolDB():pool{mongocxx::uri{"mongodb+srv://kevinodhiambo891_db_user:5gv67fLFNPW3Sg89@nyoldbcluster.phce3ii.mongodb.net/?appName=NyolDBCluster"}}
{create_indexes();}
std::map<int,std::pair<std::string,int>> GetSubjects(const std::string& school_category){
    if(school_category=="JUNIOR"){
        std::map<int,std::pair<std::string,int>>subjects{
            {901,{"English",2}},
            {902,{"Kiswahili",2}},
            {903,{"Mathematics",1}},
            {905,{"Integrated Science",2}},
            {906,{"Agriculture",1}},
            {907,{"Social Studies",1}},
            {908,{"Christian Religious Education",1}},
            {911,{"Creative Arts & Sports",1}},
            {912,{"Pre-Technical Studies",1}}
        };
        return subjects;
    }else{
        std::map<int,std::pair<std::string,int>>subjects{
            {101,{"English",3}},
            {102,{"Kiswahili",3}},
            {121,{"Mathematics",2}},
            {231,{"Biology",3}},
            {232,{"Physics",3}},
            {233,{"Chemistry",3}},
            {311,{"History and Government", 2}},
            {312,{"Geography",2}},
            {313,{"Christian Religious Education",2}},
            {443,{"Agriculture",2}},
            {565,{"Business Studies",2}}
        };
        return subjects;
    }

}

std::string SchoolDB::CreateStudent(const std::string& name,const std::string& sex,const std::string& upi,const std::string& grade,
const std::string& school_id){
 auto handle=getcollection("students");
 auto& coll=handle.coll;

 auto grdhandle=getcollection("grades");
 auto& grdcoll=grdhandle.coll;
 
    try{
        auto stude_filter=make_document(kvp("upi",upi),kvp("school_id",school_id));
        auto cursor=coll.find_one(stude_filter.view());
        bsoncxx::builder::basic::array stude_subjects;
        if (cursor) {
            return "A learner already exist with upi/adm :"+upi+". Cannot add the student.";
        }
        auto find_grade=make_document(kvp("grade",grade),kvp("school_id",school_id));
        auto found=grdcoll.find_one(find_grade.view());
        if(!found){
            return grade+" not found, the learner not added! Add grade first";
        }
        
        /*auto grd_doc=found.value();
        if(grd_doc["subjects"]&&grd_doc["subjects"].type()==bsoncxx::type::k_array){
            auto sub_arr=grd_doc["subjects"].get_array().value;
            for(const auto& sub:sub_arr){
                if(sub["subject"].type()==bsoncxx::type::k_string){
                    std::string subject=std::string(sub["subject"].get_string().value);
                    stude_subjects.append(subject);
                }
            }
        }*/

    auto stude=make_document(kvp("name",name),kvp("sex",sex),kvp("upi",upi),kvp("grade_name",grade),kvp("school_id",school_id),
    kvp("subjects",make_array()),kvp("exams",make_array()));
    auto added=coll.insert_one(stude.view());
    if(!added||added->result().inserted_count()!=1){
        return "Could not add learner "+name+" to the system! Please try again.";
    }
    return name+" added successfully to the school database";
    }
    catch(const mongocxx::operation_exception& e){
        if(e.code().value()==11000){
            throw std::runtime_error("A student with the Adm/Upi "+upi+" already exist.");
        }
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
void SchoolDB::EditStudentDetails(const School::Student& learner,const std::string& school_id){
    auto handle=getcollection("students");
    auto& coll=handle.coll;
    try{
        std::string name=learner.name();
        std::string upi=learner.upi_no();
        std::string grade=learner.grade();
        std::string gender=School::Sex_Name(learner.gender());
        auto filter=make_document(kvp("upi",upi),kvp("school_id",school_id));
        auto update=make_document(kvp("$set",make_document(
            kvp("name",name),kvp("grade_name",grade),kvp("sex",gender)
        )));
        auto result=coll.update_one(filter.view(),update.view());
        if(!result||result->matched_count()==0){
            throw std::runtime_error("The student not found in the system!");
        }
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
void SchoolDB::AddSubjects_to_Student(const std::string& upi,const std::string& school_id,
            const std::vector<std::string>& subjects){
    auto handle=getcollection("students");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("upi",upi),kvp("school_id",school_id));
        bsoncxx::builder::basic::array sub_arr{};
        for(const auto& sub:subjects){
            sub_arr.append(sub);
        }
        auto update=make_document(kvp("$addToSet",make_document(kvp("subjects",make_document(kvp("$each",
        sub_arr.extract()))))));
        auto result=coll.update_one(filter.view(),update.view());
        if (!result || result->matched_count() == 0) {
            throw std::runtime_error("Student not found");
        }
        
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
std::string SchoolDB::DeleteStudentSubject(const std::string& upi,std::vector<std::string>&& subjects,const std::string& schoolid){
        auto handle=getcollection("students");
        auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("upi",upi),kvp("school_id",schoolid));
        bsoncxx::builder::basic::array subject_array;
        if(subjects.empty()) return "Kindly, select subject to remove";
        for(const auto& item:subjects){
            subject_array.append(item);
        }
        auto update=make_document(kvp("$pull",make_document(
            kvp("subjects",make_document(kvp("$in",subject_array.extract()))))));
        auto result=coll.update_one(filter.view(),update.view());
        if(!result||result->matched_count()==0){
            return "Student not found!";
        }
        if(result->modified_count()==0){
            return "No subject removed";
        }
        return "deleted successfully.";
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}

poolhandle SchoolDB::getcollection(const std::string& name){
    auto entry=pool.acquire();
    auto db=(*entry)["Schooldb"];
    auto coll=db[name];
    return poolhandle(std::move(entry),coll);
}

std::vector<Student> SchoolDB::FindStudents(const std::string& school_id){
    
    auto handle=getcollection("students");
    auto& coll=handle.coll;
    try{
        std::vector<Student> students;
        auto filter=make_document(kvp("school_id",school_id));
        auto cursor=coll.find(filter.view());
        if(cursor.begin()==cursor.end()){
            return students;
        }
        Sex no=Sex::Unspecified;
        for(auto& doc:cursor){
            Student stud;
            auto student_subject=doc["subjects"];
            if(student_subject&&student_subject.type()==bsoncxx::type::k_array){
                auto subject_arrays=student_subject.get_array().value;
                for(const auto& item:subject_arrays){
                    if(item.type()!=bsoncxx::type::k_string)continue;
                    auto subj=std::string(item.get_string().value);
                    stud.add_subjects(subj);
                }
            }
            if(doc["name"]&&doc["name"].type()==bsoncxx::type::k_string&&doc["upi"]&&doc["upi"].
            type()==bsoncxx::type::k_string&&doc["grade_name"]&&doc["grade_name"].type()==bsoncxx::type::k_string){
                stud.set_name(std::string(doc["name"].get_string().value));
                stud.set_upi_no(std::string(doc["upi"].get_string().value));
                stud.set_grade(std::string(doc["grade_name"].get_string().value));

                if(doc["sex"]&&doc["sex"].type()==bsoncxx::type::k_string){
                    std::string gender=std::string(doc["sex"].get_string().value);
                    if(Sex_Parse(gender,&no)){
                        stud.set_gender(no);
                    }
                }

                students.push_back(std::move(stud));
            }
            
        }
        return (std::move(students));
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}

std::string SchoolDB::DeleteStudent(const std::string& upi,const std::string& school_id){
    auto handle=getcollection("students");
    auto& coll=handle.coll;
    try{
        auto stude=make_document(kvp("upi",upi),kvp("school_id",school_id));
        auto doc=coll.find_one(stude.view());
        if(!doc)return "Learner with upi "+upi+ "not found!";
        std::string name=std::string(doc->view()["name"].get_string().value);
        coll.delete_one(stude.view());
        return "Learner "+name+" deleted successfully from the school database.";
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
 std::string SchoolDB::CreateTeacher(Teacher & teach,std::vector<std::pair<std::string,std::string>>&& grade_sub,
const std::string& user,const std::string& password,const std::string& school_id){
    auto handle=getcollection("teachers");
    auto& coll=handle.coll;
    try{
        auto filter_teacher=make_document(kvp("code",teach.code()),kvp("school_id",school_id));
       auto find=coll.find_one(filter_teacher.view());
       if(find){
        throw std::runtime_error("Another teacher exist with the same code, choose another code to add the teacher");
       }
        bsoncxx::builder::basic::array subjects;
        if(!grade_sub.empty()){
            for(const auto&[grade,subject]:grade_sub){
                subjects.append(make_document(kvp("grade_name",grade),kvp("subject",subject)));
            }
        }
        auto teacher=make_document(kvp("name",teach.name()),kvp("role",Role_Name(teach.role())),kvp("school_id",school_id),
        kvp("code",teach.code()),kvp("username",user),kvp("password",password),
        kvp("classes",subjects));
        coll.insert_one(teacher.view());
        return "Teacher "+teach.name()+" added successfully to the school database.";
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
 }

  std::string SchoolDB::DeleteTeacher(int code,const std::string& school_id){
    auto handle=getcollection("teachers");
    auto& coll=handle.coll;
    try{
        auto doc=coll.find_one(make_document(kvp("code",bsoncxx::types::b_int32{code}),kvp("school_id",school_id)).view());
        if(!doc)return "Teacher with code "+std::to_string(code)+" not found!";
        std::string name=std::string(doc->view()["name"].get_string().value);
        auto teach_doc=make_document(kvp("code",bsoncxx::types::b_int32{code}),kvp("school_id",school_id));
        coll.delete_one(teach_doc.view());
        return name+" deleted successfully from the school database.";
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
  }
  std::vector<Tutor> SchoolDB::FindTeachers(const std::string& school_id){
    std::vector<Tutor> teachers;
    auto handle=getcollection("teachers");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("school_id",school_id));

        mongocxx::options::find opt;
        auto ascend=make_document(kvp("code",1));
        opt.sort(ascend.view());
        auto cursor=coll.find(filter.view(),opt);
      for(auto &&doc:cursor){
       
        if(doc["name"]&&doc["name"].type()==bsoncxx::type::k_string&&doc["code"]&&doc["code"].type()==bsoncxx::type::k_int32){
             Tutor teacher;
            teacher.name=std::string(doc["name"].get_string().value);
            teacher.tutor_code=doc["code"].get_int32().value;
            if(doc["role"]&&doc["role"].type()==bsoncxx::type::k_string){
                std::string role_str=std::string(doc["role"].get_string().value);
                teacher.role=role_str;
            }

            if(doc["classes"]&&doc["classes"].type()==bsoncxx::type::k_array){
                auto arr=doc["classes"].get_array().value;
                for(const auto& elem:arr){
                    if(elem.type()==bsoncxx::type::k_document){
                        auto class_doc=elem.get_document().value;
                        if(class_doc["grade_name"]&&class_doc["grade_name"].type()==bsoncxx::type::k_string&&
                            class_doc["subject"]&&class_doc["subject"].type()==bsoncxx::type::k_string){
                           auto grade=std::string(class_doc["grade_name"].get_string().value);
                           auto subject=std::string(class_doc["subject"].get_string().value);
                            teacher.set_grade_subjects(grade,subject);
                        }else{continue;}
                    }
                }
            }
            teachers.push_back(std::move(teacher));
        }else{continue;}
        
      }
        return teachers;
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
  }
  void SchoolDB::create_indexes(){
   
    auto teacher_handle=getcollection("teachers");
    auto& teacher=teacher_handle.coll;

    auto school_handle=getcollection("school");
    auto& schools=school_handle.coll;

    auto log_handle=getcollection("logs");
    auto& logs=log_handle.coll;

    try{
       
        auto user_index=make_document(kvp("username",1));
       
        auto school_index=make_document(kvp("schoolId",1));
        mongocxx::options::index index_opt{};
        mongocxx::options::index expiry_opt{};
        expiry_opt.expire_after(std::chrono::seconds{0});
        index_opt.unique(true);
        
        teacher.create_index(user_index.view(),index_opt);
        schools.create_index(school_index.view(),index_opt);
        logs.create_index(make_document(kvp("expireAt",1)), expiry_opt);
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
  }
  std::string SchoolDB::DeleteTeacherSubject(const std::vector<School::GradeSub>&subjects,int code,const std::string& school_id){
    auto handle=getcollection("teachers");
    auto& coll=handle.coll;
    try{
        if(subjects.empty()){
            return "No subjects selected for deletion";
        }
        bsoncxx::builder::basic::array subject_array{};
        for(const auto& item:subjects){
            subject_array.append(make_document(kvp("grade_name",item.grade()),kvp("subject",item.subject())));
        }
        auto filter=make_document(kvp("code",bsoncxx::types::b_int32{code}),kvp("school_id",school_id));

        auto update=make_document(kvp("$pull",make_document(
            kvp("classes",make_document(kvp("$or",subject_array.extract())))
        )));
        auto result=coll.update_one(filter.view(),update.view());
        if(!result||result->matched_count()==0){
            return "The teacher not matched!";
        }
        if(result->modified_count()==0){
            return "No changes made in the teacher's classes!";
        }
        return std::to_string(subjects.size())+" deleted successfully";
    }
     catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
  }
  login_response SchoolDB::UserLogin(const std::string& username,const std::string& password){
    auto handle=getcollection("teachers");
    auto& coll=handle.coll;
    try{
        size_t pos=username.find('@');
        std::string school_id="";
        if(pos!=std::string::npos){
            school_id=username.substr(pos+1);
        }

        auto filter=make_document(kvp("username",username),kvp("school_id",school_id));
        auto result=coll.find_one(filter.view());
        if(!result){
            return login_response("Invalid username or password");
        }
        auto doc=result->view();
        if(!doc["password"]||doc["password"].type()!=bsoncxx::type::k_string)
        return login_response("Invalid user record!");

        std::string stored_password=std::string(doc["password"].get_string().value);

        if(stored_password!=password) return login_response("Invalid username or password");
        if(!doc["name"]||!doc["role"]||!doc["code"]) return login_response("Invalid user record!");

        Teacher teacher;
        auto name=std::string(doc["name"].get_string().value);
        auto code=doc["code"].get_int32().value;
        auto s_id=std::string(doc["school_id"].get_string().value);
        teacher.set_code(code);
        teacher.set_name(name);
        teacher.set_school_id(s_id);
        Role role;

        std::string t_role=std::string(doc["role"].get_string().value);
        if(Role_Parse(t_role,&role))
        teacher.set_role(role);
        return login_response(teacher);
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error(std::string("Operation failed: "+std::string(e.what())));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
  }
  std::string SchoolDB::CreateGrade(Grade&& grade,std::vector<std::pair<int,std::string>>&& subs,const std::string& school_id,
    const std::string& level){
    auto handle=getcollection("grades");
    auto& coll=handle.coll;
    try{
        auto grd_filter=make_document(kvp("grade",grade.name()),kvp("school_id",school_id));
        auto find=coll.find_one(grd_filter.view());
        if(find){
            throw std::runtime_error("A grade with the same name already exist.");
        }
        bsoncxx::builder::basic::array sub_array{};
        for(const auto& elem:subs){
            auto sub_doc=make_document(kvp("subject_code",bsoncxx::types::b_int32{elem.first}),kvp("subject",elem.second));
            sub_array.append(sub_doc.view());
        }
        auto doc=make_document(kvp("grade",grade.name()),kvp("teacher_code",bsoncxx::types::
            b_int32{grade.grade_teacher_code()}),kvp("school_id",school_id),kvp("level",level),
            kvp("subjects",sub_array),kvp("exams",make_array()));

        auto result=coll.insert_one(doc.view());
        if(!result||!result->result().inserted_count()){
            throw std::runtime_error("Failed to add grade "+grade.name()+" to the school database.");
        }
        return "Grade "+grade.name()+" added successfully to the school database.";
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error(std::string("Operation failed: "+std::string(e.what())));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
  }
  std::string SchoolDB::DeleteGrade(const std::string& grade_name){
    auto handle=getcollection("grades");
    auto& coll=handle.coll;
    try{
        auto doc=make_document(kvp("grade",grade_name));
        auto cursor=coll.find_one(doc.view());
        if(!cursor)return "Could not find "+grade_name+" in the system!";
        coll.delete_one(doc.view());
        return "Grade "+grade_name+" deleted successfully from the school database.";
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
  }
  std::vector<Return_Grade>SchoolDB::FindGrades(const std::string& school_id){
    auto handle=getcollection("grades");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("school_id",school_id));
        mongocxx::options::find opt;
        auto ascend=make_document(kvp("grade",1));
        opt.sort(ascend.view());
        auto cursor=coll.find(filter.view(),opt);
        std::vector<Return_Grade> grades;

        for(const auto& doc:cursor){
            
            if(!doc["grade"]||doc["grade"].type()!=bsoncxx::type::k_string)continue;
            Return_Grade grade;
            
            if(!doc["teacher_code"]||doc["teacher_code"].type()!=bsoncxx::type::k_int32){
            grade.set_grade_teacher_code(-1);}
            else{
                int code=doc["teacher_code"].get_int32().value;
                grade.set_grade_teacher_code(code);
                grade.set_grade_teacher_name(Teacher_Name(code,school_id));
            }
            
            std::string grdname=std::string(doc["grade"].get_string().value);
            if(doc["level"]&&doc["level"].type()==bsoncxx::type::k_string){
                std::string level=std::string(doc["level"].get_string().value);
                School::Category cate;
                if(School::Category_Parse(level,&cate)){
                    grade.set_level(cate);
                }
            }
            grade.set_name(grdname);
            Return_Grade lsubs=GetSubjects_Teacher(grdname,school_id);
            grade.mutable_l_areas()->CopyFrom(lsubs.l_areas());
            
            if(doc["subjects"].type()==bsoncxx::type::k_array){
                auto subject_array=doc["subjects"].get_array().value;

                for(const auto& sub_ele:subject_array){
                    if(sub_ele.type()!=bsoncxx::type::k_document)continue;
                    auto sub_doc=sub_ele.get_document().view();
                    if(sub_doc["subject"].type()!=bsoncxx::type::k_string)continue;
                    int code=sub_doc["subject_code"].get_int32().value;
                    std::string subject=std::string(sub_doc["subject"].get_string().value);
                    School::grade_subject* grdsub=grade.add_subjects();
                    grdsub->set_code(code);
                    grdsub->set_subject(subject);
                }
            }
            if(doc["exams"]&&doc["exams"].type()==bsoncxx::type::k_array){
                auto exam_array=doc["exams"].get_array().value;
                for(const auto& exa_ele:exam_array){
                    if(!exa_ele||exa_ele.type()!=bsoncxx::type::k_document)
                    continue;
                    auto exam_doc=exa_ele.get_document().value;
                    if(exam_doc["exam_name"]&&exam_doc["exam_name"].type()==bsoncxx::type::k_string){
                        std::string exam=std::string(exam_doc["exam_name"].get_string().value);
                        
                        School::Exam* grdexam=grade.add_exams();
                        grdexam->set_exam_name(exam);
                        if(!exam_doc["analyzed"]||exam_doc["analyzed"].type()!=bsoncxx::type::k_bool)continue;
                        auto status=exam_doc["analyzed"].get_bool().value;
                        bool full=false;
                        if(exam_doc["full_paper"]&&exam_doc["full_paper"].type()==bsoncxx::type::k_bool){
                            full=exam_doc["full_paper"].get_bool().value;
                        }
                        grdexam->set_analysed(status);
                        grdexam->set_full_paper(full);
                    }
                }
            }
            grades.push_back(grade);
        }
        return grades;
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
  }
  std::string SchoolDB::UpdateGrade(const std::string& name,const std::string& school_id,const std::string& new_grade,int code,
    const std::vector<std::pair<int,std::string>>& added_sub){
    auto handle=getcollection("grades");
    auto& coll=handle.coll;
    try{
        if(name.empty()){
            return "No grade selected for update! Please specify the name of the grade to update.";
        }
        bsoncxx::builder::basic::document update_doc{};
        bsoncxx::builder::basic::document set_doc{};
        bsoncxx::builder::basic::array sub_arr{};

        if(!new_grade.empty()) set_doc.append(kvp("grade",new_grade));
        if(code>0)set_doc.append(kvp("teacher_code",bsoncxx::types::b_int32{code}));
        if(!added_sub.empty()){
            for(const auto& subs:added_sub){
                auto doc=make_document(kvp("subject_code",bsoncxx::types::b_int32{subs.first}),
                kvp("subject",subs.second));
                sub_arr.append(doc.view());
            }
        }
        auto filter=make_document(kvp("grade",name),kvp("school_id",school_id));       
        if(!new_grade.empty()||code>0){update_doc.append(kvp("$set",set_doc));}

        if(!added_sub.empty()){
            update_doc.append(kvp("$addToSet",make_document(kvp("subjects",make_document(kvp("$each",sub_arr.view()))))));
        }
        auto result=coll.update_one(filter.view(),update_doc.view());
        if(!result||result->modified_count()==0)return "Update failed: No changes made to grade "+name+".";
        return "Grade "+name+" updated successfully in the school database.";
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
  }
  void SchoolDB::AddScore(const std::string& exam,const std::string& upi,const std::string& subject, int score,const std::string& school_id){
    auto handle=getcollection("students");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("upi",upi),kvp("school_id",school_id));
        std::string path="exams.$[examelem].scores."+subject;
        auto exist=coll.find_one(filter.view());
        bool found=false;
        if(exist){
            auto exa=exist->view();
            if(exa["exams"]&&exa["exams"].type()==bsoncxx::type::k_array){
                auto exa_arr=exa["exams"].get_array().value;
                for(const auto& ele:exa_arr){
                    if(ele.type()==bsoncxx::type::k_document){
                        auto exdoc=ele.get_document().value;
                        if(exdoc["exam_name"]&&exdoc["exam_name"].type()==bsoncxx::type::k_string){
                            auto name=std::string(exdoc["exam_name"].get_string().value);
                            if(name==exam) found=true;
                        }
                    }
                }
            }
        }
        if(found){
            auto update=make_document(kvp("$set",make_document(kvp(std::string(path),bsoncxx::types::b_int32{score}))));
            bsoncxx::builder::basic::array array_filter{};
            array_filter.append(make_document(kvp("examelem.exam_name",exam)));
            mongocxx::options::update update_options{};
            update_options.array_filters(array_filter.view());
            auto result=coll.update_one(filter.view(),update.view(),update_options);
            if(result&&result->modified_count()>0) return;
        }
        if(!found){
            auto exam_doc=make_document(kvp("exam_name",exam),kvp("scores",make_document(kvp(subject,
                bsoncxx::types::b_int32{score}))));
            auto push_exam=make_document(kvp("$push",make_document(kvp("exams",exam_doc))));
            auto result_up=coll.update_one(filter.view(),push_exam.view());
      }
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(...){
        throw std::runtime_error(std::string("Unknown error occured!"));
    }
  }
  void SchoolDB::AddScore(const std::string& exam,const std::string& upi,const std::pair<std::string,int>&score,const std::string& subj,
    const std::string& s_id,int out_of){
    auto handle=getcollection("students");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("upi",upi),kvp("school_id",s_id));
        auto update_doc=make_document(kvp("$set",
            make_document(kvp("exams.$[exam].subjects.$[sub].papers."+score.first+".score",score.second),
            kvp("exams.$[exam].subjects.$[sub].papers."+score.first+".out_of",out_of))));
        mongocxx::options::update option;
        bsoncxx::builder::basic::array array_filter;
        array_filter.append(make_document(kvp("exam.exam_name",exam)));
        array_filter.append(make_document(kvp("sub.subject",subj)));
        option.array_filters(array_filter.view());

        auto result=coll.update_one(filter.view(),update_doc.view(),option);
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
  }
  student_exam SchoolDB::StudentExamReport(const std::string& upi,const std::string& examname,const std::string& school_id){
    auto handle=getcollection("students");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("upi",upi),kvp("school_id",school_id));
        auto proj=make_document(kvp("name",1),kvp("grade_name",1),kvp("exams",1),kvp("_id",0));
        mongocxx::options::find opt{};
        opt.projection(proj.view());
        auto doc=coll.find_one(filter.view(),opt);
        if(!doc)return "Student With UPI/ADM "+upi+" not found!";
        auto stude=doc->view();

        StudentExam report;
        bool found_exam=false;
        if(stude["name"]&&stude["name"].type()==bsoncxx::type::k_string)
        report.student_name=std::string(stude["name"].get_string().value);
        if(stude["grade_name"]&&stude["grade_name"].type()==bsoncxx::type::k_string)
        report.grade=std::string(stude["grade_name"].get_string().value);
        report.upi=upi;
        if(stude["exams"]&&stude["exams"].type()==bsoncxx::type::k_array){
        auto exam_array=stude["exams"].get_array().value;
        for(const auto& element:exam_array){
            if(element.type()==bsoncxx::type::k_document){
                auto exam_doc=element.get_document().value;
                std::string Exam_Name{};
                if(exam_doc["exam_name"]&&exam_doc["exam_name"].type()==bsoncxx::type::k_string)
                Exam_Name=std::string(exam_doc["exam_name"].get_string().value);
                if(Exam_Name!=examname)continue;
                
                report.exam_name=Exam_Name;
                found_exam=true;
                
                if(exam_doc["scores"]&&exam_doc["scores"].type()==bsoncxx::type::k_document){
                    auto score_doc=exam_doc["scores"].get_document().view();
                    int total_score=0;
                    int subjects_count=0;
                    for(const auto& ele_score:score_doc){
                        if(ele_score.type()==bsoncxx::type::k_int32){
                            std::string subject=std::string(ele_score.key());
                            int score=ele_score.get_int32().value;
                            total_score+=score;
                            ++subjects_count;
                            std::string level=report.get_level(score);
                            report.set_subject_scores(subject,level,score);
                        }
                    }
                    report.total_marks=total_score;
                    report.level=report.get_level(report.total_marks,subjects_count);
                }
                
            }else{
                continue;}
        }
    }else{
        return "No exams found for "+report.student_name;
    }
    if(!found_exam)return "Exam "+examname+" not found for student "+report.student_name;
    return report;
}
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
  }
std::string SchoolDB::UpdateLearningAreas(int code,const std::string& school_id,const Role& role,std::vector<std::pair<std::string,
    std::string>>&& grade_sub){
    auto handle=getcollection("teachers");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("code",bsoncxx::types::b_int32{code}),kvp("school_id",school_id));
        auto roleupdate=make_document(kvp("$set",make_document(kvp("role",Role_Name(role)))));
        auto role_result=coll.update_one(filter.view(),roleupdate.view());
        if(!role_result||role_result->matched_count()==0){
            return "Update failed: No changes made to role of teacher with code "+std::to_string(code)+".";
        }
        bsoncxx::builder::basic::array doc{};
        for(const auto& [grade,sub]:grade_sub){
            doc.append(make_document(kvp("grade_name",grade),kvp("subject",sub)));
        }
        auto update=make_document(kvp("$push",make_document(kvp("classes",make_document(kvp("$each",doc))))));
        auto result=coll.update_one(filter.view(),update.view());
        if(!result||result->matched_count()==0){
            return "Update failed: No teacher found with code->"+std::to_string(code)+".";
        }
        return "Teacher with code "+std::to_string(code)+" updated successfully in the school database.";
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
void SchoolDB::Delete_TeacherSubjects(int code,const std::string& school_id,const l_areas_container& gradsubs){
    auto handle=getcollection("teachers");
    auto& coll=handle.coll;
try{
    auto filter=make_document(kvp("code",bsoncxx::types::b_int32{code}),kvp("school_id",school_id));
    bsoncxx::builder::basic::array doc{};
    for(const auto& [grad,sub]:gradsubs){
        doc.append(make_document(kvp("grade_name",grad),kvp("subject",sub)));
    }
    auto update=make_document(kvp("$pull",make_document(kvp("classes",make_document(kvp("$in",doc))))));
    auto deleted=coll.update_one(filter.view(),update.view());
    if(!deleted||deleted->modified_count()==0)
    throw std::runtime_error("No matching elements found for deletion.");
}
catch(const mongocxx::operation_exception& e){
    throw std::runtime_error("Operation failed: "+std::string(e.what()));
}
catch(const mongocxx::exception& e){
    throw std::runtime_error("Database error: "+std::string(e.what()));
}
catch(const std::exception& e){
    throw std::runtime_error("Internal error: "+std::string(e.what()));}
}

std::string SchoolDB::CreateExam(const std::string& exam_name,int term,std::vector<std::pair<std::string,bool>>&& grades,const std::string& school_id){
    auto handle=getcollection("grades");
    auto& coll=handle.coll;
    auto handl=getcollection("exam");
    auto& exam_coll=handl.coll;
    try{
        std::string added_grades;
        School::school s_category=GetSchoolDetails(school_id);
        std::map<int,std::pair<std::string,int>> subject_paper=GetSubjects(School::Category_Name(s_category.category()));
        
        auto exam_filter=make_document(kvp("exam",exam_name),kvp("school_id",school_id));
        auto find=exam_coll.find_one(exam_filter.view());
        if(find){
            throw std::runtime_error("Assessment with same name had already been created this term, choose another name for the assessment.");
        }
        auto exam_doc=make_document(kvp("exam",exam_name),kvp("school_id",school_id),kvp("createdAt",b_date{std::chrono::system_clock::now()}),
        kvp("term",term),kvp("analyzed",false));
        auto insert_result=exam_coll.insert_one(exam_doc.view());
        
        if(insert_result){
            for(const auto& grd:grades){

            bsoncxx::builder::basic::array loaded_subs{};
            auto doc=make_document(kvp("grade",grd.first),kvp("school_id",school_id));
            auto find_grade=coll.find_one(doc.view());
            if(!find_grade)continue;

            auto class_doc=find_grade->view();
            if(!class_doc["subjects"]||class_doc["subjects"].type()!=bsoncxx::type::k_array)continue;
            auto sub_arr=class_doc["subjects"].get_array().value;
            
            for(const auto& sub_ele:sub_arr){
                if(sub_ele.type()!=bsoncxx::type::k_document)continue;
                auto sub_doc=sub_ele.get_document().view();
                if(!sub_doc["subject_code"]||sub_doc["subject_code"].type()!=bsoncxx::type::k_int32)continue;
                int code=sub_doc["subject_code"].get_int32().value;

                auto it=subject_paper.find(code);
                if(it==subject_paper.end())continue;
                int papers=it->second.second;
                bsoncxx::builder::basic::document subpaper_doc;
                subpaper_doc.append(kvp("code",it->first),kvp("subject",it->second.first));
                
                bsoncxx::builder::basic::document papers_doc;
                for(int i=1;i<=papers;++i){
                    std::string paper="pp"+std::to_string(i);
                    papers_doc.append(kvp(paper,bsoncxx::types::b_bool{false}));
                }
                subpaper_doc.append(kvp("papers",papers_doc.extract()));
                loaded_subs.append(subpaper_doc.extract());
            }
        
            bsoncxx::builder::basic::document exam{};
            if(grd.second){
                exam.append(kvp("exam_name",exam_name),kvp("analyzed",false),kvp("full_paper",grd.second),
                kvp("loaded_subjects",loaded_subs.extract()));
            }else{
                exam.append(kvp("exam_name",exam_name),kvp("analyzed",false),kvp("full_paper",grd.second),
                kvp("loaded_subjects",make_array()));
            }

            auto filter=make_document(kvp("grade",grd.first),kvp("school_id",school_id),
                        kvp("exams",make_document(kvp("$not",make_document(kvp("$elemMatch",make_document(kvp("exam_name",exam_name))))))));

            auto exam_final = exam.extract();
            auto update=make_document(kvp("$addToSet",make_document(kvp("exams",exam_final.view()))));
            auto update_result=coll.update_one(filter.view(),update.view());

                if(update_result&&update_result->modified_count()>0){
                    added_grades+=grd.first+" ";
                    if(grd.second){
                        AddExamPapers(grd.first,school_id,exam_name);
                    }
                }
            }    
            
        }
        return exam_name+" created. Added to "+added_grades+" successfully.";
    }
    catch(const mongocxx::operation_exception& e){
        if(e.code().value()==11000){
            throw std::runtime_error("Exam already exists.");
        }
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
void SchoolDB::AddExamPapers(const std::string& grade,const std::string& s_id,const std::string& exam){
    auto handle = getcollection("students");
    auto& student_coll = handle.coll;

    try {
        auto find_filter = make_document(
            kvp("grade_name", grade),
            kvp("school_id", s_id)
        );

        auto cursor = student_coll.find(find_filter.view());

        std::map<std::string, int> exam_papers = {
            {"English", 3}, {"Kiswahili", 3}, {"Mathematics", 2},
            {"Biology", 3}, {"Physics", 3}, {"Chemistry", 3},
            {"History and Government", 2}, {"Geography", 2},
            {"Christian Religious Education", 2}, {"Agriculture", 2},
            {"Business Studies", 2}
        };

        std::vector<mongocxx::model::update_one> updates;
        std::vector<bsoncxx::document::value>   owned_filters;
        std::vector<bsoncxx::document::value>   owned_updates;

        for (const auto& student : cursor) {
            if (!student["subjects"] || student["subjects"].type() != bsoncxx::type::k_array) {
                continue;
            }
            auto student_subjects = student["subjects"].get_array().value;
            bsoncxx::builder::basic::array subjects_array;

            for (const auto& elem : student_subjects) {
                if (elem.type() != bsoncxx::type::k_string) continue;

                std::string subject{ elem.get_string().value };

                auto it = exam_papers.find(subject);
                if (it == exam_papers.end()) continue;

                int paper_count = it->second;

                bsoncxx::builder::basic::document papers_doc;
                for (int i = 1; i <= paper_count; ++i) {
                    papers_doc.append(kvp(
                        "pp" + std::to_string(i),
                        make_document(
                            kvp("score",   bsoncxx::types::b_int32{-1}),
                            kvp("out_of",  bsoncxx::types::b_int32{0})
                        )
                    ));
                }

                bsoncxx::builder::basic::document subject_doc;
                subject_doc.append(kvp("subject", subject));
                subject_doc.append(kvp("papers", papers_doc.extract()));

                subjects_array.append(subject_doc.extract());
            }
            if (subjects_array.view().empty()) continue;

            bsoncxx::builder::basic::document exam_doc;
            exam_doc.append(kvp("exam_name", exam));
            exam_doc.append(kvp("subjects", subjects_array.extract()));

            auto update_doc = make_document(
                kvp("$addToSet",
                    make_document(
                        kvp("exams", exam_doc.extract())
                    )
                )
            );
            auto filter_doc = make_document(
                kvp("_id", student["_id"].get_oid())
            );

            owned_filters.push_back(std::move(filter_doc));
            owned_updates.push_back(std::move(update_doc));

            mongocxx::model::update_one op(
                owned_filters.back().view(),
                owned_updates.back().view()
            );

            updates.push_back(std::move(op));
        }
        if (!updates.empty()) {
            mongocxx::options::bulk_write options;
            options.ordered(false);
            student_coll.bulk_write(updates, options);
        }
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
std::vector<StudentExam> SchoolDB::GetStudentsperSubject(const std::string& grade,const std::string& subj,
    int t_code,const std::string& examname,const std::string& school_id){
    auto handle=getcollection("teachers");
    auto &coll=handle.coll;
    try{
        mongocxx::pipeline pipe{};
        pipe.match(make_document(kvp("code",bsoncxx::types::b_int32{t_code}),kvp("school_id",school_id)));

        pipe.project(make_document(kvp("classes",make_document(kvp("$filter",make_document(kvp("input",
        "$classes"),kvp("as","ele"),kvp("cond",make_document(kvp("$and",make_array(make_document(kvp("$eq",
        make_array("$$ele.grade_name",grade))),
        make_document(kvp("$eq",make_array("$$ele.subject",subj)))))))))))));

        pipe.lookup(make_document(kvp("from","students"),
                                  kvp("localField","classes.grade_name"),
                                  kvp("foreignField","grade_name"),
                                  kvp("as","student_info")));

        pipe.unwind(make_document(kvp("path","$student_info")));

        pipe.project(make_document(kvp("_id",0),
                                   kvp("student_info.name",1),
                                   kvp("student_info.upi",1),
                                   kvp("student_info.exams",1)));
                                
    pipe.sort(make_document(kvp("student_info.upi",1)));
    mongocxx::options::aggregate a_opts;
    a_opts.allow_disk_use(true);
    auto cursor=coll.aggregate(pipe,a_opts);
    std::vector<StudentExam> learners;

    for(auto&& doc:cursor){
        StudentExam report;
        bool exam_found=false;
        bool sub_found=false;
        if(!doc["student_info"]||doc["student_info"].type()!=bsoncxx::type::k_document)continue;
        std::string s_name,s_upi;
        int score_value=0;
        auto student_doc=doc["student_info"].get_document().view();

        if(student_doc["name"]&&student_doc["name"].type()==bsoncxx::type::k_string&&student_doc["upi"]&&
             student_doc["upi"].type()==bsoncxx::type::k_string){
        s_name=std::string(student_doc["name"].get_string().value);
        s_upi=std::string(student_doc["upi"].get_string().value);
       }
                        // scan the exam array
       if(student_doc["exams"]&&student_doc["exams"].type()==bsoncxx::type::k_array){
        auto exam_array=student_doc["exams"].get_array().value;
        for(const auto& element:exam_array){
            if(element.type()!=bsoncxx::type::k_document)continue;
            auto exa_doc=element.get_document().view();
            if(exa_doc["exam_name"]&&exa_doc["exam_name"].type()==bsoncxx::type::k_string&&
            std::string(exa_doc["exam_name"].get_string().value)==examname){
                exam_found=true;

                if(exa_doc["scores"]&&exa_doc["scores"].type()==bsoncxx::type::k_document){
                    auto exa_scores=exa_doc["scores"].get_document().view();
                    for(const auto& s:exa_scores){
                        if(s.key()==subj&&s.type()==bsoncxx::type::k_int32){
                            sub_found=true;
                            score_value=s.get_int32().value;
                        }
                    }
                }
                
                break;
            }
        }
       }
       report.student_name=s_name;
       report.upi=s_upi;
       if(exam_found&&sub_found){
        report.total_marks=score_value;
       }
       learners.push_back(report);
    }
return learners;
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
std::vector<StudentExam> SchoolDB::GetStudentScoreInSubject(const std::string& grade,const std::string& subj,
    int t_code,const std::string& examname,const std::string& s_id,int pp){
    auto handle=getcollection("students");
    auto& student_coll=handle.coll;
    try{
        std::string paper_number="pp"+std::to_string(pp);
        std::vector<StudentExam>learners;
        auto filter_students=make_document(kvp("school_id",s_id),kvp("grade_name",grade),kvp("subjects",subj));
        auto cursor=student_coll.find(filter_students.view());

        for(const auto& doc:cursor){

            if(!doc["name"]||doc["name"].type()!=bsoncxx::type::k_string)continue;
            if(!doc["upi"]||doc["upi"].type()!=bsoncxx::type::k_string)continue;

            StudentExam student;
            std::string name=std::string(doc["name"].get_string().value);
            std::string upi=std::string(doc["upi"].get_string().value);
            student.student_name=name;
            student.upi=upi;

            if(!doc["exams"]||doc["exams"].type()!=bsoncxx::type::k_array)continue;
            auto exam_array=doc["exams"].get_array().value;
            for(const auto& element:exam_array){
                if(element.type()!=bsoncxx::type::k_document)continue;

                auto exam_doc=element.get_document().view();
                if(!exam_doc["exam_name"]||exam_doc["exam_name"].type()!=bsoncxx::type::k_string)continue;
                std::string exam_name=std::string(exam_doc["exam_name"].get_string().value);
                if(exam_name==examname){
                    if(!exam_doc["subjects"] || exam_doc["subjects"].type() != bsoncxx::type::k_array) continue;
                    auto sub_arr=exam_doc["subjects"].get_array().value;
                    for(const auto& sub_ele:sub_arr){
                        if(sub_ele.type()!=bsoncxx::type::k_document)continue;
                        auto sub_doc=sub_ele.get_document().view();
                        if(!sub_doc["subject"]||sub_doc["subject"].type()!=bsoncxx::type::k_string)continue;
                        std::string subject=std::string(sub_doc["subject"].get_string().value);
                        if(subject==subj){
                            if(!sub_doc["papers"]||sub_doc["papers"].type()!=bsoncxx::type::k_document)continue;
                            auto papers=sub_doc["papers"].get_document().view();
                            for(const auto& all:papers){
                                auto key=std::string(all.key());
                                if(key==paper_number){
                                auto paper=all.get_document().view();
                                student.pp_score=paper["score"].get_int32().value;
                                }
                                else if(key==paper_number){
                                    auto paper=all.get_document().view();
                                    student.pp_score=paper["score"].get_int32().value;
                                }
                                else if(key==paper_number){
                                    auto paper=all.get_document().view();
                                    student.pp_score=paper["score"].get_int32().value;
                                }
                            }
                            break;
                        }
                    }
                    break;
                }
            }
            learners.push_back(student);
        }
        return learners;
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
std::vector<StudentExam> SchoolDB::GetStudents_Mark(const std::string& grade,const std::string& subj,int t_code,
    const std::string& examname,const std::string& s_id){
        auto handle=getcollection("students");
        auto& student_coll=handle.coll;
        try{
            auto filter=make_document(kvp("grade_name",grade),kvp("school_id",s_id),kvp("subjects",subj));

            auto cursor=student_coll.find(filter.view());
            std::vector<StudentExam>learners;
            for(const auto& doc:cursor){
                if(!doc["name"]||doc["name"].type()!=bsoncxx::type::k_string)continue;
                if(!doc["upi"]||doc["upi"].type()!=bsoncxx::type::k_string)continue;
                StudentExam student;
                bool exam_found=false;
                bool sub_found=false;
                int score_value=-1;

                std::string student_name=std::string(doc["name"].get_string().value);
                std::string student_upi=std::string(doc["upi"].get_string().value);
                
                if(!doc["exams"]||doc["exams"].type()!=bsoncxx::type::k_array)continue;
                auto exam_array=doc["exams"].get_array().value;
                for(const auto& element:exam_array){
                    if(element.type()!=bsoncxx::type::k_document)continue;
                    auto exam_doc=element.get_document().view();
                    if(!exam_doc["exam_name"]||exam_doc["exam_name"].type()!=bsoncxx::type::k_string)continue;
                    std::string exam_name=std::string(exam_doc["exam_name"].get_string().value);

                    if(exam_name==examname){
                        exam_found=true;
                        if(exam_doc["scores"]&&exam_doc["scores"].type()==bsoncxx::type::k_document){
                            auto exa_scores=exam_doc["scores"].get_document().view();

                            for(const auto& s:exa_scores){
                                if(s.key()==subj&&s.type()==bsoncxx::type::k_int32){
                                    sub_found=true;
                                    score_value=s.get_int32().value;
                                }
                            }
                        }
                        break;
                    }

                }
                student.student_name=student_name;
                student.upi=student_upi;
                if(exam_found&&sub_found){
                    student.total_marks=score_value;
                }
                learners.push_back(student);
            }
            return learners;
        }
        catch(const mongocxx::operation_exception& e){
            throw std::runtime_error("Operation failed: "+std::string(e.what()));
        }
        catch(const mongocxx::exception& e){
            throw std::runtime_error("Database error: "+std::string(e.what()));
        }
        catch(const std::exception& e){
            throw std::runtime_error("Internal error: "+std::string(e.what()));
        }
    }
std::vector<School::Exam>SchoolDB::GetExams(const std::string& school_id){
    auto handle=getcollection("exam");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("school_id",school_id));
        std::vector<School::Exam>exam_status;
        mongocxx::options::find opt;
        auto sort_doc=make_document(kvp("createdAt",-1),kvp("_id",-1));
        opt.sort(sort_doc.view());
        auto cursor=coll.find(filter.view(),opt);
        for(auto& doc:cursor){
            
            if(doc["exam"]&&doc["exam"].type()==bsoncxx::type::k_string&&doc["term"]&&doc["term"].type()==bsoncxx::type::k_int32&&
            doc["analyzed"]&&doc["analyzed"].type()==bsoncxx::type::k_bool){
                 School::Exam assmnt;
                std::string exam_name=std::string(doc["exam"].get_string().value);
                int term=doc["term"].get_int32().value;
                bool analyzed=doc["analyzed"].get_bool().value;
               
                assmnt.set_exam_name(exam_name);
                assmnt.set_term(term);
                assmnt.set_analysed(analyzed);
                exam_status.push_back(assmnt);
            }
        }
        return exam_status;
    }
    catch(const mongocxx::operation_exception& e){
        throw(std::runtime_error("Operation failed: "+std::string(e.what())));
    }
    catch(const mongocxx::exception&e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
bool SchoolDB::ExamAnalyzed(const std::string& grdname,const std::string& exam_name,bool analysis,const std::string& school_id){
    auto handle=getcollection("grades");
    auto& coll=handle.coll;

    auto examhandle=getcollection("exam");
    auto& examcoll=examhandle.coll;
    try{
        auto exam_filter=make_document(kvp("exam",exam_name),kvp("school_id",school_id));
        auto exam_update=make_document(kvp("$set",make_document(kvp("analyzed",true))));

        auto filter=make_document(kvp("grade",grdname),kvp("school_id",school_id),kvp("exams.exam_name",exam_name));
        auto update=make_document(kvp("$set",make_document(kvp("exams.$.analyzed",analysis))));
        auto result=coll.update_one(filter.view(),update.view());
        auto response=examcoll.update_one(exam_filter.view(),exam_update.view());
        if(result&&result->modified_count()>0){
            return true;
        }
        return false;
    }
    catch(const mongocxx::operation_exception& e){
        throw(std::runtime_error("Operation failed: "+std::string(e.what())));
    }
    catch(const mongocxx::exception&e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}

std::vector<Student>SchoolDB::FindGradeStudents(const std::string& grade,const std::string& school_id){
    auto handle=getcollection("students");
    auto& coll=handle.coll;
    std::vector<Student>students;
    try{
        auto filter=make_document(kvp("grade_name",grade),kvp("school_id",school_id));
        auto cursor=coll.find(filter.view());

        Sex no=Sex::Unspecified;
        for(const auto& doc:cursor){
              Student stud;
            if(doc["name"]&&doc["name"].type()==bsoncxx::type::k_string&&doc["upi"]&&doc["upi"].
            type()==bsoncxx::type::k_string&&doc["grade_name"]&&doc["grade_name"].type()==bsoncxx::type::k_string){
                stud.set_name(std::string(doc["name"].get_string().value));
                stud.set_upi_no(std::string(doc["upi"].get_string().value));
                stud.set_grade(std::string(doc["grade_name"].get_string().value));

                if(doc["sex"]&&doc["sex"].type()==bsoncxx::type::k_string){
                    std::string gender=std::string(doc["sex"].get_string().value);
                    if(Sex_Parse(gender,&no)){
                        stud.set_gender(no);
                    }
                }

                students.push_back(std::move(stud));
            }
        }
        return students;
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed:"+ std::string(e.what()));
    }
    catch(const mongocxx::exception&e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
void SchoolDB::LoadedSubjects(const std::string& grade,const std::string& exam,const std::string& subject,
        const std::string& school_id,bool full_paper,int paper_number){
    auto handle=getcollection("grades");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("grade",grade),kvp("school_id",school_id),kvp("exams.exam_name",exam));
        if(full_paper){
            std::string paper_no="pp"+std::to_string(paper_number);
            auto update_doc=make_document(kvp("$set",make_document(
                kvp("exams.$[exam].loaded_subjects.$[subj].papers."+paper_no,bsoncxx::types::b_bool{true})
            )));
            bsoncxx::builder::basic::array array_filter{};
            array_filter.append(make_document(kvp("exam.exam_name",exam)));
            array_filter.append(make_document(kvp("subj.subject",subject)));

            mongocxx::options::update opt;
            opt.array_filters(array_filter.view());
            auto loaded_result=coll.update_one(filter.view(),update_doc.view(),opt);
            if(!loaded_result||loaded_result->matched_count()==0){
                throw std::runtime_error("Failed to update loaded subjects for the specified paper.");
            }
        }
        else{
            auto update_doc=make_document(kvp("$addToSet",make_document(kvp("exams.$.loaded_subjects",subject))));
            auto result=coll.update_one(filter.view(),update_doc.view());
            if(!result||result->matched_count()==0){
                throw std::runtime_error("Failed to update loaded subjects for the specified subject.");
            }
        }
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed:"+ std::string(e.what()));
    }
    catch(const mongocxx::exception&e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
std::vector<std::string> SchoolDB::get_loadedsubjects(const std::string& grade,const std::string& exam,const std::string& school_id){
    auto handle=getcollection("grades");
    auto &coll=handle.coll;
    try{
        auto filter=make_document(kvp("grade",grade),kvp("school_id",school_id),kvp("exams.exam_name",exam));
        auto result=coll.find_one(filter.view());
        if(!result){
            throw std::runtime_error("No grade or Assessment found !");
        }
        std::vector<std::string>LoadedSubjects;
        auto doc=result->view();
        if(!doc["exams"]||doc["exams"].type()!=bsoncxx::type::k_array)
        throw std::runtime_error("invalid Assessment structure");
        auto exams_array=doc["exams"].get_array().value;
        for(const auto& exa:exams_array){
            if(exa.type()!=bsoncxx::type::k_document)continue;
            auto exam_doc=exa.get_document().view();
            if(!exam_doc["exam_name"]||exam_doc["exam_name"].type()!=bsoncxx::type::k_string)continue;
            std::string examName=std::string(exam_doc["exam_name"].get_string().value);

            bool full_paper=false;
            if(exam_doc["full_paper"]&&exam_doc["full_paper"].type()==bsoncxx::type::k_bool){
                full_paper=exam_doc["full_paper"].get_bool().value;
            }
            if(examName!=exam)continue;
            auto loaded_elem=exam_doc["loaded_subjects"];
            if(!loaded_elem||loaded_elem.type()!=bsoncxx::type::k_array)continue;
            auto exa_arr=loaded_elem.get_array().value;

            if(full_paper){
                for(const auto& element:exa_arr){
                    if(element.type()!=bsoncxx::type::k_document)continue;
                    auto sub_doc=element.get_document().view();

                    if(!sub_doc["subject"]||sub_doc["subject"].type()!=bsoncxx::type::k_string)continue;
                    std::string subject=std::string(sub_doc["subject"].get_string().value);
                    subject+=": ";
                    if(!sub_doc["papers"]||sub_doc["papers"].type()!=bsoncxx::type::k_document)continue;
                    auto papers=sub_doc["papers"].get_document().view();
                     bool unloaded=false;
                    for(const auto& ele:papers){
                        if(ele.type()!=bsoncxx::type::k_bool)continue;
                        if(!ele.get_bool().value){
                            unloaded=true;
                            subject+=std::string(ele.key())+" ";
                        }
                        
                    }
                    if(unloaded){
                        if(subject.back()==' '){
                            subject.pop_back();
                        }
                        LoadedSubjects.push_back(subject);
                    }
                }
            }
            else{
                for(const auto& ele:exa_arr){
                    if(ele.type()==bsoncxx::type::k_string){
                        std::string subj=std::string(ele.get_string().value);
                        LoadedSubjects.push_back(subj);
                    }
                }
            }
            break;
        }
        return LoadedSubjects;
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed:"+ std::string(e.what()));
    }
    catch(const mongocxx::exception&e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}

School::MeritList SchoolDB::FetchGradeMeritList(const std::string& grade_name,const std::string& exam_name,const std::string& school_id){
    auto handle=getcollection("students");
    auto& coll=handle.coll;
    try{
        
        auto filter_doc=make_document(kvp("grade_name",grade_name),kvp("school_id",school_id));
        auto cursor=coll.find(filter_doc.view());
        School::MeritList studentlist;
        for(const auto& doc:cursor){
            bool exam_found=false;
            School::StudentMerit* stude=nullptr;
            std::map<std::string,int>subjects_done;

        if(doc["subjects"]&&doc["subjects"].type()==bsoncxx::type::k_array){
            auto sub_array=doc["subjects"].get_array().value;
            for(const auto& element:sub_array){
                if(element.type()!=bsoncxx::type::k_string)continue;
                std::string subject=std::string(element.get_string().value);
                subjects_done[subject]=-1;
            }
        }
        if(!doc["exams"]||doc["exams"].type()!=bsoncxx::type::k_array)continue;
        auto Exam_array=doc["exams"].get_array().value;
        for(const auto& element:Exam_array){
            if(element.type()!=bsoncxx::type::k_document)continue;
            auto examdoc=element.get_document().view();
            if(!examdoc["exam_name"]||examdoc["exam_name"].type()!=bsoncxx::type::k_string)continue;
            std::string exam="";
            exam=std::string(examdoc["exam_name"].get_string().value);
            if(exam!=exam_name)continue;
            exam_found=true;
            
            if(!examdoc["scores"]||examdoc["scores"].type()!=bsoncxx::type::k_document)continue;
            if(!stude)stude=studentlist.add_learners();

            auto all_cores=examdoc["scores"].get_document().view();
            for(const auto& s:all_cores){
                if(s.type()==bsoncxx::type::k_int32){
                    School::SubjectScore* sub_score=stude->add_scores();
                    std::string sub=std::string(s.key());
                    sub_score->set_subject(sub);
                    int mark=s.get_int32().value;
                    sub_score->set_score(mark);
                    auto it=subjects_done.find(sub);
                    if(it!=subjects_done.end()){
                        subjects_done.erase(it);
                    }

                    if(sub=="Mathematics"||sub=="Physics"||sub=="Chemistry"||sub=="Biology"){
                        sub_score->set_level(Science_Math_Grading(mark,school_id));
                    }else{
                        std::string lev=getsubject_level(mark,school_id);
                        sub_score->set_level(lev);
                    }
                    
                }
            }
        }
        if(!exam_found)continue;
        if(doc["name"]&&doc["name"].type()==bsoncxx::type::k_string){
            std::string name=std::string(doc["name"].get_string().value);
            stude->set_student_name(name);
            if(subjects_done.size()>0){
                for(const auto& sub:subjects_done){
                    School::SubjectScore* sub_score=stude->add_scores();
                    sub_score->set_subject(sub.first);
                    sub_score->set_score(-1);
                    sub_score->set_level(getsubject_level(-1,school_id));
                }
            }
        }
        if(doc["upi"]&&doc["upi"].type()==bsoncxx::type::k_string){
            std::string upi=std::string(doc["upi"].get_string().value);
            stude->set_upi(upi);
        }
        
        Sex sex;
        if(doc["sex"]&&doc["sex"].type()==bsoncxx::type::k_string){
            std::string gender=std::string(doc["sex"].get_string().value);
            if(Sex_Parse(gender,&sex)) stude->set_sex(sex);
          }
        }
        return studentlist;
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
std::string SchoolDB::Science_Math_Grading(int marks,const std::string&s_id){
    School::school c_s=GetSchoolDetails(s_id);

    if(c_s.category()==School::Category::SECONDARY){
        if(marks>=75)return "A";
        else if(marks>=70) return "A-";
        else if(marks>=65) return "B+";
        else if(marks>=60)return "B";
        else if(marks>=55) return "B-";
        else if(marks>=50)return "C+";
        else if(marks>=45)return "C";
        else if(marks>=40)return "C-";
        else if(marks>=30)return "D+";
        else if(marks>=20) return "D";
        else if(marks>=14) return "D-";
        else if(marks>=0) return "E";
        else{return "X";}
    }
    else{
         if(marks>=90)return "EE1";
        else if(marks>=75) return "EE2";
        else if(marks>=58)return "ME1";
        else if(marks>=41)return "ME2";
        else if(marks>=31)return "AE1";
        else if(marks>=21)return "AE2";
        else if(marks>=11)return "BE1";
        else if(marks>=0)return "BE2";
        else return "X";
    }
}
std::string SchoolDB::getsubject_level(int marks,const std::string& s_id){
    School::school c_s=GetSchoolDetails(s_id);
    if(c_s.category()==School::Category::SECONDARY){
        if(marks>=80)return "A";
        else if(marks>=75) return "A-";
        else if(marks>=70)return "B+";
        else if(marks>=65)return "B";
        else if(marks>=60)return "B-";
        else if(marks>=55)return "C+";
        else if(marks>=50)return "C";
        else if(marks>=40)return "C-";
        else if(marks>=35)return "D+";
        else if(marks>=30)return "D";
        else if(marks>=20)return "D-";
        else if(marks>=0)return "E";
        else return "X";

    }else{
        if(marks>=90)return "EE1";
        else if(marks>=75) return "EE2";
        else if(marks>=58)return "ME1";
        else if(marks>=41)return "ME2";
        else if(marks>=31)return "AE1";
        else if(marks>=21)return "AE2";
        else if(marks>=11)return "BE1";
        else if(marks>=0)return "BE2";
        else return "X";
    }
}
std::string SchoolDB::ResetUserPassword(const std::string& username,const std::string& newpassword){
    auto handle=getcollection("teachers");
    auto& coll=handle.coll;
    try{
        size_t pos=username.find('@');
        std::string school_id;
        if(pos!=std::string::npos){
            school_id=username.substr(pos+1);
        }else{
            throw std::runtime_error("The username doesnt exist!");
        }

        auto filter=make_document(kvp("username",username),kvp("school_id",school_id));
        auto update=make_document(kvp("$set",make_document(kvp("password",newpassword))));

        auto result=coll.update_one(filter.view(),update.view());
        if(!result||result->matched_count()==0){
            return "Password reset failed: No user found with username "+username+".";
        }
        return "Password for user "+username+" has been reset successfully.";
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
Return_Grade SchoolDB::GetSubjects_Teacher(const std::string& grade,const std::string& school_id) {
    auto handle = getcollection("teachers");
    auto& coll = handle.coll;
    Return_Grade grade_info;
    try {
        mongocxx::pipeline pipeline;

        pipeline.match(
            make_document(kvp("classes.grade_name", grade),kvp("school_id",school_id))
        );

        //Project name, code, and SUBJECTS (not classes!)
        pipeline.project(
            make_document(
                kvp("name", 1),
                kvp("code", 1),
                kvp("subjects",
                    make_document(
                        kvp("$map",
                            make_document(
                                kvp("input",
                                    make_document(
                                        kvp("$filter",
                                            make_document(
                                                kvp("input", "$classes"),
                                                kvp("as", "cls"),
                                                kvp("cond",
                                                    make_document(
                                                        kvp("$eq",
                                                            make_array("$$cls.grade_name", grade)
                                                        )
                                                    )
                                                )
                                            )
                                        )
                                    )
                                ),
                                kvp("as", "c"),
                                kvp("in", "$$c.subject")
                            )
                        )
                    )
                )
            )
        );
        auto cursor = coll.aggregate(pipeline);

        //Read results
        for (auto&& doc : cursor) {
            int code = doc["code"].get_int32().value;
            std::string tname = std::string(doc["name"].get_string().value);

            auto subjects = doc["subjects"].get_array().value;

            for (const auto& sub : subjects) {
                School::teacher_subject* l = grade_info.add_l_areas();
                std::string subb=std::string(sub.get_string().value);
                l->set_subject(subb);
                l->set_teacher_name(tname);
                l->set_teacher_code(code);
            }
        }
    }
    catch (const mongocxx::operation_exception& e) {
        throw std::runtime_error("Operation failed: " + std::string(e.what()));
    }
    catch (const mongocxx::exception& e) {
        throw std::runtime_error("Database error: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Internal error: " + std::string(e.what()));
    }
    return grade_info;
}
std::string SchoolDB::Teacher_Name(int code,const std::string& school_id){
    auto handle=getcollection("teachers");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("code",bsoncxx::types::b_int32{code}),kvp("school_id",school_id));
        auto result= coll.find_one(filter.view());
        if(!result) return "";
        auto doc=result->view();
        std::string return_name="";
        if(doc["name"]&&doc["name"].type()==bsoncxx::type::k_string){
            return_name=std::string(doc["name"].get_string().value);
        }
        return return_name;
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
std::string SchoolDB::Delete_GradeSubjects(const std::string& grade,const std::string& school_id,
        const std::vector<std::pair<int,std::string>>& subjects){
    auto handle=getcollection("grades");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("grade",grade),kvp("school_id",school_id));
        if(subjects.empty()) return "Provide at least one learning area to be removed.";
        bsoncxx::builder::basic::array arr_sub{};

        for(const auto& item:subjects){
            auto doc=make_document(kvp("subject_code",bsoncxx::types::b_int32{item.first}),
            kvp("subject",item.second));
            arr_sub.append(doc.view());
        }
        auto update=make_document(kvp("$pull",make_document(kvp("subjects",
        make_document(kvp("$or",arr_sub.view()))))));

        auto result=coll.update_one(filter.view(),update.view());
        if(!result||result->modified_count()==0){
            return "No matching subjects found for deletion in grade "+grade+".";
        }
        return "Specified subjects deleted successfully from grade "+grade+" in the school database.";
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
std::string SchoolDB::CreateSchool(const std::string& name,const std::string& addr,const std::string& id,
        const std::string& email,const School::Category& cate,const std::string& motto){
    auto handle=getcollection("school");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("schoolId",id));
        auto result=coll.find_one(filter.view());
        if(result){
            return "The school_id already exist, select a new one";
        }
        auto doc=make_document(kvp("name",name),kvp("address",addr),kvp("email",email),kvp("motto",motto),
        kvp("category",School::Category_Name(cate)),kvp("schoolId",id),kvp("school_head",""));

        auto reply=coll.insert_one(doc.view());
        if(!reply||!reply->result().inserted_count()){
            return "Failed to add School, try again";
        }
        return (name+" created successfully");
    }
    catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
School::school SchoolDB::GetSchoolDetails(const std::string& school_id){
    auto handle=getcollection("school");
    auto& coll=handle.coll;
    try{
        auto filter=make_document(kvp("schoolId",school_id));
        auto doc=coll.find_one(filter.view());
        if(!doc){
            throw std::runtime_error("The school does not exist in the database!");
        }
        auto view=doc->view();
        School::school s_details;
        s_details.set_address(std::string(view["address"].get_string().value));
        s_details.set_name(std::string(view["name"].get_string().value));
        s_details.set_email(std::string(view["email"].get_string().value));
        s_details.set_school_id(std::string(view["schoolId"].get_string().value));
        s_details.set_school_motto(std::string(view["motto"].get_string().value));
        if(view["school_head"]&&view["school_head"].type()==bsoncxx::type::k_string){
            s_details.set_school_head(std::string(view["school_head"].get_string().value));
        }
        std::string cate=std::string(view["category"].get_string().value);
        School::Category catego=School::Category::SECONDARY;
        if(School::Category_Parse(cate,&catego)){
            s_details.set_category(catego);
        }

        if(view["subscription_status"]&&view["subscription_status"].type()==bsoncxx::type::k_string){
            School::SubscriptionStatus status=School::SubscriptionStatus::GRACE;
            std::string subs_status=std::string(view["subscription_status"].get_string().value);
            if(School::SubscriptionStatus_Parse(subs_status,&status)){
                s_details.set_status(status);
            }
        }
        return s_details;
    }
     catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
School::SchoolList SchoolDB::GetSchools(){
    auto handle=getcollection("school");
    auto& coll=handle.coll;
    try{
        School::SchoolList schools;
        auto cursor=coll.find({});
        for(const auto& doc:cursor){
            School::school* s=schools.add_schools();
            s->set_address(std::string(doc["address"].get_string().value));
            s->set_name(std::string(doc["name"].get_string().value));
            s->set_email(std::string(doc["email"].get_string().value));
            s->set_school_id(std::string(doc["schoolId"].get_string().value));
            s->set_school_motto(std::string(doc["motto"].get_string().value));
            if(doc["school_head"]&&doc["school_head"].type()==bsoncxx::type::k_string){
                s->set_school_head(std::string(doc["school_head"].get_string().value));
            }
            std::string cate=std::string(doc["category"].get_string().value);
            School::Category catego=School::Category::SECONDARY;
            if(School::Category_Parse(cate,&catego)){
                s->set_category(catego);
            }
            if(doc["subscription_status"]&&doc["subscription_status"].type()==bsoncxx::type::k_string){
                School::SubscriptionStatus status=School::SubscriptionStatus::GRACE;
                std::string subs_status=std::string(doc["subscription_status"].get_string().value);
                if(School::SubscriptionStatus_Parse(subs_status,&status)){
                    s->set_status(status);
                }
            }
        }
        return schools;
    }
     catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}
School::MeritList SchoolDB::GetGradeMeritList(const std::string& grade,const std::string& exam,const std::string& s_id){
    auto handle=getcollection("students");
    auto& coll=handle.coll;
    try{
        School::MeritList students;
        auto filter=make_document(kvp("grade_name",grade),kvp("school_id",s_id));
        auto cursor=coll.find(filter.view());
        for(const auto& doc:cursor){
            if(!doc["name"]||doc["name"].type()!=bsoncxx::type::k_string)continue;
            std::string name=std::string(doc["name"].get_string().value);
            std::string sex=std::string(doc["sex"].get_string().value);
            std::string upi=std::string(doc["upi"].get_string().value);

            School::StudentMerit* student=students.add_learners();
            student->set_upi(upi);
            student->set_student_name(name);
            School::Sex gender=School::Sex::Unspecified;
            if(School::Sex_Parse(sex,&gender)){
                student->set_sex(gender);
            }

            if(!doc["exams"]||doc["exams"].type()!=bsoncxx::type::k_array)continue;
            auto exam_array=doc["exams"].get_array().value;

            for(const auto& element:exam_array){
                if(element.type()!=bsoncxx::type::k_document)continue;
                auto exams_doc=element.get_document().view();
                if(!exams_doc["exam_name"]||exams_doc["exam_name"].type()!=bsoncxx::type::k_string)continue;
                std::string exam_name=std::string(exams_doc["exam_name"].get_string().value);
                if(exam==exam_name){

                    if(exams_doc["subjects"].type()!=bsoncxx::type::k_array)continue;
                    auto subject_array=exams_doc["subjects"].get_array().value;
                    for(const auto& sub_ele:subject_array){
                        if(sub_ele.type()!=bsoncxx::type::k_document)continue;
                        auto sub_doc=sub_ele.get_document().view();
                        if(!sub_doc["subject"]||sub_doc["subject"].type()!=bsoncxx::type::k_string)continue;

                        std::string subject=std::string(sub_doc["subject"].get_string().value);
                        if(sub_doc["papers"].type()!=bsoncxx::type::k_document)continue;
                        auto papers=sub_doc["papers"].get_document().view();
                        int score{};
                        int out_of{};
                        School::SubjectScore* each_subject=student->add_scores();
                        each_subject->set_subject(subject);
                        for(const auto& paper:papers){
                            if(paper.type()!=bsoncxx::type::k_document)continue;
                            auto sc_doc=paper.get_document().view();

                            if(!sc_doc["score"]||sc_doc["score"].type()!=bsoncxx::type::k_int32)continue;
                            if(!sc_doc["out_of"]||sc_doc["out_of"].type()!=bsoncxx::type::k_int32)continue;
                            int mark=sc_doc["score"].get_int32().value;
                            if(mark>=0)score+=mark;
                            int max_score=sc_doc["out_of"].get_int32().value;
                            if(max_score>0)out_of+=max_score;
                        }
                        int sub_score=0;
                        if(out_of>0&&score>=0){
                            double percentage=(double(score)/out_of)*100;
                            sub_score=static_cast<int>(std::round(percentage));
                        }else{
                            sub_score=-1;
                        }
                        each_subject->set_score(sub_score);
                        if(subject=="Mathematics"||subject=="Physics"||subject=="Biology"||subject=="Chemistry"){
                            each_subject->set_level(Science_Math_Grading(sub_score,s_id));
                        }else{
                            each_subject->set_level(getsubject_level(sub_score,s_id));
                        }
                       
                        
                    }
                    break;
                }
            }
        }
        return students;
    }
     catch(const mongocxx::operation_exception& e){
        throw std::runtime_error("Operation failed: "+std::string(e.what()));
    }
    catch(const mongocxx::exception& e){
        throw std::runtime_error("Database error: "+std::string(e.what()));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Internal error: "+std::string(e.what()));
    }
}

std::string SchoolDB::insertInvoice(const School::Invoice& inv) {
    try {
        auto handle=getcollection("invoices");
        auto& collection=handle.coll;

        auto doc=make_document(kvp("invoiceNumber",inv.invoicenumber()),
            kvp("school_id",inv.schoolid()),
            kvp("status",School::InvoiceStatus_Name(inv.status())),
            kvp("billingYear",inv.billingyear()),
            kvp("amount",inv.amount()),
            kvp("description",inv.description()),
            kvp("startDate",bsoncxx::types::b_date(std::chrono::milliseconds(static_cast<int64_t>(inv.startdate().seconds() * 1000)))),
            kvp("dueDate",bsoncxx::types::b_date(std::chrono::milliseconds(static_cast<int64_t>(inv.duedate().seconds() * 1000)))),
            kvp("createdAt",bsoncxx::types::b_date(std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now().time_since_epoch())))
        );

        auto result = collection.insert_one(doc.view());

        if (!result) {
            return "Invoice insert failed";
        }

        return "Invoice inserted successfully";

    }catch(const mongocxx::operation_exception& e) {
        throw std::runtime_error("Operation failed: " + std::string(e.what()));
    } 
    catch (const std::exception& e) {
        throw std::runtime_error("Error inserting invoice: " + std::string(e.what()));
    }
}
bool SchoolDB::insertReceipt(const School::Receipt& r){
    try {
        auto handle=getcollection("receipts");
        auto& collection=handle.coll;

        auto paymentDoc=make_document(kvp("method",r.payment().method()),
            kvp("transactionCode",r.payment().transactioncode()),
            kvp("sender",r.payment().sendername()));

        auto doc=make_document(kvp("school_id",r.schoolid()),
             kvp("receiptNumber",r.receiptnumber()),
             kvp("invoice_id",r.invoiceid()),
             kvp("amount",bsoncxx::types::b_double{r.amount()}),
             kvp("date",bsoncxx::types::b_date(std::chrono::milliseconds(static_cast<int64_t>(r.date().seconds() * 1000)))),
             kvp("payment",paymentDoc.view()),
             kvp("description",r.description()),
             kvp("amountinwords",r.amountinwords()),
             kvp("createdAt",bsoncxx::types::b_date(std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()
            )))
        );

        auto result = collection.insert_one(doc.view());

        if (!result) {
            return false;
        }
        //set invoice status paid
        auto handle_inv=getcollection("invoices");
        auto& coll_inv=handle_inv.coll;
        auto filter=make_document(kvp("invoiceNumber",r.invoiceid()),kvp("school_id",r.schoolid()));
        School::InvoiceStatus set_status=School::InvoiceStatus::INVOICE_PAID;

        auto update=make_document(kvp("$set",make_document(kvp("status",School::InvoiceStatus_Name(set_status)))));
        coll_inv.update_one(filter.view(),update.view());
        return true;

    }catch(const mongocxx::operation_exception& e) {
        throw std::runtime_error("Operation failed: " + std::string(e.what()));
    }
    catch(const mongocxx::exception& e) {
        throw std::runtime_error("Database error: " + std::string(e.what()));
    }
    catch (const std::exception& e) {
        throw std::runtime_error("Error inserting receipt: " + std::string(e.what()));
    }
}
std::vector<School::Invoice> SchoolDB::getInvoicesBySchool(const std::string& schoolId) {
    std::vector<School::Invoice> result;
    try {
        auto handle = getcollection("invoices");
        auto& collection = handle.coll;

        auto filter = make_document(kvp("school_id", schoolId));
        auto cursor = collection.find(filter.view());

        for (auto&& doc : cursor) {
            try {
                School::Invoice inv;

                if (doc["invoiceNumber"])
                    inv.set_invoicenumber(std::string(doc["invoiceNumber"].get_string().value));

                if (doc["school_id"])
                    inv.set_schoolid(std::string(doc["school_id"].get_string().value));

                if (doc["status"]) {
                    std::string inv_status = std::string(doc["status"].get_string().value);
                    School::InvoiceStatus status;
                    if (!School::InvoiceStatus_Parse(inv_status, &status)) {
                        throw std::runtime_error("Invalid invoice status: " + inv_status);
                    }
                    inv.set_status(status);
                }
                if (doc["startDate"]) {
                    auto millis = doc["startDate"].get_date().value.count();

                    google::protobuf::Timestamp ts;
                    ts.set_seconds(millis / 1000);
                    ts.set_nanos((millis % 1000) * 1000000);
                    *inv.mutable_startdate() = ts;
                }
                if (doc["dueDate"]) {
                    auto millis = doc["dueDate"].get_date().value.count();

                    google::protobuf::Timestamp ts;
                    ts.set_seconds(millis / 1000);
                    ts.set_nanos((millis % 1000) * 1000000);
                    *inv.mutable_duedate() = ts;
                }

                if (doc["billingYear"])
                    inv.set_billingyear(doc["billingYear"].get_int32().value);

                if (doc["amount"]) inv.set_amount(doc["amount"].get_double().value);

                if (doc["description"])
                    inv.set_description(std::string(doc["description"].get_string().value));

                result.push_back(inv);

            } catch (const std::exception& e) {
                continue;
            }
        }

    } catch (const mongocxx::operation_exception& e) {
        throw std::runtime_error("Operation failed: " + std::string(e.what()));
    } catch (const mongocxx::exception& e) {
        throw std::runtime_error("Database error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Error fetching invoices: " + std::string(e.what()));
    }

    return result;
}
std::vector<School::Receipt> SchoolDB::getReceiptsBySchool(const std::string& schoolId) {
    std::vector<School::Receipt> result;
    try {
        auto handle = getcollection("receipts");
        auto& collection = handle.coll;

        auto filter = make_document(kvp("school_id", schoolId));
        auto cursor = collection.find(filter.view());

        for (auto&& doc : cursor) {
            try {
                School::Receipt r;
                // --- Basic fields ---
                if (doc["receiptNumber"])
                    r.set_receiptnumber(std::string(doc["receiptNumber"].get_string().value));

                if (doc["school_id"])
                    r.set_schoolid(std::string(doc["school_id"].get_string().value));

                if (doc["invoice_id"])
                    r.set_invoiceid(std::string(doc["invoice_id"].get_string().value));

                if (doc["amount"])r.set_amount(doc["amount"].get_double().value);
                if(doc["amountinwords"]&&doc["amountinwords"].type()==bsoncxx::type::k_string){
                    r.set_amountinwords(std::string(doc["amountinwords"].get_string().value));
                }

                // --- Date conversion (BSON -> Protobuf Timestamp) ---
                if (doc["date"]) {
                    auto millis = doc["date"].get_date().value.count();

                    google::protobuf::Timestamp* ts = r.mutable_date();
                    ts->set_seconds(millis / 1000);
                    ts->set_nanos((millis % 1000) * 1000000);
                }
                if(doc["description"]&&doc["description"].type()==bsoncxx::type::k_string){
                    r.set_description(std::string(doc["description"].get_string().value));
                }

                // --- Nested payment ---
                if (doc["payment"] && doc["payment"].type() == bsoncxx::type::k_document) {
                    auto paymentView = doc["payment"].get_document().view();

                    auto* payment = r.mutable_payment();

                    if (paymentView["method"])
                        payment->set_method(std::string(paymentView["method"].get_string().value));

                    if (paymentView["transactionCode"])
                        payment->set_transactioncode(std::string(paymentView["transactionCode"].get_string().value));

                    if (paymentView["sender"])
                        payment->set_sendername(std::string(paymentView["sender"].get_string().value));
                }

                result.push_back(r);

            } catch (const std::exception& e) {
                continue;
            }
        }

    } catch (const mongocxx::operation_exception& e) {
        throw std::runtime_error("Operation failed: " + std::string(e.what()));
    } catch (const mongocxx::exception& e) {
        throw std::runtime_error("Database error: " + std::string(e.what()));
    } catch (const std::exception& e) {
        throw std::runtime_error("Error fetching receipts: " + std::string(e.what()));
    }

    return result;
}