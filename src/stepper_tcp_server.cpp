#include "stepper_tcp_server.h"
#include <iostream>

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
			CloseSocket((void*)&this->sock);
			CloseSocket((void*)&this->new_conn);
			CreateSocket((void*)&this->sock, (void*)&this->hint, this->output_port);
			Listen((void*)&this->sock);
			timeval tv{ 0, 300000 };
			AcceptOutput(&this->sock, &this->new_conn, &this->cli, tv);
		}

		this->bytes_sent = Send((void*)&this->new_conn, this->message, kMessageSize);
		memset(this->message, 0, kMessageSize);	

		/* Sleep */
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	memset(this->message, 0, kMessageSize);
	CloseSocket((void*)&this->sock);
	CloseSocket((void*)&this->new_conn);
	this->started.store(false);
}

void StepperTCPServer::InputThreadHandler()
{
	this->initialized.store(true);
	while (!this->stopped.load())
	{
		if(this->bytes_received == -1 || this->bytes_received == 0)
		{
			CloseSocket((void*)&this->sock_in);
			CloseSocket((void*)&this->new_conn_in);
			CreateSocket((void*)&this->sock_in, (void*)&this->hint_in, this->input_port);
			Listen((void*)&this->sock_in);
			timeval tv{ 0, 100000 };
			AcceptInput((void*)&this->sock_in, (void*)&this->new_conn_in, (void*)&this->cli_in, tv);
		}
		

		// write data to this socket
		this->bytes_received = Receive((void*)&this->new_conn_in, this->input_message, kInputMessageSize);


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
	CloseSocket((void*)&this->sock_in);
	CloseSocket((void*)&this->new_conn_in);
	this->started.store(false);
}

int StepperTCPServer::CreateSocket(void* psock, void* addr, int port)
{
#ifdef _WIN32
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) {
		return 1;
	}
	SOCKADDR_IN* phint = (SOCKADDR_IN*)addr;
	phint->sin_addr.s_addr = inet_addr(this->ip.c_str());
	phint->sin_port = htons(port);
	phint->sin_family = AF_INET;

	*(SOCKET*)psock = socket(AF_INET, SOCK_STREAM, NULL);
	bind(*(SOCKET*)psock, (SOCKADDR*)phint, addrLen);
#endif
#ifdef __linux__
	sockaddr_in* phint = (sockaddr_in*) addr;
	*(int*)psock = socket(AF_INET, SOCK_STREAM, 0);
	memset(phint, 0, sizeof(sockaddr_in));
	phint->sin_family = AF_INET;
	phint->sin_addr.s_addr = INADDR_ANY;
	phint->sin_port = htons(port);
	bind(*(int*)psock, (sockaddr*)phint, sizeof(sockaddr_in));
#endif
	return 0;
}

int StepperTCPServer::CloseSocket(void* psock)
{
#ifdef _WIN32
	shutdown(*(SOCKET*)psock, SD_BOTH);
	closesocket(*(SOCKET*)psock);
#endif
#ifdef __linux__
	shutdown(*(int*)psock, SHUT_RDWR);	
	close(*(int*)psock);
#endif
	return 0;
}

int StepperTCPServer::Listen(void* psock)
{
#ifdef _WIN32
	int res = listen(*(SOCKET*)psock, 1);
#endif
#ifdef __linux__
	int res = listen(*(int*)psock, 1);
#endif
	return res;
}

int StepperTCPServer::AcceptOutput(void* psock, void* pnewsock, void* addr, timeval timeout)
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

#ifdef _WIN32
	int size = sizeof(sockaddr_in);
	*(SOCKET*)pnewsock = accept(*(SOCKET*)psock, (sockaddr*)addr, &size);
	if (*(SOCKET*)pnewsock == -1)
	{
	}
	else
	{
		//std::cout << "Accepted!" << std::endl;
		std::cout << "Output connection accepted, port " << this->output_port << std::endl;
	} 
#endif
#ifdef __linux__
	socklen_t size = sizeof(sockaddr_in);
	*(int*)pnewsock = accept(*(int*)psock, (sockaddr*)addr, &size);
	if (*(int*)pnewsock == -1)
	{
	}
	else
	{
		std::cout << "Output connection accepted, port " << this->output_port << std::endl;
	}
#endif


	return *(int*)pnewsock;
}

int StepperTCPServer::AcceptInput(void* psock, void* pnewsock, void* addr, timeval timeout)
{
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

#ifdef _WIN32
	int size = sizeof(sockaddr_in);
	*(SOCKET*)pnewsock = accept(*(SOCKET*)psock, (sockaddr*)addr, &size);
	if (*(SOCKET*)pnewsock == -1)
	{
	}
	else
	{
		std::cout << "Input connection accepted, port " << this->input_port << std::endl;
	}
#endif

#ifdef __linux__
	socklen_t size = sizeof(sockaddr_in);
	*(int*)pnewsock = accept(*(int*)psock, (sockaddr*)addr, &size);
	if (*(int*)pnewsock == -1)
	{
	}
	else
	{
		std::cout << "Input connection accepted, port " << this->input_port << std::endl;
	}
#endif

	return *(int*)pnewsock;
}

int StepperTCPServer::Send(void* psock, char* buff, size_t size) // new conn
{
#ifdef _WIN32
	int res = send(*(SOCKET*)psock, buff, size, 0);
#endif
#ifdef __linux__
	int res = send(*(int*)psock, buff, size, MSG_NOSIGNAL);
#endif
	return res;
}

int StepperTCPServer::Receive(void* psock, char* buff, size_t size)
{
#ifdef _WIN32
	SOCKET* temp_sock = (SOCKET*)psock;
#endif
#ifdef __linux__
	int* temp_sock = (int*)psock
#endif


	int total_bytes_received = 0;
	int res = 0;
	int current_buff_index = 0;

	//char temp_buffer[size];
	char* temp_buffer = new char[size];
	memset(temp_buffer, 0, size);

	while(total_bytes_received != size)
	{
		int res = recv(*temp_sock, temp_buffer, size, MSG_WAITALL);
		if(res == 0)  return res;
		if(res < 0)  return res;

		total_bytes_received += res;

		memcpy(buff + current_buff_index, temp_buffer, res);
		current_buff_index += res;		
	}
	delete[] temp_buffer;
	return total_bytes_received;
}
