//============================================================================
// Name        : My.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <fstream>
#include <errno.h>
#include <string>
#include <sstream>

#define WINDOWSIZE 8

using namespace std;
struct packet {
	char buffer[960];
	int TTL;
	uint16_t checksum;
	int sequenceNumber;
	int windowSize;
	bool synFlag, fileFlag;
	int dataSize;
	int noOfPackets;
	int pos;
	bool transmitted,recivedACK;
};
struct acknowledegment {
	int sequenceNumber;
};
///////////////////////////////////////////
bool writeFile(char* buffer, streampos pos);
int selectiveRepeat(int seq_num, int N, packet Packet);
void displayList();
int clientConfiguration();
uint16_t udp_checksum(const void *buff, size_t len);
///////////////////////////////////////////
///////////////////////////////////////////
std::string Server_IP, Server_port, Client_Port, File_name, Packet_size,
		ServerTimeout, ACK_number_to_drop, ACK_number_to_corrupt;
///////////////////////////////////////////
///////////////////////////////////////////
static void sig_alrm(int signo) {
	return;
}
///////////////////////////////////////////
///////////////////////////////////////////

int main() {
	int ACK = 0;
	int connectionSocket;
	int clientCommunicationSocket;
	int noOfClientsConnected = 0;
	int structSize;
	int status, status2;
	int success = clientConfiguration();
	if (!success) {
		cout << "Client configured correctly" << endl;
	}

	struct sockaddr_in serverAddress, clientAddress;

	connectionSocket = socket(AF_INET, SOCK_DGRAM, 0);

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(1256);
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	structSize = sizeof(serverAddress);

	bind(connectionSocket, (struct sockaddr *) &serverAddress, structSize);
	struct timeval timeout;
	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	if (setsockopt(connectionSocket, SOL_SOCKET, SO_RCVTIMEO, (char *) &timeout,
			sizeof(timeout)) < 0)
		cout << "setsockopt failed\n" << endl;

	/*while(true){
	 packet recieve;
	 cin >> recieve.buffer;
	 cin.ignore();
	 signal(SIGALRM, sig_alrm);
	 status = sendto(connectionSocket, &recieve, sizeof(recieve), 0,
	 (struct sockaddr *) &serverAddress, structSize);



	 if (setsockopt(connectionSocket, SOL_SOCKET, SO_SNDTIMEO, (char *) &timeout,
	 sizeof(timeout)) < 0)
	 cout << "setsockopt failed\n" << endl;
	 alarm(5);
	 packet packRecieved;
	 if ((status2 = recvfrom(connectionSocket, &packRecieved, sizeof(packRecieved),
	 0, (struct sockaddr *) &serverAddress, (socklen_t*) &structSize))< 0) {
	 cout << "Recieve timeout" << endl;
	 for (int i = 0; i < 960; i++) {
	 packRecieved.buffer[i] = '0';
	 }//end for
	 } //end if
	 else {
	 alarm(0);
	 cout << "Server has sent the following items: " << endl;
	 cout << "IP address of Client: " << inet_ntoa(serverAddress.sin_addr)
	 << endl;
	 cout << "Port Number: " << serverAddress.sin_port << endl;
	 cout << "Message is: " << packRecieved.buffer << endl;
	 }//end else
	 }//end while*/

	////////////////////////////////////
	packet receive;
	cin >> receive.buffer;
	cin.ignore();
	signal(SIGALRM, sig_alrm);
	status = sendto(connectionSocket, &receive, sizeof(receive), 0,
			(struct sockaddr *) &serverAddress, structSize);
	cout << "Waiting" << endl;
	///////////////////////////////
	alarm(5);
	if ((status2 = recvfrom(connectionSocket, &receive, sizeof(receive),
			SO_RCVTIMEO, (struct sockaddr *) &serverAddress,
			(socklen_t*) &structSize)) < 0) {
		if (errno == EINTR)
			cout << "Receive timeout" << endl;
	} else {
		alarm(0);

		//packet *write=new packet[receive.noOfPackets];
		//packet window[receive.noOfPackets];
		acknowledegment ack;
		for (int i = 0; i < receive.noOfPackets; i++) {
			alarm(5);
			//cout<<recieve.noOfPackets<<endl;
			if ((status2 = recvfrom(connectionSocket, &receive, sizeof(receive),
			MSG_WAITALL, (struct sockaddr *) &serverAddress,
					(socklen_t*) &structSize)) <= 0) {
				cout << "Receive timeout" << endl;
			} else {
				alarm(0);
				//	window[i]= receive;
				//write[i] = receive;

				cout << udp_checksum(receive.buffer, sizeof(receive.buffer))
						<< " : " << receive.checksum << endl;
				if (udp_checksum(receive.buffer, sizeof(receive.buffer))
						== receive.checksum) {
					ACK = selectiveRepeat(receive.sequenceNumber+1,
							WINDOWSIZE, receive);
					cout << "ACK returned" << ACK << endl;
					ack.sequenceNumber = ACK;
					//if(ACK!=0||ACK!=-1){i=ACK-1;}
				} else {
					ack.sequenceNumber = -1;
				}
				//writeFile(receive.buffer,sizeof(receive.buffer));

				if (ACK < receive.noOfPackets) {

					sendto(connectionSocket, &ack, sizeof(ack), 0,
							(struct sockaddr *) &serverAddress, structSize);
					cout << "Acknowledgment of packet " << ACK << " sent"
							<< endl;
				}			//end if
			}			//end else
		}			// end for loop

	} //else write file
	displayList();
	cout << "Closing Program" << endl;
	return 0;
} //end main

bool writeFile(char* buffer, streampos pos) {
	ofstream myfile;
	myfile.open(File_name.c_str(), ios::binary | ios::out | ios::app);
	myfile.write(buffer, pos);
	myfile.close();
	return true;
} //end function
///////////////////////////////////////////
///////////////////////////////////////////
int rcv_base = 1;
///////////////////////////////////////////
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct deliveredPackets {
	packet Packet;
	deliveredPackets *link;
};
deliveredPackets *head1 = NULL, *tail1 = NULL;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void displayList() {
	if (head1 == NULL && tail1 == NULL) //list is empty
	{
		cout << "\nlist is empty\n";
	} else //list not empty
	{
		deliveredPackets *temp = head1;
		while (temp != NULL) {
			//cout << "\npacket " << temp->Packet.sequenceNumber;
			writeFile(temp->Packet.buffer, sizeof(temp->Packet.buffer));
			temp = temp->link;
		}
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void addAtBackList(packet p) {
	deliveredPackets *temp = new deliveredPackets;
	temp->Packet = p;

	if (head1 == NULL && tail1 == NULL) // first element, link list is empty
	{
		head1 = temp; // our head and tail will be pointing to the same Node.
		tail1 = temp; // head=tail=temp;
	} else {
		tail1->link = temp; // first, chain the new node to the tail of existing list
		tail1 = temp; // update the tail information,
		tail1->link = NULL;
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
struct buffer {
	packet Packet;
	buffer *link;
};
buffer *head = NULL, *tail = NULL;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void addAtBackBuffer(packet p) {
	buffer *temp = new buffer;
	temp->Packet = p;

	if (head == NULL && tail == NULL) // first element, link list is empty
	{
		head = temp; // our head and tail will be pointing to the same Node.
		tail = temp; // head=tail=temp;
	} else {
		tail->link = temp; // first, chain the new node to the tail of existing list
		tail = temp; // update the tail information,
		tail->link = NULL;
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool previouslyRecieved(int seqNo) {
	if (head == NULL && tail == NULL) //buffer is empty
	{
		return false; //seqNo not recieved
	} else //buffer not empty
	{
		buffer *temp = head;
		while (temp != NULL) {
			if (temp->Packet.sequenceNumber == seqNo) //seqNo recieved
					{
				return true;
			}
			temp = temp->link;
		}
	}
	return false; //seqNo not previously recieved
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void removeConsectivePacketsFromBuffer(int seq_num) {
	if (head == NULL && tail == NULL) //buffer is empty
	{
		return; //no packets to remove
	} else //buffer not empty
	{
		buffer *temp = head;
		buffer *preTemp = head;

		while (temp != NULL) {
			cout << "\nCheck " << temp->Packet.sequenceNumber << "=="
					<< (seq_num + 1) << "\n";
			if (temp->Packet.sequenceNumber == (seq_num + 1)) //seqNo found at 1st node
					{
				cout << "" << temp->Packet.sequenceNumber
						<< " removed from buffer\n";
				addAtBackList(temp->Packet);
				temp = temp->link;
				preTemp->link = temp;
				temp = head; //packet removed
				preTemp = temp;
				rcv_base += 1;
				seq_num += 1; //look for next consective packet
				continue;
			}
			temp = temp->link;
			preTemp = temp;
		}
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int selectiveRepeat(int seq_num, int N, packet Packet) {
	int ack;
	//case 1
	if (seq_num >= rcv_base && seq_num <= (rcv_base + N - 1)) {
		if (seq_num == rcv_base) {
			cout << "\nDelivered packet: " << Packet.sequenceNumber << "\n";
			addAtBackList(Packet);
			rcv_base += 1;
			removeConsectivePacketsFromBuffer(seq_num);
			cout << "Consecutive Removed" << endl;
			ack = seq_num;
			return ack;
		}
		if (!previouslyRecieved(seq_num)) {
			cout << "added to buffer: " << seq_num << "\n";
			addAtBackBuffer(Packet);
			ack = seq_num;
			return ack;
		}
	}
	//case 2
	else if (seq_num >= (rcv_base - N) && seq_num <= (rcv_base - 1)) {
		ack = seq_num;
		return ack;
	}
	//case 3
	else {
		//do nothing
		cout << "\tUninterested packet received !\n";
		return -1; //its not ACK
	}
	return -1;
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
int clientConfiguration() {
	std::ifstream file("Read.txt");
	std::string str;
	int flag = 0;
	std::string tag;

	while (std::getline(file, str)) {
		stringstream ss(str);
		string token;

		while (getline(ss, token, ' ')) {
			if (flag == 0) {
				tag = token;
				//cout<< token << " ";
				flag = 1;
			} //end if
			else if (flag == 1) {
				//cout << token;
				flag = 0;

				if (tag == "Server_IP:")
					Server_IP = token;

				else if (tag == "Server_port:")
					Server_port = token;

				else if (tag == "Client_Port:")
					Client_Port = token;

				else if (tag == "File_name:")
					File_name = token;

				else if (tag == "Packet_size:")
					Packet_size = token;

				else if (tag == "ServerTimeout:")
					ServerTimeout = token;

				else if (tag == "ACK_number_to_drop:")
					ACK_number_to_drop = token;

				else if ("ACK_number_to_corrupt:")
					ACK_number_to_corrupt = token;

			} //end elseif
		} //end token while
	} //end file while

	cout << "\nServer_IP " << Server_IP << "\nServer_port " << Server_port
			<< "\nClient_Port " << Client_Port << "\nFile_name " << File_name
			<< "\nPacket_size " << Packet_size << "\nServerTimeout "
			<< ServerTimeout << "\nACK_number_to_drop " << ACK_number_to_drop
			<< "\nACK_number_to_corrupt " << ACK_number_to_corrupt << endl;

	return 0;
}
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
uint16_t udp_checksum(const void *buff, size_t len)		//		See Reference
		{
	const uint16_t *buf = (uint16_t *) buff;

	//      uint16_t *ip_src=(void *)&src_addr, *ip_dst=(void *)&dest_addr;

	uint32_t sum;
	size_t length = len;

	// Calculate the sum                                            //
	sum = 0;
	while (len > 1) {
		sum += *buf++;
		if (sum & 0x80000000)
			sum = (sum & 0xFFFF) + (sum >> 16);
		len -= 2;
	}

	if (len & 1)
		// Add the padding if the packet lenght is odd          //
		sum += *((uint8_t *) buf);

	// Add the carries                                              //
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);

	// Return the one's complement of sum                           //
	return ((uint16_t) (~sum));
}
/////////////////////////////////////////////////////////////////////////
