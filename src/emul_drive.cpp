#include "emul_drive.h"

template <typename T> int sgn(T val) {
    return (T(0) < val) - (val < T(0));
}

EmulDrive::EmulDrive(float min_velocity, float max_velocity, int move_code,
        int hold_code, int reset_code) :
        min_velocity{min_velocity},
        max_velocity{max_velocity},
        move_code{move_code},
        hold_code{hold_code},
        reset_code{reset_code}
{}

void EmulDrive::ParseMessage(int command_code, int param1, int param2)
{
    if(command_code == move_code)
    {
        this->Move(param1, abs(param2));
    }
    else if (command_code == hold_code)
    {
        this->Hold();
    }
    else if (command_code == reset_code)
    {
        this->Reset();
    }
}

void EmulDrive::Update()
{
    clock.Update();
    int delta_pos = target_position - start_position;

    actual_position += sgn(delta_pos) * target_velocity * clock.Microseconds() / 1e+6;

    if(delta_pos > 0)
    {
        actual_position = std::min(actual_position, target_position);  
    }
    else if (delta_pos < 0)
    {
        actual_position = std::max(actual_position, target_position);
    }

    clock.Reset();
}

void EmulDrive::Reset()
{
    Update();
    this->target_position = target_position - actual_position;
    this->actual_position = 0;
}

void EmulDrive::Hold()
{
    Move(GetPosition(), 0);
}

void EmulDrive::Move(int pos, int vel)
{
    Update();
    this->start_position = actual_position;
    this->target_position = (float)pos;

    this->target_velocity = (float)abs(vel);
    this->target_velocity = std::min(this->max_velocity, target_velocity);
    this->target_velocity = std::max(this->min_velocity, target_velocity);
}

int EmulDrive::GetPosition()
{
    Update();
    return (int)actual_position;
}

void EmulDrive::SetPosition(int pos)
{
    this->actual_position = pos;
    Hold();
}