#ifndef STEPPER_TCP_SERVER_H
#define STEPPER_TCP_SERVER_H

#ifdef _WIN32
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "Ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif

#ifdef __linux__
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#endif

#include <string>
#include <atomic>
#include <mutex>
#include <thread>

#include "emul_drive.h"

class StepperTCPServer
{
	/* Output message info */
	static const int kMessageSize = 12;
	static const int kBufferSize = kMessageSize;
	char message[kMessageSize];
	char buffer[kMessageSize];

	static const int kBytePosMessageID = 0;
	static const int kBytePosLongEncValue = 4;
	static const int kBytePosAngEncValue = 8;

	/* Input message info */
	static const int kInputMessageSize = 12;
	static const int kInputBufferSize = kInputMessageSize;
	char input_message[kMessageSize];
	//char input_message_temp[kMessageSize];
	char input_buffer[kMessageSize];

	int conn_alive = 0;

	static const int kInBytePosCommand = 0;
	static const int kInBytePosParam1 = 4;
	static const int kInBytePosParam2 = 8;

	/* Server info */
	std::string ip;
	int output_port;
	int input_port;

	/* Drives */
	EmulDrive drive_long{1000, 5000, 0x01, 0x02, 0x03};
	EmulDrive drive_ang {1000, 5000, 0x11, 0x12, 0x13};
	std::mutex int_mtx;
#ifdef _WIN32
	SOCKET sock, new_conn;
	SOCKADDR_IN addr;
	int addrLen = sizeof(addr);
#endif

#ifdef __linux__
	int sock, new_conn;
	sockaddr_in hint, cli;
	socklen_t len = sizeof(hint);
#endif
	int bytes_sent = -1;

	/* Input socket */
	#ifdef __linux__
	int sock_in, new_conn_in;
	sockaddr_in hint_in, cli_in;
#endif
	int bytes_received = -1;

	/* Thread info */
	std::mutex mtx;
	std::atomic<bool> started{ false };
	std::atomic<bool> initialized{ false };
	std::atomic<bool> stopped{ false };

	uint32_t message_no = 0;
public:
	StepperTCPServer(std::string ip, int output_port, int input_port);
	~StepperTCPServer() = default;
	int Run();
	int Stop();

	void SetLongEncoderValue(int);
	void SetAngEncoderValue(int);
private:
	void OutputThreadHandler();
	void InputThreadHandler();


	int CreateSocket(int* psock, void* addr, int port);
	int CloseSocket(int* psock);
	int Listen(int* psock);

	int AcceptOutput(int* psock, int* pnewsock, void* addr, timeval timeout);
	int AcceptInput(int* psock, int* pnewsock, void* addr, timeval timeout);

	int Send(int* psock, char* buff, size_t size);
	int Receive(int* psock, char* buff, size_t size);
};

#endif

