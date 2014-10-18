#include "Sockets.h"
#include <stdio.h>
#include <cstring> //linux strlen


int main(int argc, const char* argv[]) {

	InitializeSockets();

	if (argc < 2) {
		printf("Host mode\n");
		UDPSocket* socket = new UDPSocket(8383);
		int x;
		while (true) {
			x = socket->Recieve();
			//if(x > 0) break;
		}
		printf("recieved %i bytes", x);
	}
	else {
		printf("Client mode\n");
		UDPSocket* socket = new UDPSocket(8384);
		unsigned int address = (192 << 24) | (168 << 16) | (0 << 8) | (20);
		sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(address);
		addr.sin_port = htons(8383);
		for(int i = 1; i < argc; i++) {
			socket->Send((const char*)argv[i], strlen(argv[i]), addr);
		}
	}

	ShutdownSockets();
}