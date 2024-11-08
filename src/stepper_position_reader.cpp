#include "stepper_position_reader.h"
#include <fstream>
#include <iostream>

StepperPositionReader::StepperPositionReader(std::string file) : filename{file}
{
    for(int i = 0; i < this->vec_size; ++i)
    {
        this->curr_data.push_back(0);
        this->prev_data.push_back(0);
    }
}

int StepperPositionReader::GetValue(int index)
{
    int res = 0;
    try
    {
        res = this->curr_data.at(index);
    }
    catch(...)
    {}
    
    return res;
}

int StepperPositionReader::ReadFile()
{
    std::ifstream file;
    file.open(this->filename);
    if(file.is_open())
    {
        std::string line0, line1, line2, line3;
        std::getline(file, line0);
        std::getline(file, line1);
        std::getline(file, line2);
        std::getline(file, line3);

        //std::cout << curr_data.size() << std::endl;

        try
        {
            curr_data.at(0) = std::stoi(line0);
            curr_data.at(1) = std::stoi(line1);
            curr_data.at(2) = std::stoi(line2);
            curr_data.at(3) = std::stoi(line3);
        }
        catch (...)
        {
           // std::cout << "Catched!\n";
            return 0;
        }
    }
    file.close();

    if(curr_data == prev_data)
    {
       return 0;
    } 

    //std::cout << "Updated!\n";
    prev_data = curr_data;
    return 1;
}
