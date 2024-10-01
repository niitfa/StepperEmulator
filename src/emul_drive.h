#ifndef EMUL_DRIVE_H 
#define EMUL_DRIVE_H

#include "stopwatch.h"

class EmulDrive
{
    int move_code = 1;
    int hold_code = 2;
    int reset_code = 3;

    float actual_position = 0;
    float target_position = 0;
    float start_position = 0; 

    float target_velocity = 0;
    float min_velocity = 0;
    float max_velocity = 0;

    Stopwatch clock;
public:
    EmulDrive(
        float min_velocity,
        float max_velocity,
        int move_code,
        int hold_code,
        int reset_code
        );
    ~EmulDrive() = default;

    int GetPosition();
    void ParseMessage(int command_code, int param1, int param2);
private:

    void Update();

    void Reset();
    void Hold();
    void Move(int pos, int vel);

    

};

#endif