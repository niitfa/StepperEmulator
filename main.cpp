#include <iostream>
#include "stepper_tcp_server.h"

int main(int argc, char* argv[])
{
    if(argc == 3)
    {
        StepperTCPServer* server = new StepperTCPServer("127.0.0.1", 6101);
        int val1 = atoi(argv[0]);
        int val2 = atoi(argv[1]);
        sscanf(argv[1], "%d", &val1);
        sscanf(argv[2], "%d", &val2);
        server->Run();
        server->SetLongEncoderValue(val1);
        server->SetAngEncoderValue(val2);
        while(true);
    }
    else
    {
        std::cout << "Enter 2 integer arguments.\n";
    }
    return 0;
}