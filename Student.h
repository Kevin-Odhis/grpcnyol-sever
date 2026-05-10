#pragma once
#include<string>
#include<vector>
#include<map>
#include<variant>

struct StudentExam{
    std::string exam_name;
    std::string student_name;
    std::string grade;
    std::string upi;
    int total_marks;
    int pp_score=-1;
    std::string level;
    std::map<std::string,std::pair<std::string,int>>subject_scores;

    StudentExam();
    std::map<std::string,std::pair<std::string,int>> get_subject_scores()const;
    void set_subject_scores(const std::string& subject,const std::string& level,int score);
    std::string get_level(int score=1,int count=1)const;
};