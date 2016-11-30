#ifndef TCPHEADER_H
#define TCPHEADER_H

#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>
#include <thread>
#include <stdlib.h>
#include <stdint.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <unordered_map>
#include <queue>
#include <unordered_set>
#include <string>
#include <map>
#include <vector>
#include <cstdint>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sstream>
#include <fstream>

#define MAX_PACKET_LEN 1032 //bytes
#define MAX_SEQ_NUM 30720 //bytes
#define MIN_CWND_SIZE 1024 //bytes
#define SLOW_START_THRES 15360 //bytes
#define RCV_WIN_SIZE 15360 //bytes
#define ALPHA 0.125
#define BETA 0.25

//Data structure for a TCP packet
class TCPPacket {
private:
	uint16_t seqNum;
	uint16_t ackNum;
	uint16_t winSize;
	bool ack;
	bool syn;
	bool fin;
	std::vector<uint8_t> data;

public:
	//Get the sequence number of the packet
	uint16_t getSeqNum();
	//Set the sequence number of the packet
	void setSeqNum(uint16_t seqNum);
	//Get the acknowledge number of the packet
	uint16_t getAckNum();
	//Set the acknowledge number of the packet
	void setAckNum(uint16_t ackNum);
	//Get the acceptable window size of the packet
	uint16_t getWinSize();
	//Set the acceptable window size of the packet
	void setWinSize(uint16_t winSize);
	//Get the acknowledge flag
	bool getACK();
	//Set the acknowledge flag
	void setACK(bool ack);
	//Get the syn flag
	bool getSYN();
	//Set the syn flag
	void setSYN(bool syn);
	//Get the fin flag
	bool getFIN();
	//Set the fin flag
	void setFIN(bool fin);
	//Get the data part of the packet
	std::vector<uint8_t> getData();
	//Set the data part of the packet
	void setData(std::vector<uint8_t> data);
	//Encode the packet for sending
	std::vector<uint8_t> encode();
	//Consume the data from encoded packet
	void consume(std::vector<uint8_t> wire);
};

//Data structure for a server packet record for congestion control
class PacketRecord {
private:
	uint16_t seqNum;
	int numOfRTS;
	int dataSize;
	// double sentTimeStamp;
	// bool isInTransmit;
	// bool isAcked;
	std::vector<uint8_t> packet;

public:
	//Get the sequence number of the packet
	uint16_t getSeqNum();
	//Set the sequence number of the packet
	void setSeqNum(uint16_t seqNum);
	//Get the number of retransmissions of the packet
	int getNumOfRTS();
	//Set the number of retransmissions of the packet
	void setNumOfRTS(int numOfRTS);
	//Get the size of the packet's data
	int getDataSize();
	//Set the size of the packet's data
	void setDataSize(int dataSize);
	// //Get the time stamp of sending the packet
	// double getSentTimeStamp();
	// //Set the time stamp of sending the packet
	// void setSentTimeStamp(double sentTimeStamp);
	//Get whether the packet is in transmission
	// bool getIsInTransmit();
	// //Set whether the packet is in transmission
	// void setIsInTransmit(bool isInTransmit);
	// //Get whether the packet is acked
	// bool getIsAcked();
	// //Set whether the packet is acked
	// void setIsAcked(bool isAcked);
	//Get the packet payload
	std::vector<uint8_t> getPacket();
	//Set the packet payload
	void setPacket(std::vector<uint8_t> packet);
};

//Class to handle flow and congestion control
class SenderControl {
private:
	int cwinSize;
	int freeBufferSize;
	int sendBase;
	int nextSend;
	int numOfDupAcks;
	int newAckedSeqNum;
	double startTime;
	double RTO;
	double SRTT;
	double DevRTT;
	double ssthresh;
	int lastCwinSize;
	int congestionState;//0 - slow start; 1 - congestion avoidance
	std::queue<PacketRecord> cwin;
	std::map<int, int> cwinMap;

public:
	SenderControl(double startTime);
	//Get the start time of the oldest packet that is unacked
	double getStartTime();
	//Set the start time of the oldest packet that is unacked
	void setStartTime(double startTime);
	//Get the congestion control window size
	int getCwinSize();
	//Set the congestion control window size
	void setCwinSize(int cwinSize);
	//Get the free receive buffer size
	int getFreeBufferSize();
	//Set the free receive buffer size value
	void setFreeBufferSize(int freeBufferSize);
	//Get the oldest unacked packet in the cwindow
	std::vector<uint8_t> getFirstUnAckedPacket();
	//Tell whether the oldest unacked packet time out
	bool isTimeout(double currentTime);
	//Handle timeout event
	void timeoutAct();
	//Tell whether there are three duplicate acks for a single packet
	// bool isThreeDupAcks();
	// //Increase the number of duplicate ack
	// void incrementDupAcks();
	// //Reset number of duplicate acks
	// void resetNumOfDupAcks();
	// //Ack some packet in the congestion control window
	int ackPacket(int seqNum, double time);
	//Add a PacketRecord
	void AddPacketRecord(PacketRecord p);
};

//Client
//Data structure for Client packet record
class ReceivedPacketRecord {
private:
	int seqNum;
	int ackNum;
	std::vector<uint8_t> data;

public:
	ReceivedPacketRecord(int seqNum, int ackNum, std::vector<uint8_t> data);
	//Get the sequence number of the packet
	int getSeqNum();
	//Set the sequence number of the packet
	void setSeqNum(int seqNum);
	//Get the packet paylaod
	std::vector<uint8_t> getData();
	//Set the packet payload
	void setData(std::vector<uint8_t> data);
};

//Class to handle the data buffer in the client
class ReceiverBufferManager {
private:
	std::vector<ReceivedPacketRecord> rwin;
	char receiveBuffer[RCV_WIN_SIZE];
	std::unordered_set<int> recvCheckWin;
	int bufferStart;
	int bufferEnd;
	int freeBufferSize;
	int nextExpectedSeq;

public:
	ReceiverBufferManager();
	//Get the free buffer size of the receiver
	int getFreeBufferSize();
	//Set the free buffer size of the receiver
	void setFreeBufferSize(int freeBufferSize);
	//Get the next expected sequence number
	int getNextExpectedSeq();
	//Set the next expected sequence number
	void setNextExpectedSeq(int nextExpectedSeq);
	//Add a packet record in the receiver buffer
	int addPacketRecord(TCPPacket p);
	//Read the data of the buffer
	std::vector<char> bufferRead();
};

#endif