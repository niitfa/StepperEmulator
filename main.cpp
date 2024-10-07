#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include "stepper_tcp_server.h"

int main(int argc, char* argv[])
{
    StepperTCPServer* server = new StepperTCPServer("127.0.0.1", 11151, 11152);
    server->Run();
    while(true)
        ;
        
    return 0;
}