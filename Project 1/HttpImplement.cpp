#include "HttpAbstract.h"

string HttpBase::getHttpVersion() {
	return version;
}

void HttpBase::setHttpVersion(string version) {
	this->version = version;
}

string HttpBase::getHeaderByKey(string headerKey) {
	for (int i=0;i<headerKey.size();i++) {
		if (headerKey[i] >= 65 && headerKey[i]<=90) {
			headerKey[i] = headerKey[i] - 'A' + 'a';
		}
	}
	return headers[headerKey];
}

void HttpBase::setHeader(string headerKey, string headerValue) {
	for (int i=0;i<headerKey.size();i++) {
		if (headerKey[i] >= 65 && headerKey[i]<=90) {
			headerKey[i] = headerKey[i] - 'A' + 'a';
		}
	}
	headers[headerKey] = headerValue;
}

string HttpBase::encodeHeaders() {
	string headerCode = "";

	for (unordered_map<string, string>::iterator it = headers.begin(); it != headers.end(); it++) {
		headerCode = headerCode + it->first + ": " + it->second + "\r\n";
	}

	return headerCode;
}

string HttpRequest::getURL() {
	return url;
}

void HttpRequest::setURL(string url) {
	this->url = url;
}

string HttpRequest::getHttpMethod() {
	return method;
}

void HttpRequest::setHttpMethod(string method) {
	this->method = method;
}

vector<uint8_t> HttpRequest::getOptionalInfo() {
	return optionalInfo;
}

void HttpRequest::setOptionalInfo(vector<uint8_t> optionalInfo) {
	this->optionalInfo = optionalInfo;
}

int HttpRequest::consumeFirstLine(vector<uint8_t>& wire) {
	int pointer = 0;
	int cont = 1;

	while (wire[pointer] != '\r') pointer++;
	string firstLine(wire.begin(), wire.begin()+pointer);
	istringstream iss(firstLine);

	while (iss) {
		string info;
		iss >> info;
		switch (cont) {
			case 1: {
				if (info != "GET") {
					return -1;
				}
				setHttpMethod(info);
				break;
			}
			case 2: {
				setURL(info);
				break;
			}
			case 3: {
				if (!(info == "HTTP/1.1" || info == "HTTP/1.0" || info == "HTTP/2.0")) {
					return -1;
				}
				setHttpVersion(info.substr(info.size()-3));
				break;
			}
		}
		cont++;
	}

	if (cont<4) {
		return -1;
	}

	return pointer + 2;
}

vector<uint8_t> HttpRequest::encode() {
	string httpRequestCodeString = "";
	vector<uint8_t> httpRequestCode;
	vector<uint8_t> optionalInfo = getOptionalInfo();
	//Encode the first line of the http request
	httpRequestCodeString = httpRequestCodeString + getHttpMethod() + " " + url + " " + "HTTP/" + getHttpVersion() + "\r\n" + encodeHeaders() + "\r\n";

	for (int i=0;i<httpRequestCodeString.size();i++) {
		httpRequestCode.push_back(httpRequestCodeString[i]);
	}

	if (optionalInfo.size()!=0) {
		httpRequestCode.insert(httpRequestCode.end(), optionalInfo.begin(), optionalInfo.end());
	}

	return httpRequestCode;
}

bool HttpRequest::consume(vector<uint8_t> wire) {
	int lenWire = wire.size();
	bool isEnd = false;
	bool isEndOfName = false;

	int pointerL, pointerR;
	int lineStart = consumeFirstLine(wire);
	if (lineStart == -1)
		return false;
	int i;
	for (i=lineStart; i<lenWire; i++) {
		if (!isEndOfName && wire[i] == ':') {
			isEndOfName = true;
			pointerL = i - 1;
		}
		if (wire[i] == '\r') {
			if (lenWire-i<4) return false;
			pointerR = i - 1;
			string key(wire.begin()+lineStart, wire.begin()+pointerL+1), value(wire.begin()+pointerL+2, wire.begin()+pointerR+1);
			lineStart = i+2;
			for (int i=0;i<key.size();i++) {
				if (key[i] >= 65 && key[i]<=90) {
					key[i] = key[i] - 'A' + 'a';
				}
			}
			headers[key] = value;
			isEndOfName = false;
			if (i+3 < lenWire && wire[i+1] == '\n' && wire[i+2] == '\r' && wire[i+3] == '\n') {
				isEnd = true;
				i = i + 4;
				break;
			}
		}
	}

	if (isEnd && i <= lenWire-1) {
		vector<uint8_t> tmpOptional;
		tmpOptional.assign(wire.begin()+i, wire.end());
		setOptionalInfo(tmpOptional);
	}

	return isEnd;
}

int HttpResponse::consumeFirstLine(vector<uint8_t>& wire) {
	int pointer = 0;
	int cont = 1;
	string statusInfo = "";

	while (wire[pointer] != '\r') pointer++;
	string firstLine(wire.begin(), wire.begin()+pointer);
	istringstream iss(firstLine);

	while (iss) {
		string info;
		iss >> info;
		switch (cont) {
			case 1: {
				if (!(info == "HTTP/1.1" || info == "HTTP/1.0" || info == "HTTP/2.0"))
					return -1;
				setHttpVersion(info.substr(info.size()-3));
				break;
			}
			case 2: {
				setStatus(info);
				break;
			}
			case 3: {
				statusInfo += info;
				break;
			}
			case 4: {
				statusInfo += (" " + info);
				break;
			}
		}
		cont++;
	}
	setStatusInfo(statusInfo);

	if (cont < 4) return -1;

	return pointer + 2;
}

string HttpResponse::getStatus() {
	return status;
}

void HttpResponse::setStatus(string status) {
	this->status = status;
}

string HttpResponse::getStatusInfo() {
	return statusInfo;
}

void HttpResponse::setStatusInfo(string statusInfo) {
	this->statusInfo = statusInfo;
}

vector<uint8_t> HttpResponse::getData() {
	return data;
}

void HttpResponse::setData(vector<uint8_t> data) {
	this->data = vector<uint8_t>(data.size(), 0);
	for (int i=0;i<data.size();i++) this->data[i] = data[i];
	//this->data = data;
}

vector<uint8_t> HttpResponse::encode(vector<uint8_t>& data) {
	string httpResponseCodeString = "";
	vector<uint8_t> httpResponseCode;
	//vector<uint8_t> data = getData();

	httpResponseCodeString = httpResponseCodeString + "HTTP/" + getHttpVersion() + " " + getStatus() + " " + getStatusInfo() + "\r\n" + encodeHeaders() + "\r\n";

	for (int i=0;i<httpResponseCodeString.size();i++) {
		httpResponseCode.push_back(httpResponseCodeString[i]);
	}
	if (data.size()!=0) {
		httpResponseCode.insert(httpResponseCode.end(), data.begin(), data.end());
	}

	return httpResponseCode;
}

bool HttpResponse::consume(vector<uint8_t> wire) {
	int lenWire = wire.size();
	bool isEnd = false;
	bool isEndOfName = false;

	int pointerL, pointerR;
	int lineStart = consumeFirstLine(wire);
	if (lineStart == -1)
		return false;
	int i;
	for (i=lineStart; i<lenWire; i++) {
		if (!isEndOfName && wire[i] == ':') {
			isEndOfName = true;
			pointerL = i - 1;
		}
		if (wire[i] == '\r') {
			if (lenWire-i<4) return false;
			pointerR = i - 1;
			string key(wire.begin()+lineStart, wire.begin()+pointerL+1), value(wire.begin()+pointerL+2, wire.begin()+pointerR+1);
			for (int i=0;i<key.size();i++) {
				if (key[i] >= 65 && key[i]<=90) {
					key[i] = key[i] - 'A' + 'a';
				}
			}
			headers[key] = value;
			isEndOfName = false;
			if (i+3 < lenWire && wire[i+1] == '\n' && wire[i+2] == '\r' && wire[i+3] == '\n') {
				isEnd = true;
				i = i + 4;
				break;
			}
		}
	}

	if (isEnd && i <= lenWire-1) {
		vector<uint8_t> data;
		data.assign(wire.begin()+i, wire.end());
		setData(data);
	}

	return isEnd;
}