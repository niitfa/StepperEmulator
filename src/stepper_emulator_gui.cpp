#include "stepper_emulator_gui.h"
#include <iostream>
#include <string>

void StepperEmulatorGUI::Run()
{
	std::cout << "Creating server: " 
		<< this->ip << " port " << this->port << "." << std::endl;
	this->server->Run();
	this->Menu();
}

void StepperEmulatorGUI::Menu()
{
	while (true)
	{
		std::string option_str{""};
		int option;
		std::cout << "\n" << "Select option:\n"
			"1. Change longitudal axis position.\n" <<
			"2. Change angular axis position.\n" <<
			"3. Reset all  positions.\n\n" <<
			"> ";
		std::cin >> option_str;
		try
		{
			option = std::stoi(option_str);
		}
		catch (...)
		{
			std::cout << "Bad value.\n";
			continue;
		}
		switch (option)
		{
		case 1:
			LongAxisMenu();
			continue;
		case 2:
			AngAxisMenu();
			continue;
		case 3:
			std::cout << "\nPositions was reset.\n";
			server->SetLongEncoderValue(0);
			server->SetAngEncoderValue(0);
			continue;
		default:
			std::cout << "Bad value.\n\n";
			continue;
		}
	}
}

void StepperEmulatorGUI::LongAxisMenu()
{
	int position;
	std::string position_str{ "" };
	std::cout << "\n" << "Set longitudal position as int value (q to back):\n\n";
	while (true)
	{
		std::cout << "> ";
		std::cin >> position_str;
		if (position_str == "q")
		{
			return;
		}
		else
		{
			try 
			{
				position = std::stoi(position_str);
			}
			catch(...)
			{
				std::cout << "Bad value.\n";
				continue;
			}
			server->SetLongEncoderValue(std::stoi(position_str));
		}
	}
}

void StepperEmulatorGUI::AngAxisMenu()
{
	int position;
	std::string position_str{ "" };
	std::cout << "\n" << "Set angular position as int value (q to back):\n\n";
	while (true)
	{
		std::cout << "> ";
		std::cin >> position_str;
		if (position_str == "q")
		{
			return;
		}
		else
		{
			try
			{
				position = std::stoi(position_str);
			}
			catch (...)
			{
				std::cout << "Wrong value.\n";
				continue;
			}
			server->SetAngEncoderValue(std::stoi(position_str));
		}
	}
}
