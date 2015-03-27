#include<iostream>
#include <list>
#include <vector>
using namespace std;
struct packet {
	char buffer[960];
	int TTL;
	int checksum;
	int sequenceNumber;
	int windowSize;
	int receiveBase;
	bool synFlag, fileFlag;
	int dataSize;
	int noOfPackets;
	int pos;
};
packet* buffer[];
//template < class packet, class Alloc = allocator<packet> > class list;
std::list<int> mylist;
bool packetRecieved() {
	return true;
}

void selectiveRepeat(int sequenceNumber, int windowSize, int receiveBase,
		packet packetCurrent) {
	if (sequenceNumber >= receiveBase
			&& sequenceNumber <= receiveBase + (windowSize - 1)) {
		if (!packetRecieved()) {
			mylist.push_back(packet); //add to linkedList
		}
		if (sequenceNumber == receiveBase) {
			packet.sendToUpperLayer();
			traverseLinkedList(sequenceNumber); //check consecutive sequenceNumbers after this
			//return count send to upper layer;
			receiveBase += count;
		}
		//return acknowledgement;
	}			//end if
	else if (sequenceNumber >= (receiveBase - windowSize)
			&& sequenceNumber <= receiveBase - 1) {
		//return acknowledgment;
	}else{

		cout<<"Undesired Packet Received"<<endl;
	}
}

int main(){
	return 0;
}


