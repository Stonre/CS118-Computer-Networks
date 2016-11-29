#include "TCPHeader.h"

/**
	Implementation for class TCPPacket
*/

uint16_t TCPPacket::getSeqNum() {
	return seqNum;
}

void TCPPacket::setSeqNum(uint16_t seqNum) {
	this->seqNum = seqNum;
}

uint16_t TCPPacket::getAckNum() {
	return ackNum;
}

void TCPPacket::setAckNum(uint16_t ackNum) {
	this->ackNum = ackNum;
}

uint16_t TCPPacket::getWinSize() {
	return winSize;
}

void TCPPacket::setWinSize(uint16_t winSize) {
	this->winSize = winSize;
}

bool TCPPacket::getACK() {
	return ack;
}

void TCPPacket::setACK(bool ack) {
	this->ack = ack;
}

bool TCPPacket::getSYN() {
	return syn;
}

void TCPPacket::setSYN(bool syn) {
	this->syn = syn;
}

bool TCPPacket::getFIN() {
	return fin;
}

void TCPPacket::setFIN(bool fin) {
	this->fin = fin;
}

std::vector<uint8_t> TCPPacket::getData() {
	return data;
}

void TCPPacket::setData(std::vector<uint8_t> data) {
	this->data = data;
}

std::vector<uint8_t> TCPPacket::encode() {
	std::vector<uint8_t> resCode;
	resCode.push_back((uint8_t)((seqNum & 0xFF00) >> 8));
	resCode.push_back((uint8_t)(seqNum & 0x00FF));
	resCode.push_back((uint8_t)((ackNum & 0xFF00) >> 8));
	resCode.push_back((uint8_t)(ackNum & 0x00FF));
	resCode.push_back((uint8_t)((winSize & 0xFF00) >> 8));
	resCode.push_back((uint8_t)(winSize & 0x00FF));
	resCode.push_back((uint8_t)(0x00));
	resCode.push_back((uint8_t)((ack << 2) + (syn << 1) + fin));

	return resCode;
}

void TCPPacket::consume(std::vector<uint8_t> wire) {
	seqNum = (wire[0] << 8) + wire[1];
	ackNum = (wire[2] << 8) + wire[3];
	winSize = (wire[4] << 8) + wire[5];
	ack = ((wire[7] & 0x04) == 0x04);
	syn = ((wire[7] & 0x02) == 0x02);
	fin = ((wire[7] & 0x01) == 0x01);
}

/**
	Implemention for class PacketRecord
*/
uint16_t PacketRecord::getSeqNum() {
	return seqNum;
}

void PacketRecord::setSeqNum(uint16_t seqNum) {
	this->seqNum = seqNum;
}

int PacketRecord::getNumOfRTS() {
	return numOfRTS;
}

void PacketRecord::setNumOfRTS(int numOfRTS) {
	this->numOfRTS = numOfRTS;
}

int PacketRecord::getDataSize() {
	return dataSize;
}

void PacketRecord::setDataSize(int dataSize) {
	this->dataSize = dataSize;
}

// double PacketRecord::getSentTimeStamp() {
// 	return sentTimeStamp;
// }

// void PacketRecord::setSentTimeStamp(double sentTimeStamp) {
// 	this->sentTimeStamp = sentTimeStamp;
// }

// bool PacketRecord::getIsInTransmit() {
// 	return isInTransmit;
// }

// void PacketRecord::setIsInTransmit(bool isInTransmit) {
// 	this->isInTransmit = isInTransmit;
// }

// bool PacketRecord::getIsAcked() {
// 	return isAcked;
// }

// void PacketRecord::setIsAcked(bool isAcked) {
// 	this->isAcked = isAcked;
// }

std::vector<uint8_t> PacketRecord::getPacket() {
	return packet;
}

void PacketRecord::setPacket(std::vector<uint8_t> packet) {
	this->packet = packet;
}

/**
	Implementation of the ReceivedPacketRecord
*/
ReceivedPacketRecord::ReceivedPacketRecord(int seqNum, int ackNum, std::vector<uint8_t> data) {
	this->seqNum = seqNum;
	this->ackNum = ackNum;
	this->data = data;
}
int ReceivedPacketRecord::getSeqNum() {
	return seqNum;
}

void ReceivedPacketRecord::setSeqNum(int seqNum) {
	this->seqNum = seqNum;
}

std::vector<uint8_t> ReceivedPacketRecord::getData() {
	return data;
}

void ReceivedPacketRecord::setData(std::vector<uint8_t> data) {
	this->data = data;
}

/**
	Implementation of SenderControl
*/
SenderControl::SenderControl(double startTime) {
	RTO = 500;
	cwinSize = MIN_CWND_SIZE;
	freeBufferSize = RCV_WIN_SIZE;
	numOfDupAcks = 0;
	this->startTime = startTime;
	ssthresh = 15360;
	congestionState = 0;
}

double SenderControl::getStartTime() {
	return startTime;
}

void SenderControl::setStartTime(double startTime) {
	this->startTime = startTime;
}

int SenderControl::getCwinSize() {
	return cwinSize;
}

void SenderControl::setCwinSize(int cwinSize) {
	this->cwinSize = cwinSize;
} 

int SenderControl::getFreeBufferSize() {
	return freeBufferSize;
}

void SenderControl::setFreeBufferSize(int freeBufferSize) {
	this->freeBufferSize = freeBufferSize;
}

std::vector<uint8_t> SenderControl::getFirstUnAckedPacket() {
	PacketRecord pr = cwin.front();
	return pr.getPacket();
}

bool SenderControl::isTimeout(double currentTime) {
	return (currentTime - startTime) >= RTO;
}

// bool SenderControl::isThreeDupAcks() {
// 	return numOfDupAcks >= 3;
// }

// void SenderControl::incrementDupAcks() {
// 	numOfDupAcks++;
// }

// void SenderControl::resetDupAcks() {
// 	numOfDupAcks = 0;
// }

void SenderControl::timeoutAct() {
	ssthresh = std::max(cwinSize/2, MIN_CWND_SIZE);
	cwinSize = MIN_CWND_SIZE;
	congestionState = 0;
	numOfDupAcks = 0;
}

int SenderControl::ackPacket(int ackNum, double time) {
	if (ackNum == cwin.front().getSeqNum()) {
		numOfDupAcks++;
		if (numOfDupAcks >= 3) {
			numOfDupAcks = 0;
			ssthresh = std::max(cwinSize/2, MIN_CWND_SIZE);
			cwinSize = MIN_CWND_SIZE;
			congestionState = 0;
			return 1;
		}

		return 2;
	}
	else if (ackNum < cwin.front().getSeqNum()) {
		return 3;
	}

	while (cwin.size() > 0 && cwin.front().getSeqNum() < ackNum) {
		sendBase += cwin.front().getDataSize();
		lastCwinSize--;
		if (congestionState == 0) {
			cwinSize += MIN_CWND_SIZE;
			if (cwinSize >= ssthresh) {
				congestionState = 1;
				lastCwinSize = cwinSize;
			}
		}
		else {
			if (lastCwinSize == 0) {
				cwinSize += MIN_CWND_SIZE;
				lastCwinSize = cwinSize;
			}
		}
		cwin.pop();
	}

	startTime = time;

	return 0;
}

void SenderControl::AddPacketRecord(PacketRecord p) {
	cwin.push(p);
}

/**
	Implementation of ReceiverBufferManager
*/
ReceiverBufferManager::ReceiverBufferManager() {
	bufferStart = -1;
	bufferEnd = -1;
	freeBufferSize = RCV_WIN_SIZE;
}
int ReceiverBufferManager::getFreeBufferSize() {
	return freeBufferSize;
}

void ReceiverBufferManager::setFreeBufferSize(int freeBufferSize) {
	this->freeBufferSize = freeBufferSize;
}

int ReceiverBufferManager::getNextExpectedSeq() {
	return nextExpectedSeq;
}

void ReceiverBufferManager::setNextExpectedSeq(int nextExpectedSeq) {
	this->nextExpectedSeq = nextExpectedSeq;
}

int ReceiverBufferManager::addPacketRecord(TCPPacket p) {
	if (recvCheckWin.find(p.getSeqNum()) != recvCheckWin.end()) {
		return 1;
	}

	if (p.getSeqNum() == nextExpectedSeq) {
		for (uint8_t i: p.getData()) {
			if (bufferEnd == RCV_WIN_SIZE) {
				bufferEnd = 0;
				receiveBuffer[bufferEnd] = (char)i;
			}
			else receiveBuffer[++bufferEnd] = char(i);
			freeBufferSize--;
		}
		nextExpectedSeq = p.getSeqNum() + p.getData().size();
		while (rwin[0].getSeqNum() == nextExpectedSeq) {
			for (uint8_t i: rwin[0].getData()) {
				if (bufferEnd == RCV_WIN_SIZE) {
					bufferEnd = 0;
					receiveBuffer[bufferEnd] = (char)i;
				}
				else receiveBuffer[++bufferEnd] = char(i);
				freeBufferSize--;
			}
			nextExpectedSeq = rwin[0].getSeqNum() + rwin[0].getData().size();
			rwin.erase(rwin.begin());
		}

		return 2;
	}

	ReceivedPacketRecord recvRecord(p.getSeqNum(), p.getAckNum(), p.getData());
	for (std::vector<ReceivedPacketRecord>::iterator it = rwin.begin(); it != rwin.end(); it++) {
		if (it->getSeqNum() > recvRecord.getSeqNum()) {
			rwin.insert(it, recvRecord);
			recvCheckWin.insert(recvRecord.getSeqNum());
			return 3;
		}
	}

	rwin.insert(rwin.end(), recvRecord);

	return 3;
}

std::vector<char> ReceiverBufferManager::bufferRead() {
	std::vector<char> res;
	if (bufferStart <= bufferEnd) {
		for (int i=bufferStart;i<=bufferEnd;i++) res.push_back(receiveBuffer[i]);
	}
	else {
		for (int i=bufferStart;i<=(bufferEnd+RCV_WIN_SIZE);i++) res.push_back(receiveBuffer[i%RCV_WIN_SIZE]);
	}
	bufferStart = -1;
	bufferEnd = -1;

	return res;
}