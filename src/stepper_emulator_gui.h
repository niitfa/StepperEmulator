#ifndef STEPPER_EMULATOR_H
#define STEPPER_EMULATOR_H

#include "stepper_tcp_server.h"
#include <string>

class StepperEmulatorGUI
{
	std::string	ip = "127.0.0.1";
	int	port = 6101;
	std::unique_ptr<StepperTCPServer> server = std::make_unique<StepperTCPServer>(ip, port);
public:
	StepperEmulatorGUI() = default;
	~StepperEmulatorGUI() = default;
	void Run();
private:
	void Menu();
	void LongAxisMenu();
	void AngAxisMenu();
};

#endif
