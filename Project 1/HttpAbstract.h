#ifndef HttpAbstract_h
#define HttpAbstract_h

#include <iostream>
#include <unordered_map>
#include <vector>
#include <string>
#include <sstream>

using namespace std;

class HttpBase {
private:
	string version;

protected:
	unordered_map<string, string> headers;
	/* Method to encode the http headers */
	string encodeHeaders();

public:
	/* Method to get the Http version */
	string getHttpVersion();
	/* Method to set the Http version */
	void setHttpVersion(string version);
	/* Method to get Http header content by a key */
	string getHeaderByKey(string headerKey);
	/* Method to set a Http header information by a key and a corresponding value */
	void setHeader(string headerKey, string headerValue);
};

class HttpRequest: public HttpBase {
private:
	string url;
	string method;
	vector<uint8_t> optionalInfo;

	/* Method to recover the first line of http code */
	int consumeFirstLine(vector<uint8_t>& wire);

public:
	/* Method to get the url of the http request */
	string getURL();
	/* Method to set the url of the http request */
	void setURL(string url);
	/* Method to get the http method like GET POST PUT of http request */
	string getHttpMethod();
	/* Method to set the http method like GET POST PUT of http request */
	void setHttpMethod(string method);
	/* Method to get the optional message body of the http request */
	vector<uint8_t> getOptionalInfo();
	/* Method to set the optional message body of the http request */
	void setOptionalInfo(vector<uint8_t> optionalinfo);
	/* Method to encode the http request */
	vector<uint8_t> encode();
	/* Method to recover the http request object based on encoded http request information */
	bool consume(vector<uint8_t> wire);
};

class HttpResponse: public HttpBase {
private:
	string status;
	string statusInfo;
	vector<uint8_t> data;

	/* Method to recover the first line of http code */
	int consumeFirstLine(vector<uint8_t>& wire);

public:
	/* Method to get the http response status */
	string getStatus();
	/* Method to set the http response status */
	void setStatus(string status);
	/* Method to get the http response status information */
	string getStatusInfo();
	/* Method to set the http response status information */
	void setStatusInfo(string statusInfo);
	/* Method to get the http response data body */
	vector<uint8_t> getData();
	/* Method to set the http response data body */
	void setData(vector<uint8_t>);
	/* Method to encode the http response */
	vector<uint8_t> encode(vector<uint8_t> &data);
	/* Method to recover the http response object based on encoded http response information */
	bool consume(vector<uint8_t> wire);
};

#endif