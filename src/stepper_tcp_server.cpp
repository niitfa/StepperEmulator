#include "stepper_tcp_server.h"
#include <iostream>

StepperTCPServer::StepperTCPServer(std::string ip, int port) : ip{ip}, port{port}
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
		std::thread thrd(&StepperTCPServer::ThreadHandler, this);
		thrd.detach();
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
void StepperTCPServer::ThreadHandler()
{
	this->initialized.store(true);
	while (!this->stopped.load())
	{
		this->mtx.lock();
		memcpy(this->message, this->buffer, kMessageSize);
		this->mtx.unlock();

		
		memcpy(this->buffer + this->kBytePosMessageID, &this->message_no, sizeof(this->message_no));
		if(this->bytes_sent == -1)
		{
			CloseSocket();
			CreateSocket();
			Listen();
			Accept();
			Send();
		}
		else
		{
			Send();
		}
		memset(this->message, 0, kMessageSize);
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
	memset(this->message, 0, kMessageSize);
	CloseSocket();
	this->started.store(false);
}

int StepperTCPServer::CreateSocket()
{
	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) {
		return 1;
	}

	this->addr.sin_addr.s_addr = inet_addr(this->ip.c_str());
	this->addr.sin_port = htons(this->port);
	this->addr.sin_family = AF_INET;

	this->sock = socket(AF_INET, SOCK_STREAM, NULL);
	bind(this->sock, (SOCKADDR*)&this->addr, addrLen);

	return 0;
}

int StepperTCPServer::CloseSocket()
{
	closesocket(this->sock);
	return 0;
}

int StepperTCPServer::Listen()
{
	listen(this->sock, 1);
	return 0;
}

int StepperTCPServer::Accept()
{
	this->new_conn = accept(this->sock, (SOCKADDR*)&this->addr, &this->addrLen);
	return 0;
}

int StepperTCPServer::Send()
{
	this->bytes_sent = send(new_conn, this->message, kMessageSize, NULL);
	if (bytes_sent != -1)
	{
		this->message_no++;
	}
	return 0;
}