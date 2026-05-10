#include"Tutor.h"

void Tutor::set_grade_subjects(const std::string& grade,const std::string& subject){
    std::pair<std::string,std::string> grd_sub{grade,subject};
    grade_subjetcs.emplace_back(std::move(grd_sub));
}
std::vector<std::pair<std::string,std::string>> Tutor::get_grade_subjects()const{
    return grade_subjetcs;
}