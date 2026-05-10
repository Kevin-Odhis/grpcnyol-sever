#include "Student.h"

StudentExam::StudentExam():total_marks(-1),student_name(""),grade(""){}
std::map<std::string,std::pair<std::string,int>> StudentExam::get_subject_scores()const{
    return subject_scores;
}
void StudentExam::set_subject_scores(const std::string& subject,const std::string& level,int score){
    subject_scores.emplace(subject,std::make_pair(level,score));
}
std::string StudentExam::get_level(int score,int count)const{
    if(count>1) score=score/count;
    if(score>=88)return"Exceeding Expectation 2";
    else if(score>=75) return"Exceeding Expectation 1";
    else if(score>=58) return "Meeting Expectation 2";
    else if(score>=41) return "Meetind Expectation 1";
    else if(score>=31){return "Approaching Expectation 2";}
    else if(score>=21){return "Approaching Expectation 1";}
    else if(score>=11){return "Below Expectation 2";}
    else{
        return "Below Expectation 1";
    }
}