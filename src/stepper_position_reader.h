#ifndef STEPPER_POSITION_READER_H
#define STEPPER_POSITION_READER_H

#include <string>
#include <vector>

class StepperPositionReader
{
    std::string filename;
    int vec_size = 2;
    std::vector<int> curr_data {vec_size, 0};
    std::vector<int> prev_data {vec_size, 0};
public:
    StepperPositionReader(std::string);
    int ReadFile();
    int GetValue(int index);
};

#endif