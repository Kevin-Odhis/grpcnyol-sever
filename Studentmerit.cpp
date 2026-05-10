#include"Studentmerit.h"
#include<iomanip>
#include<cmath>

int Studentmerit::total_marks()const{
    int total=0;
    for(const auto& subject:subjects){
        if(subject.marks<0)continue;
        total+=subject.marks;
    }
    return total;
}

int Studentmerit::total_points()const{
    int points=0;
    for(const auto& subject:subjects){
        if(subject.marks<0)continue;
        if(subject.level=="EE1")points+=8;
        else if(subject.level=="EE2")points+=7;
        else if(subject.level=="ME1")points+=6;
        else if(subject.level=="ME2")points+=5;
        else if(subject.level=="AE1")points+=4;
        else if(subject.level=="AE2")points+=3;
        else if(subject.level=="BE1")points+=2;
        else if(subject.level=="BE2")points+=1;
        else{points+=0;}
    }
    return points;
}

std::string Subject::get_level()const{
    if(marks>=90)return "EE1";
    else if(marks>=75) return "EE2";
    else if(marks>=58)return "ME1";
    else if(marks>=41)return "ME2";
    else if(marks>=31)return "AE1";
    else if(marks>=21)return "AE2";
    else if(marks>=11)return "BE1";
    else if(marks>=0)return "BE2";
    else return "";
}
std::string Studentmerit::average_level()const{
    double marks=static_cast<double>(total_points());
    int subject_count=static_cast<double>(subjects.size());
    if(subject_count==0) return "";
    double average=marks/subject_count;
    int pts=static_cast<int>(std::round(average));
    if(pts>=8)return "EE1";
    else if(pts>=7) return "EE2";
    else if(pts>=6)return "ME1";
    else if(pts>=5)return "ME2";
    else if(pts>=4)return "AE1";
    else if(pts>=3)return "AE2";
    else if(pts>=2)return "BE1";
    else if(pts>=1)return "BE2";
    else return "";
}