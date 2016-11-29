#include "TCPHeader.h"

#define SERVICE_PORT 4000

int main(int argc, char* args[])
{
	std::string fileName;
	int portNum;

	if (argc != 3) {
		fprintf(stderr, "Arguments format error: please input as \"./server PORT-NUMBER FILE-NAME\".");
		return -1;
	}
	else {
		portNum = atoi(args[1]);
		fileName = args[2];
	}

	int socketfd;
	struct sockaddr_in myaddr, remaddr;

	if ((socketfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Cannot create socekt.");
		return -1;
	}

	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = inet_addr("10.0.0.1");;
	myaddr.sin_port = htons(portNum);

	memset((char *) &remaddr, 0, sizeof(remaddr));
	remaddr.sin_family = AF_INET;
	remaddr.sin_port = htons(SERVICE_PORT);

	struct timeval startTime, tmp;
	gettimeofday(&startTime , NULL);

	if (bind(socketfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("Bind failed");
		return -2;
	}

	while (true) {
		
	}

	return 0;
}
