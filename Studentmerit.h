#pragma once
#include<string>
#include<vector>
struct Subject{
    std::string subject;
    int marks{-1};
    std::string level{""};
    //level method
    std::string get_level()const;
};
struct Studentmerit{
    std::string name;
    std::string upi;
    int t_marks{0};
    std::string sex{"Unspecified"};
    int rank{-1};
    std::vector<Subject>subjects;
    
    //computation methods
    std::string average_level()const;
    int total_marks()const;
    int total_points()const;
};