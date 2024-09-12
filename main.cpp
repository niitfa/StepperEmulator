#include <iostream>
#include <thread>
#include <fstream>
#include <string>
#include "stepper_tcp_server.h"

#include <netdb.h>

void read_file(int*, int*, std::string);

int main(int argc, char* argv[])
{
    std::string filename = "message.txt";

    StepperTCPServer* server = new StepperTCPServer("127.0.0.1", 11151);
    int val1 = 0;
    int val2 = 0;

    server->Run();
    while(true)
    {
        read_file(&val1, &val2, filename);
        server->SetLongEncoderValue(val1);
        server->SetAngEncoderValue(val2);

        //std::this_thread::sleep_for(std::chrono::seconds(1));
    }
 
    return 0;
}

void read_file(int* val1, int* val2, std::string filename)
{
    std::ifstream file;
    file.open(filename);
    if(file.is_open())
    {
        std::string line1, line2;
        std::getline(file, line1);
        std::getline(file, line2);
        try
		{
			*val1 = std::stoi(line1);
            *val2 = std::stoi(line2);
		}
		catch (...)
		{
			*val1 = 0;
            *val2 = 0;
		}
    }
    file.close();
}