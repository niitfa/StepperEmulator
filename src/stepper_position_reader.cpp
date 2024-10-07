#include "stepper_position_reader.h"
#include <fstream>

StepperPositionReader::StepperPositionReader(std::string file) : filename{file}
{}

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
        std::string line1, line2;
        std::getline(file, line1);
        std::getline(file, line2);

        try
        {
            curr_data.at(0) = std::stoi(line1);
            curr_data.at(1) = std::stoi(line2);
        }
        catch (...)
        {
            return 0;
        }
    }
    file.close();

    if(curr_data == prev_data)
    {
       return 0;
    } 

    prev_data = curr_data;
    return 1;
}
