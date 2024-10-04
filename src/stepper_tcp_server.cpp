#include "stepper_tcp_server.h"
#include <iostream>
#include <poll.h>

/* TODO:
	1) CreateSocket(..) for Windows
	2) Accept(...) for Windows
*/

StepperTCPServer::StepperTCPServer(std::string ip, int output_port, int input_port) : 
	ip{ip},
	output_port{output_port},
	input_port{input_port}
{
	memset(this->message, 0, kMessageSize);
	memset(this->buffer, 0, kMessageSize);
}

int StepperTCPServer::Run()
{
	if (!this->started.load())
	{
		this->stopped.store(false);
		this->initialized.store(false);
		this->started.store(true);

		std::thread thrdOut(&StepperTCPServer::OutputThreadHandler, this);
		thrdOut.detach();

		std::thread thrdIn(&StepperTCPServer::InputThreadHandler, this);
		thrdIn.detach();

		while (!this->initialized.load());
	}
	return 0;
}

int StepperTCPServer::Stop()
{
	this->stopped.store(true);
	while (this->started.load());
	return 0;
}

void StepperTCPServer::SetLongEncoderValue(int val)
{
	this->mtx.lock();
	memcpy(this->buffer + this->kBytePosLongEncValue, &val, sizeof(val));
	this->mtx.unlock();
}

void StepperTCPServer::SetAngEncoderValue(int val)
{
	this->mtx.lock();
	memcpy(this->buffer + this->kBytePosAngEncValue, &val, sizeof(val));
	this->mtx.unlock();

}

/* Server thread section */
void StepperTCPServer::OutputThreadHandler()
{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));


		this->initialized.store(true);
		while (!this->stopped.load())
		{

			
		message_no++;
		//this->mtx.lock();
		//memcpy(this->message, this->buffer, kMessageSize);
		//this->mtx.unlock();	

		// Fill message frome emul_drive objects
		int pos_long 	= drive_long.GetPosition();
		//std::cout << "StepperTCPServer::OutputThreadHandler(): pos_long " << pos_long << std::endl;
		int pos_ang 	= drive_ang.GetPosition();

		memcpy(this->message + this->kBytePosMessageID, &this->message_no, sizeof(this->message_no));
		memcpy(this->message + this->kBytePosLongEncValue, &pos_long, sizeof(pos_long));
		memcpy(this->message + this->kBytePosAngEncValue, &pos_ang, sizeof(pos_ang));


		if(this->bytes_sent == -1)
		{	
			CloseSocket(&this->sock);
			CloseSocket(&this->new_conn);
			CreateSocket(&this->sock, &this->hint, this->output_port);
			Listen(&this->sock);
			AcceptOutput(&this->sock, &this->new_conn, &this->cli, (timeval){0, 300000});
		}
		this->bytes_sent = Send(&this->new_conn, this->message, kMessageSize);
		memset(this->message, 0, kMessageSize);	

		/* Sleep */
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	memset(this->message, 0, kMessageSize);
	CloseSocket(&this->sock);
	CloseSocket(&this->new_conn);
	this->started.store(false);
}

void StepperTCPServer::InputThreadHandler()
{
	this->initialized.store(true);
	while (!this->stopped.load())
	{
		if(this->bytes_received == -1 || this->bytes_received == 0)
		{
			CloseSocket(&this->sock_in);
			CloseSocket(&this->new_conn_in);
			CreateSocket(&this->sock_in, &this->hint_in, this->input_port);
			Listen(&this->sock_in);
			AcceptInput(&this->sock_in, &this->new_conn_in, &this->cli_in, (timeval){0, 100000});
		}

		// write data to this socket
		this->bytes_received = Receive(&this->new_conn_in, this->input_message, kInputMessageSize);
		// debug output
		/* std::cout << "bytes received: " << this->bytes_received << "\t";
		std::cout << *(int*)(this->input_message + kInBytePosCommand) << " " 
			<< *(int*)(this->input_message + kInBytePosParam1) << " "
			<< *(int*)(this->input_message + kInBytePosParam2) << "\n"; */

		if(bytes_received == kInputMessageSize)
		{
			int val_1 = *(int*)(this->input_message + kInBytePosCommand);
			int val_2 = *(int*)(this->input_message + kInBytePosParam1);
			int val_3 = *(int*)(this->input_message + kInBytePosParam2);
			//std::cout << "Received: " << val_1 << " " << val_2 << " " << val_3 << std::endl;

			std::cout << "Received 12 bytes: 0-3 bytes: " << val_1 << "; 3-7 bytes: " << val_2 << ";  8-11 bytes = " << val_3 << "\n\t";
			printf("hex: 0x%08x%08x%08x\n", val_3, val_2, val_1);

			// Handling message
			int_mtx.lock();
			drive_long.ParseMessage(val_1, val_2, val_3);
			drive_ang.ParseMessage(val_1, val_2, val_3);
			int_mtx.unlock();

		}

		// reset message
		memset(this->input_message, 0, kInputMessageSize);
					
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
		
	}
	memset(this->input_message, 0, kInputMessageSize);
	CloseSocket(&this->sock_in);
	CloseSocket(&this->new_conn_in);
	this->started.store(false);
}

int StepperTCPServer::CreateSocket(int* psock, void* addr, int port)
{
#ifdef _WIN32
	//  WILL NOT WORK ON WINDOWS !!!
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) {
		return 1;
	}

	this->addr.sin_addr.s_addr = inet_addr(this->ip.c_str());
	this->addr.sin_port = htons(port);
	this->addr.sin_family = AF_INET;

	*psock = socket(AF_INET, SOCK_STREAM, NULL);
	bind(*psock, (SOCKADDR*)&this->addr, addrLen);
#endif
#ifdef __linux__
	sockaddr_in* phint = (sockaddr_in*) addr;
	*psock = socket(AF_INET, SOCK_STREAM, 0);
	memset(phint, 0, sizeof(sockaddr_in));
	phint->sin_family = AF_INET;
	phint->sin_addr.s_addr = INADDR_ANY;
	phint->sin_port = htons(port);
	bind(*psock, (sockaddr*)phint, sizeof(sockaddr_in));
#endif
	return 0;
}

int StepperTCPServer::CloseSocket(int* psock)
{
#ifdef _WIN32
	closesocket(*psock);
#endif
#ifdef __linux__
	shutdown(*psock, SHUT_RDWR);	
	close(*psock);
#endif
	return 0;
}

int StepperTCPServer::Listen(int* psock)
{
	int res = listen(*psock, 1);
	return res;
}

int StepperTCPServer::AcceptOutput(int* psock, int* pnewsock, void* addr, timeval timeout)
{
/* #ifdef _WIN32
	*pnewsock = accept(*psock, (SOCKADDR*)&this->addr, &this->addrLen);
#endif
#ifdef __linux__	
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(*psock, &readfds);

	int sel = select(*psock + 1 , &readfds , NULL , NULL , &timeout);
	//int sel = select(*psock + 1, NULL, &readfds, NULL, &timeout);
	if(sel == 0)
	{
		std::cout << "Select timeout " << std::endl;
		return -2;
	}
	if(sel == -1)
	{
		std::cout << "Select error " << std::endl;
		return -3;
	}

	socklen_t size = sizeof(sockaddr_in);
	std::cout << "Output block...\n";
	*pnewsock = accept(*psock, (sockaddr*)addr, &size);
	if(*pnewsock == -1)
	{
		std::cout << "Accept fail " <<  strerror(errno) << std::endl;
	}
	else
	{
		std::cout << "Output connection accepted, port " << this->output_port << std::endl;
	}
#endif */


	socklen_t size = sizeof(sockaddr_in);
	*pnewsock = accept(*psock, (sockaddr*)addr, &size);
	if(*pnewsock == -1)
	{
		//std::cout << "Accept fail " <<  strerror(errno) << std::endl;
	}
	else
	{
		//std::cout << "Accepted!" << std::endl;
		std::cout << "Output connection accepted, port " << this->output_port << std::endl;
	} 

	return *pnewsock;
}

int StepperTCPServer::AcceptInput(int* psock, int* pnewsock, void* addr, timeval timeout)
{
	socklen_t size = sizeof(sockaddr_in);
	*pnewsock = accept(*psock, (sockaddr*)addr, &size);
	if(*pnewsock == -1)
	{
		//std::cout << "Accept fail " <<  strerror(errno) << std::endl;
	}
	else
	{
		//std::cout << "Accepted!" << std::endl;
		std::cout << "Input connection accepted, port " << this->input_port << std::endl;
	} 

/* #ifdef __linux__	
	fd_set readfds;
	FD_ZERO(&readfds);
	FD_SET(*psock, &readfds);

	int sel = select(*psock + 1 , &readfds , NULL , NULL , &timeout);
	//int sel = select(*psock + 1, NULL, &readfds, NULL, &timeout);
	if(sel == 0)
	{
		//std::cout << "Select timeout " << std::endl;
		return -2;
	}
	if(sel == -1)
	{
		//std::cout << "Select error " << std::endl;
		return -3;
	}

	socklen_t size = sizeof(sockaddr_in);
	*pnewsock = accept(*psock, (sockaddr*)addr, &size);
	if(*pnewsock == -1)
	{
		//std::cout << "Accept fail " <<  strerror(errno) << std::endl;
	}
	else
	{
		std::cout << "Input connection accepted, port " << this->input_port << std::endl;
	}
#endif */

	return *pnewsock;
}

int StepperTCPServer::Send(int* psock, char* buff, size_t size) // new conn
{
	int res;
#ifdef _WIN32
	res = send(*psock, buff, size, 0);
#endif
#ifdef __linux__
	res = send(*psock, buff, size, MSG_NOSIGNAL);
#endif
	return res;
}

int StepperTCPServer::Receive(int* psock, char* buff, size_t size)
{

	int total_bytes_received = 0;
	int res = 0;
	int current_buff_index = 0;

	char temp_buffer[size];
	memset(temp_buffer, 0, size);

	while(total_bytes_received != size)
	{
		int res = recv(*psock, temp_buffer, size, MSG_WAITALL);
		if(res == 0)  return res;
		if(res < 0)  return res;

		total_bytes_received += res;

		memcpy(buff + current_buff_index, temp_buffer, res);
		current_buff_index += res;		
	}
	return total_bytes_received;
}
