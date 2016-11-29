#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>
#include <netdb.h>
#include "HttpAbstract.h"

#define BUFSIZE 2048

char* hostname;
char* portNum;
char* fileRoot;

std::unordered_map<std::string, std::string> fileType;
std::string getFileType(std::string fileName);

//Here we just list the basic and usual content type
void initialFileType() {
    fileType["zip"]="application/zip";
    fileType["htm"]="text/html";
    fileType["html"]="text/html";
    fileType["txt"]="text/plain";
    fileType["xml"]="text/xml";
    fileType["css"]="text/css";
    fileType["wav"]="audio/wav";
    fileType["aiff"]="audio/aiff";
    fileType["mp4"]="video/mp4";
    fileType["mpg"]="video/mpg";
    fileType["png"]="image/png";
    fileType["gif"]="image/gif";
    fileType["jpg"]="image/jpg";
    fileType["jpeg"]="image/jpeg";
    fileType["wmv"]="video/wmv";
    fileType["mp3"]="audio/mp3";
}

void childConnection(int clientSockfd) {
	char buf[BUFSIZE];
	HttpRequest request;
	HttpResponse response;
	std::vector<uint8_t> requestStream;
	std::vector<uint8_t> data;
	size_t requestSize;

	//Typically, header size of 700 - 800 bytes is common. So the request size will not be very large.
	cout<<"Receive http request\n";
	memset(buf, '\0', sizeof(buf));
	if ((requestSize = recv(clientSockfd, &buf, sizeof(buf), 0)) == -1) {
		fprintf(stderr, "recv error\n");
		return;
	}
	cout<<"Receive done\n";
	cout<<endl;
	cout<<"Socket file descriptor: "<<clientSockfd<<endl;

	cout<<"Http response object construction\n";
	for (int i=0;i<requestSize;i++) requestStream.push_back(buf[i]);
	//requestStream.assign(buf, buf + requestSize);cout<<requestStream.size()<<endl;
	if (!request.consume(requestStream) || request.getHeaderByKey("Host").size() == 0) {
		response.setHttpVersion("1.0");
		response.setStatus("400");
		response.setStatusInfo("Bad request");
		response.setData(data);
		response.setHeader("Content-Length", to_string(data.size()));
		response.setHeader("Content-Type", "text/html");
	}
	else {
		//Get filePath and fileName
		std::string URL = request.getURL();
		std::string filePath;
		std::string fileName;
		int posSeparator;

		for (posSeparator = URL.size()-1; posSeparator >= 0; posSeparator--) {
			if (URL[posSeparator] == '/') {
				filePath = URL.substr(0, posSeparator);
				if (filePath.size() == 0) {
					filePath = "/";
				}
				else {
					filePath = filePath + "/";
				}
				fileName = URL.substr(posSeparator+1, URL.size()-1-posSeparator);
				break;
			}
		}

		if (URL == "/") {
			filePath = "/";
			fileName = "index.html";
		}

		cout<<"File full path: "<<fileRoot + filePath + fileName<<"\n";
		std::ifstream file(fileRoot + filePath + fileName, ios::binary);
		if (!file.good()) {
			response.setHttpVersion(request.getHttpVersion());
			response.setStatus("404");
			response.setStatusInfo("Not found");
			response.setData(data);
			response.setHeader("Content-Length", to_string(data.size()));
			response.setHeader("Content-Type", "text/html");
		}
		else {
			data = vector<uint8_t>((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
			response.setHttpVersion(request.getHttpVersion());
			response.setStatus("200");
			response.setStatusInfo("OK");
			//response.setData(data);cout<<"jajaja"<<endl;
			response.setHeader("Content-Length", to_string(data.size()));
			response.setHeader("Content-Type", getFileType(fileName));
		}
	}
	cout<<"Http Response object construction done\n";

	std::vector<uint8_t> responseCode = response.encode(data);
	int sentSize = 0;
	int codeSize = responseCode.size();
	cout<<"Start sending http response\n";
	while (sentSize < codeSize) {
		int startPoint = sentSize;
		int endPoint;
		if (codeSize - sentSize < BUFSIZE) {
			endPoint = responseCode.size();
			sentSize = codeSize;
		}
		else {
			endPoint = sentSize + BUFSIZE;
			sentSize += BUFSIZE;
		}

		std::string segmentS(responseCode.begin()+startPoint, responseCode.begin()+endPoint);
		const char* toSend = segmentS.c_str();
		if (send(clientSockfd, toSend, endPoint - startPoint, 0) == -1) {
			fprintf(stderr, "sending failed\n");
			return;
		}
	}
	cout<<"Response sent done\n";

}

int main(int argc, char* argv[]) {
	initialFileType();
	if (argc != 4) {
		if (argc == 1) {
			hostname = "localhost";
			portNum = "4000";
			fileRoot = ".";
		}
		cout<<"Invalid arguments: you should input three arguments as hostname, port number and working directory\n";
	}
	else {
		hostname = argv[1];
		portNum = argv[2];
		fileRoot = argv[3];
	}

	//Get the ip address information corresponding to hostname
	int status;
	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	int socketfd;

	if ((status = getaddrinfo(hostname, portNum, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return 1;
	}

	for (p = servinfo; p != NULL; p = (p->ai_next)) {
		if ((socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) != -1) break;
	}

	if (p == NULL) {
		fprintf(stderr, "socket creation error: no feasible socket for given hostname\n");
		return 2;
	}

	int yes = 1;
	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		fprintf(stderr, "setsocketopt error\n");
		return 3;
	}

	if (::bind(socketfd, p->ai_addr, p->ai_addrlen) == -1) {
		fprintf(stderr, "bind error\n");
		close(socketfd);
		return 4;
	}

	freeaddrinfo(servinfo);

	if (listen(socketfd, 8) == -1) {
		fprintf(stderr, "listen error\n");
		return 5;
	}

	while (1) {
		struct sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);
		cout<<"Waiting for http request...\n";
		int clientSockfd = accept(socketfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
		if (clientSockfd == -1) {
			close(socketfd);
	    	fprintf(stderr, "connection accept error\n");
	    	return 4;
	  	}
	  	cout<<"Http request accepted\n";

	  	char ipstr[INET_ADDRSTRLEN] = {'\0'};
	  	inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
	  	cout<<"Creating thread to handle the http request\n";
	  	thread t(childConnection, clientSockfd);
	  	t.detach();
  	}

}

string getFileType(string fileName) {
	int pos;
	for (pos=0;pos<fileName.size();pos++) {
		if (fileName[pos] == '.') break;
	}

	return fileType[fileName.substr(pos+1)];
}