#pragma once
#include<string>
#include<vector>

struct Tutor{
    std::string name;
    int tutor_code;
    std::string role;
    std::vector<std::pair<std::string,std::string>> grade_subjetcs;

    void set_grade_subjects(const std::string& grade,const std::string& subject);
    std::vector<std::pair<std::string,std::string>> get_grade_subjects()const;
};