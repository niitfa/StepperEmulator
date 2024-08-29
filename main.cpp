#include <iostream>
#include "stepper_emulator_gui.h"

int main()
{
    StepperEmulatorGUI* emulator = new StepperEmulatorGUI();
    emulator->Run();
    return 0;
}