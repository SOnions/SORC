//////////////////////////////////////////////////////////////////////
//Sockets.h
//by Sam Onions
//////////////////////////////////////////////////////////////////////
//Provides a bisic interface for cross-platform socket communication
//////////////////////////////////////////////////////////////////////

//Platform Detection
#define PLATFORM_WINDOWS 1
#define PLATFORM_LINUX 2

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#else
#define PLATFORM PLATFORM_LINUX
#endif


//Included Dependancies
#if PLATFORM == PLATFORM_WINDOWS
	#include <WinSock2.h>
	#pragma comment(lib, "Ws2_32.lib")
#elif PLATFORM == PLATFORM_LINUX
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <fcntl.h>
#endif


#define DEFAULT_BUFFER_SIZE 256


bool InitializeSockets();
void ShutdownSockets();



//////////////////////////////////////////////////////////////////////
//STRUCT: PacketHeader
//////////////////////////////////////////////////////////////////////
//A small structrue which should be sent at the head of every UDP
//packet and manages a virtual connection.
//////////////////////////////////////////////////////////////////////
struct PacketHeader
{
	unsigned int protocolID;
	unsigned int sequence;
	unsigned int ack;
	unsigned int ackField;
};


//////////////////////////////////////////////////////////////////////
//STRUCT: UDPConnection
//////////////////////////////////////////////////////////////////////
//Holds information used by a UDPSocket to provide some basic 
//reliability improvements to connection
//////////////////////////////////////////////////////////////////////
struct UDPConnection
{
	sockaddr_in remoteAddress;
	unsigned int remoteSequence;
	unsigned int ackField;
	unsigned int sequence;
};


//////////////////////////////////////////////////////////////////////
//CLASS: UDPSocket
//////////////////////////////////////////////////////////////////////
//basic wrapper of a UDP communication socket
//////////////////////////////////////////////////////////////////////
class UDPSocket {
	//Membder variables
private:
	int m_Handle;
	bool m_bInitialized;
	UDPConnection m_Connection;  //TODO: Remove/expand for multiple connections
	char* m_Buffer;
	int m_BufferSize;

	//Constructors
protected:
	UDPSocket();
public:
	UDPSocket(unsigned short port);

	//Communication methods
	int Send(const char* buffer, int bufferSize, sockaddr_in address);
	int Recieve();
	int Recieve(char* buffer, int bufferSize);
};