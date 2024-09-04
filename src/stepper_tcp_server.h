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

class StepperTCPServer
{
	/* Message info */
	static const int kMessageSize = 12;
	static const int kBufferSize = kMessageSize;
	char message[kMessageSize];
	char buffer[kMessageSize];

	static const int kBytePosMessageID = 0;
	static const int kBytePosLongEncValue = 4;
	static const int kBytePosAngEncValue = 8;

	/* Server info */
	std::string ip;
	int port;

	/* Socket */
#ifdef _WIN32
	SOCKET sock, new_conn;
	SOCKADDR_IN addr;
	int addrLen = sizeof(addr);
#endif

#ifdef __linux__
	int sock, new_conn;
	sockaddr_in hint, cli;
	socklen_t len = sizeof(cli);
#endif
	int bytes_sent = -1;

	/* Thread info */
	std::mutex mtx;
	std::atomic<bool> started{ false };
	std::atomic<bool> initialized{ false };
	std::atomic<bool> stopped{ false };

	uint32_t message_no = 0;
public:
	StepperTCPServer(std::string ip, int port);
	~StepperTCPServer() = default;
	int Run();
	int Stop();

	void SetLongEncoderValue(int);
	void SetAngEncoderValue(int);
private:
	void ThreadHandler();
	int CreateSocket();
	int CloseSocket();
	int Listen();
	int Accept();
	int Send();
};

#endif

