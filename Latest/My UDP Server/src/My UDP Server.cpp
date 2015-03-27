//============================================================================
// Name        : My.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include <iostream>
#include <fstream>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sstream>
#include <string>
using namespace std;
////////////////////////////////////////////////////////
#define PACKETSIZECLIENT 1000
#define headerSize 40
struct packet {
	char buffer[PACKETSIZECLIENT - headerSize];
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
////////////////////////////////////////////////////////
char * readFile(string fileName, streampos * pos);
bool writeFile(char* buffer, streampos pos);
uint16_t udp_checksum(const void *buff, size_t len);
int serverConfiguration();
void addAtBackList(packet p);
void makePackets(int noOfPackets, int dataSize, streampos pos,
		char* buffer) ;
void addAtBackTransmitted(packet p);
void addAtBackWindow(packet p);
bool windowPacketsTransmitted();
void transmitPackets();
////////////////////////////////////////////////////////
int send_base=0, max_base=8;
////////////////////////////////////////////////////////
std::string Server_port, File_name, Packet_size, Packet_number_to_drop,
		Packet_number_to_corrupt;
////////////////////////////////////////////////////
struct notTransmittedPackets {
	packet Packet;
	notTransmittedPackets *link;
};
notTransmittedPackets *head1 = NULL, *tail1 = NULL;
///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
struct TransmittedPackets {
	packet Packet;
	TransmittedPackets *link;
};
TransmittedPackets *head = NULL, *tail = NULL;
/////////////////////////////////////////////////////
struct WindowPackets {
	packet Packet;
	int windowNumber;
	WindowPackets *link;
};
WindowPackets *headW = NULL, *tailW = NULL;
/////////////////////////////////////////////////////
struct acknowledgment {
	int sequenceNumber;
};
///////////////////////////////////////////
static void sig_alrm(int signo) {
	return;
}
//////////////////////////////////////////////
//////////////////////////////////////////////
packet removeFromFront(){
	if(head1==NULL){
		cout<<"No Packets in buffer"<<endl;
		packet temp;
		temp.sequenceNumber=-1;
		return temp;
	}

		notTransmittedPackets* tempPoint;
		tempPoint=head1;
		packet temp=head1->Packet;
		head1=head1->link;
		delete tempPoint;
		return temp;
}
///////////////////////////////////////////
int main(){
	int success = serverConfiguration();
	if (!success) {
		cout << "Server Configured Correctly" << endl;
	}
	streampos pos;
	int status2;
	char* buffer = readFile(File_name, &pos);
	cout << File_name << endl;
	int dataSize = PACKETSIZECLIENT - headerSize;
	int noOfPackets = (pos / dataSize) + 1; //pos is file size +1 for just in case for points
	cout << "File Size: " << pos << endl;
	cout << "No of packets: " << noOfPackets << endl;
//packet *read=new packet[noOfPackets];
///////////////////////////code was here
//packet write[noOfPackets];

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
	int connectionSocket;
	int clientCommunicationSocket;
//	int noOfClientsConnected = 0;
	int structSize;
	int serverPort;
	istringstream(Server_port.c_str()) >> serverPort;

	struct sockaddr_in serverAddress, clientAddress;

	connectionSocket = socket(AF_INET, SOCK_DGRAM, 0);

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	structSize = sizeof(serverAddress);

	bind(connectionSocket, (struct sockaddr *) &serverAddress, structSize);
	packet recieve;
	while (true) {
		listen(connectionSocket, 10);

		cout << "Looking for client" << endl;

		recvfrom(connectionSocket, &recieve, sizeof(recieve), 0,
				(struct sockaddr *) &clientAddress, (socklen_t*) &structSize);
		signal(SIGALRM, sig_alrm);
		if (fork() == 0) {

			clientCommunicationSocket = socket(AF_INET, SOCK_DGRAM, 0);
			bind(clientCommunicationSocket, (struct sockaddr *) &clientAddress,
					structSize);
			struct timeval timeout;
			timeout.tv_sec = 10;
			timeout.tv_usec = 0;
			if (setsockopt(clientCommunicationSocket, SOL_SOCKET, SO_RCVTIMEO,
					(char *) &timeout, sizeof(timeout)) < 0)
				cout << "setsockopt failed\n" << endl;
			cout << "Message from client to server is :" << recieve.buffer
					<< endl;
			cout << "IP address of Client: "
					<< inet_ntoa(clientAddress.sin_addr) << endl;
			cout << "Port Number: " << clientAddress.sin_port << endl;
			cout << "before while loop" << endl;
			recieve.noOfPackets = noOfPackets;
			recieve.dataSize = dataSize;
			recieve.pos = pos;
			sendto(clientCommunicationSocket, &recieve, sizeof(recieve), 0,
					(struct sockaddr *) &clientAddress, structSize);

			acknowledgment ack;
			int expectedSequenceNumber = 1;

			makePackets(noOfPackets,dataSize,pos,buffer);
				  //sleep(6);
				//here
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
			packet read=removeFromFront();
			while (read.sequenceNumber != -1) {
				cout<<read.sequenceNumber<<endl;
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
				if (read.sequenceNumber >= send_base
						&& read.sequenceNumber <= max_base) {
					//send data
					sendto(clientCommunicationSocket, &read, sizeof(read),
					MSG_WAITALL, (struct sockaddr *) &clientAddress,
							structSize);
					read.transmitted = true;
					addAtBackWindow(read);
					addAtBackTransmitted(read);
				} else {
					//return data to upper layer
					//sendDataToUpperLayer();
					read.transmitted = false;
					addAtBackWindow(read);
					//addAtBackList(read);
				}					//end else
				alarm(5);
				int status = recvfrom(clientCommunicationSocket, &ack,
						sizeof(ack), SO_RCVTIMEO,
						(struct sockaddr *) &clientAddress,
						(socklen_t*) &structSize);
				if (status < 0) {
					cout << "Receive timeout" << endl;
				}
				if (ack.sequenceNumber == expectedSequenceNumber++) {
					alarm(0);
					cout << "Acknowledgment of packet " << ack.sequenceNumber
							<< " received" << send_base<<"-"<<max_base<<endl;
					cout<<read.sequenceNumber<<"-"<<ack.sequenceNumber<<endl;
					if (read.sequenceNumber == ack.sequenceNumber-1) {
						if (read.sequenceNumber >= send_base
								&& read.sequenceNumber <= max_base) {
							read.recivedACK = true;
							if (read.sequenceNumber == send_base) {
								send_base += 1;
								max_base += 1;
								if (!windowPacketsTransmitted()) {

									WindowPackets *temp = headW;
									while (temp != NULL) //for all window packets
									{
										if (temp->windowNumber >= send_base
												&& temp->windowNumber
														>= max_base)
											if (temp->Packet.transmitted
													== false) {
												//transmit packet. work here
												sendto(
														clientCommunicationSocket,
														&temp->Packet,
														sizeof(temp->Packet),
														MSG_WAITALL,
														(struct sockaddr *) &clientAddress,
														structSize);
												temp->Packet.transmitted = true;
												//addAtBackWindow(read);
												addAtBackTransmitted(
														temp->Packet);

											}
										temp = temp->link;
									} //end while
								}
							}
						}
					}

				} /*else if (ack.sequenceNumber == -1
						|| ack.sequenceNumber != expectedSequenceNumber) {

					sendto(clientCommunicationSocket, &read, sizeof(read),
					MSG_WAITALL, (struct sockaddr *) &clientAddress,
							structSize);//msg wait all problem
				}	*/				//end else if
				cout<<"Before Remove"<<endl;
				read = removeFromFront();
				cout<<"After Remove"<<read.sequenceNumber<<endl;
			}					//end while
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
				//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//


//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>//

		//	}//end for loop

		}//end if
		else {
			cout << "Here" << endl;
			exit(0);	//never exit or program will work only once
		}	//end else
	}	// end while
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

////////////////////////////////////////////////WRITING////////////////////////////////////////////////

}	//end main

char * readFile(string fileName, streampos* pos) {
	//int x;

	ifstream infile;
	infile.open(fileName.c_str(), ios::binary | ios::in | ios::ate);
	if (infile.is_open()) {
		cout << "OPENED" << endl;
	} else
		cout << "NOT" << endl;
	// infile.seekp(243, ios::beg); // move 243 bytes into the file
	//infile.read(&x, sizeof(x));
	*pos = infile.tellg();
	cout << *pos << endl;
	char *buffer = new char[*pos];
	infile.seekg(0, ios::beg);	// ptr at start
	infile.read(buffer, *pos);
	//cout << "BUFF:" << buffer << endl;
	infile.close();
	return buffer;
}	//end function
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
bool writeFile(char* buffer, streampos pos) {
	ofstream myfile;
	myfile.open("hamza.pdf", ios::binary | ios::out);
	myfile.write(buffer, pos);
	myfile.close();
	return true;
}	//end function

///////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
uint16_t udp_checksum(const void *buff, size_t len)		//		See Reference
		{
	const uint16_t *buf = (uint16_t *) buff;

	//      uint16_t *ip_src=(void *)&src_addr, *ip_dst=(void *)&dest_addr;

	uint32_t sum;
	//size_t length = len;

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
////////////////////////////////////////////////////////
///////////////////////////////////////////////////////
int serverConfiguration() {
	std::ifstream file("serverConfiguration.txt");
	std::string str;
	int flag = 0;
	std::string tag;

	while (std::getline(file, str)) {
		stringstream ss(str);
		string token;

		while (getline(ss, token, ' ')) {
			if (flag == 0) {
				tag = token;
				flag = 1;
			} else if (flag == 1) {
				flag = 0;
				if (tag == "Server_port:")
					Server_port = token;

				else if (tag == "File_name:")
					File_name = token;

				else if (tag == "Packet_size:")
					Packet_size = token;

				else if (tag == "Packet_number_to_drop:")
					Packet_number_to_drop = token;

				else if (tag == "Packet_number_to_corrupt:")
					Packet_number_to_corrupt = token;
			}
		}
	}

	cout << "\nServer_port " << Server_port << "\nFile_name " << File_name
			<< "\nPacket_size " << Packet_size << "\nPacket_number_to_drop "
			<< Packet_number_to_drop << "\nPacket_number_to_corrupt "
			<< Packet_number_to_corrupt << endl;

	return 0;
}
////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////


struct Packet
{
    int seq_num, ACK;
    bool transmitted, recivedACK;
};
Packet packets[10];
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool windowPacketsTransmitted()
{
    int transmissionFlag = 0;
    WindowPackets *temp=headW;
    while (temp!=NULL ) //for all window packets
    {
        if ( temp->windowNumber >=send_base && temp->windowNumber >=max_base)
        {
            transmissionFlag += 1;
        }
        temp=temp->link;
    }
    if ( transmissionFlag == (max_base - send_base) )
    {
        return true; //all packets in window are transmitted
    }
    return false; //one or more packets in window not transmitted
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void transmitPackets()
{
	 WindowPackets *temp=headW;
	    while (temp!=NULL ) //for all window packets
	    {
	    if ( temp->windowNumber >=send_base && temp->windowNumber >=max_base)
        if ( temp->Packet.transmitted== false)
        {
            //transmit packet. work here
            temp->Packet.transmitted = true;
        }
        temp=temp->link;
	  }//end while

}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void makePackets(int noOfPackets, int dataSize, streampos pos,
		char* buffer) {
	int counter = 0;
	packet read;
	for (int k = 0; k <noOfPackets; k++) {
		for (int j = 0; j < dataSize; j++) {
			if (counter == pos) {
				break;
			}
			read.buffer[j] = buffer[counter++];
			read.noOfPackets = noOfPackets;
			read.dataSize = dataSize;
			read.pos = pos;
			read.sequenceNumber = k; //Either add +1 here or on Client Side
			read.checksum = udp_checksum(read.buffer,
					sizeof(read.buffer));
		} //j
		addAtBackList(read);
	} //k

}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>


////////////////////////////////////////////////////////////////////////////////
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void addAtBackList(packet p) {
	notTransmittedPackets *temp = new notTransmittedPackets;
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
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void addAtBackTransmitted(packet p) {
	TransmittedPackets *temp = new TransmittedPackets;
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
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static int WindowNumber=0;
void addAtBackWindow(packet p) {
	WindowPackets *temp = new WindowPackets;
	temp->Packet = p;
	temp->windowNumber=WindowNumber++;

	if (headW == NULL && tailW == NULL) // first element, link list is empty
	{
		headW = temp; // our head and tail will be pointing to the same Node.
		tailW = temp; // head=tail=temp;
	} else {
		tailW->link = temp; // first, chain the new node to the tail of existing list
		tailW = temp; // update the tail information,
		tailW->link = NULL;
	}
}
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
