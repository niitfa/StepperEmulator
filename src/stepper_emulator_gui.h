#ifndef STEPPER_EMULATOR_H
#define STEPPER_EMULATOR_H

#include "stepper_tcp_server.h"
#include <string>
#include <mutex>
#include <atomic>


class StepperEmulatorGUI
{
	std::string	ip = "127.0.0.1";
	int	port = 6101;
	StepperTCPServer* server = new StepperTCPServer(ip, port, port);

	std::mutex mtx;
	std::atomic<bool> stopflag{false};
public:
	StepperEmulatorGUI() = default;
	~StepperEmulatorGUI() = default;
	void Run(int val_1, int val_2);
private:
	void Menu();
	void LongAxisMenu();
	void AngAxisMenu();

	void Input();
};

#endif
