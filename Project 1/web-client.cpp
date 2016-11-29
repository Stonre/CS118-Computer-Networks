#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/time.h>
#include <fstream>
#include "URLParser.h"
#include "HttpAbstract.h"

using namespace std;

#define CHK_SIZE 1000
#define TIME_OUT 1
#define FIRST_TIME_OUT 3

int recvFromServer(int sockfd, string file)
{
	vector<uint8_t> res_v;
    int total_len = 0, recv_len = 0;
    struct timeval begin , now;
    char buff[CHK_SIZE];
    double timediff;
     
    //beginning time
    gettimeofday(&begin , NULL);
    bool isFirst = true;

    ofstream outfile;
    
    while(1)
    {
    	memset(buff ,0 , CHK_SIZE);  //clear the variable
        if((recv_len =  recv(sockfd , &buff , CHK_SIZE , 0) ) <= 0)
        {
            usleep(100000);
        }
        else
        {
            gettimeofday(&begin , NULL);
            
            //cout << buff << endl;
            total_len += recv_len;
			vector<uint8_t> res_vtmp(buff, buff + recv_len);
			if(isFirst) {
				res_v.insert(res_v.end(), res_vtmp.begin(), res_vtmp.end());
				//parse response
			    HttpResponse response;
			    if(response.consume(res_v)) {
			    	cout << response.getStatus() << ' ' << response.getStatusInfo() << endl;
			    	if(response.getStatus() != "200") return -1;
			        res_v.clear();		        
			        outfile.open(file);
			        isFirst = false;
			        vector<uint8_t> data = response.getData();
			        cout << "success!" << endl;
			        outfile << string(data.begin(), data.end());
			        data.clear();
			    }
			}
			else {
				outfile << string(res_vtmp.begin(), res_vtmp.end());
			}

		    res_vtmp.clear();

        }

        gettimeofday(&now , NULL);
         
        //time elapsed in seconds
        timediff = (now.tv_sec - begin.tv_sec) + 1e-6 * (now.tv_usec - begin.tv_usec);
         
        //if you got some data, then break after timeout
        if( total_len > 0 && timediff > TIME_OUT )
        {
            break;
        }
         
        //if you got no data at all, wait a little longer, twice the timeout
        else if( timediff > FIRST_TIME_OUT)
        {
            break;
        }
    }
     
    outfile.close();
    return total_len;
}

const char* formRequest(string method, string url, string host, string port) {
    HttpRequest req;
    req.setURL(url);
    req.setHttpMethod(method);
    req.setHttpVersion("1.0");
    req.setHeader("Host", host + ":" + port);
    req.setHeader("User-Agent", "Wget/1.15 (linux-gnu)");
    req.setHeader("Accept", "*/*");
    vector<uint8_t> req_v = req.encode();
    //const char *request = reinterpret_cast<const char*>(req_v.data());
    const char *request = string(req_v.begin(), req_v.end()).c_str();
    req_v.clear();
    return request;
}

string getFileName(string path) {
	string file = "index.html";
	int path_len = path.size(), last_pos;
	if(path_len == 0) return file;
	for(last_pos = path_len - 1; last_pos >= 0; last_pos --) {
		if(path[last_pos] == '/') break;
	}
	if(last_pos == path_len - 1) return file;
	return path.substr(last_pos + 1);
}

int main(int argc, char *argv[]) {
    struct addrinfo hints;
    struct addrinfo *res, *p;

    if (argc < 2) {
        cerr << "No input URL" << endl;
        return -1;
    }

    int sockfd;

    // prepare hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET; // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP

    // get address
    int status = 0;
    for(int i = 1; i < argc; i ++) {
        cout << "================================" << endl;
        cout << "Request " << i << " : "<< argv[i] << endl;
        string urlstr(argv[i]);
        HTTPURL url(urlstr);
        string protocol = url.protocol;
        string host = url.domain;
        string port = to_string(url.port);
        string path = url.path;

        /*cout << "protocol: " << protocol << endl;
        cout << "host: " << host << endl;
        cout << "port: " << port << endl;
        cout << "path: " << path << endl << endl;*/

        status = getaddrinfo(host.c_str(), port.c_str(), &hints, &res);
        //status = getaddrinfo(argv[i], "80", &hints, &res);

        if (status != 0) {
            cout << "getaddrinfo: " << gai_strerror(status) << endl;
            continue;
        }
        for(p = res; p != NULL; p = p -> ai_next) {
            sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
            if(sockfd == -1) {
                cerr << "client: socket" << endl;
                continue;
            }
            if(connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
                close(sockfd);
                cerr << "client: connect" << endl;
                continue;
            }
            break;
        }
        if (p == NULL) {
            cerr << "client: failed to connect" << endl;
            continue;
        }

        freeaddrinfo(res);

        //form http request
        const char* request = formRequest("GET", path, host, port);

        //send http request through socket
        cout << "Sending Request:" << endl;
        cout << request << endl;
        int byte_sent = 0;
        byte_sent = send(sockfd, request, strlen(request), 0);
        if(byte_sent < 0) {
            perror("send");
            continue;
        }

        //receive http response from server
        cout << "Receiving Response: " << endl;
        fcntl(sockfd, F_SETFL, O_NONBLOCK);
        string file_name = getFileName(path);
        int total_len = recvFromServer(sockfd, file_name);
        if(total_len > 0) {
        	cout << "File saved! File length is: " << total_len << endl << endl;
        }
        
		close(sockfd);
    }

}