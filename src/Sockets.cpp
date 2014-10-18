#include "Sockets.h"
#include <stdio.h>
#include <cstring> //for memcpy in Linux

#define PROTOCOL_ID 1337

using namespace std;


#if PLATFORM == PLATFORM_WINDOWS
typedef int socklen_t;
#endif

bool InitializeSockets() 
{
#if PLATFORM == PLATFORM_WINDOWS
	WSADATA wsaData;
	return WSAStartup(MAKEWORD(2, 2), &wsaData) == NO_ERROR;
#else
	return true;
#endif
}//InitializeSockets()


void ShutdownSockets() 
{
#if PLATFORM == PLATFORM_WINDOWS
	WSACleanup();
#endif
}//ShutdownSockets()




//////////////////////////////////////////////////////////////////////
//CLASS: UDPSocket
//////////////////////////////////////////////////////////////////////
UDPSocket::UDPSocket(unsigned short port) 
{
	m_bInitialized = false;

	m_Handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_Handle < 0) {
		printf("Failed to create Socket");
		return;
	}

	sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	if (bind(m_Handle, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0) {
		printf("Unable to bind socket to port %i", port);
		return;
	}

#if PLATFORM == PLATFORM_WINDOWS
	DWORD nonBlocking = 1;
	if (ioctlsocket(m_Handle, FIONBIO, &nonBlocking) != 0) {
		printf("Falied to set socket %i into non-blocking mode", m_Handle);
		return;
	}
#elif PLATFORM == PLATFORM_LINUX
	int nonBlocking = 1;
	if (fcntl(m_Handle, F_SETFL, O_NONBLOCK, nonBlocking) == -1) {
		printf("Falied to set socket %i into non-blocking mode", m_Handle);
		return;
	}
#endif

	m_Connection.ackField = 0;
	m_Connection.remoteSequence = 0;
	m_Connection.sequence = 1;
	
	//Create our buffer
	m_BufferSize = DEFAULT_BUFFER_SIZE;
	m_Buffer = new char[m_BufferSize];

	m_bInitialized = true;
}


int UDPSocket::Send(const char* buffer, int bufferSize, sockaddr_in address) 
{
	PacketHeader header;
	header.sequence = m_Connection.sequence;
	header.protocolID = PROTOCOL_ID;
	int packetSize = bufferSize + sizeof(PacketHeader);

	char* dataBuffer = new char[DEFAULT_BUFFER_SIZE];

	memcpy((void*)dataBuffer, (void*)&header, sizeof(PacketHeader));
	memcpy((void*)(dataBuffer + sizeof(PacketHeader)), buffer, bufferSize);

	m_Connection.sequence++;

	int sentBytes = sendto(m_Handle, (const char*)dataBuffer, packetSize, 0, (sockaddr*)&address, sizeof(sockaddr_in));
	printf("Sending %i (%i) bytes: %s\n", bufferSize, packetSize, buffer);

	if (sentBytes != packetSize) {
		printf("Only sent %i of %i bytes", sentBytes, packetSize);
	}
	return sentBytes;
}


int UDPSocket::Recieve() 
{
	if (!m_bInitialized) {
		printf("Recieve called on an un-initialized Socket");
	}

	sockaddr_in from;
	socklen_t fromLength = sizeof(sockaddr_in);

	int recievedBytes = recvfrom(m_Handle, (char*)m_Buffer, m_BufferSize, 0, (sockaddr*)&from, &fromLength);

	if (recievedBytes > 0) {
		printf("Recieved %i bytes: ", recievedBytes);
		for (int i = sizeof(PacketHeader); i < recievedBytes; i++) {
			printf("%c", m_Buffer[i]);
		}
		printf("\n");
		
		//Update details of the connection
		PacketHeader* header = (PacketHeader*)m_Buffer;
		if(header->sequence > m_Connection.remoteSequence) {
				m_Connection.remoteSequence = header->sequence;
				printf("Packed Sequence %i\n", m_Connection.remoteSequence);
		}
	}

	return recievedBytes;
}


int UDPSocket::Recieve(char* buffer, int bufferSize)
{		
	if (!m_bInitialized) {
		printf("Recieve called on an un-initialized Socket");
		return -1;
	}

	sockaddr_in from;
	socklen_t fromLength = sizeof(sockaddr_in);
	PacketHeader* header;
	int recievedBytes;
	
	recievedBytes = recvfrom(m_Handle, (char*)m_Buffer, m_BufferSize, 0, (sockaddr*)&from, &fromLength);		
	
	if(recievedBytes < sizeof(PacketHeader)) {
		printf("Recieved Invalid Buffer size: %i", recievedBytes);
		return -1;
	}
	
	header = (PacketHeader*)m_Buffer;
	
	if(header->protocolID != PROTOCOL_ID) {
		printf("Invalid Portocol ID: %i", header->protocolID);
		return -1;
	}
	
	//Now discount the header
	recievedBytes -= sizeof(PacketHeader);

	if (recievedBytes > 0) {
		memcpy((void*)buffer, (void*)m_Buffer + sizeof(PacketHeader), recievedBytes);
		printf("Recieved %i bytes: ", recievedBytes);
		for (int i = sizeof(PacketHeader); i < recievedBytes; i++) {
			printf("%c", buffer[i]);
		}
		printf("\n");
	}
	
	return recievedBytes;
}
	