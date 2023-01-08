#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SIZE 500
#define WIN_SIZE 5

char* EFLAG = "=====END=====\0";

int main(){

char *ip = "192.168.10.11"; // Public IP of Server's Router ( ALso enable port forwarding )
int port = 25000; 
int e;

int sockfd;
struct sockaddr_in server_addr;
FILE *fp;
char *filename = "Revolution.mp4";

sockfd = socket(AF_INET, SOCK_DGRAM, 0);
if (sockfd < 0){
	perror("[-]Error in socket.");
	exit(1);
}

printf("[+]Suspected Server Socket as Target Selected.\n");


server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(port);
server_addr.sin_addr.s_addr = inet_addr(ip);

// FILE HANDLING
socklen_t addr_size = sizeof(server_addr);
fp = fopen(filename, "rb");
if (fp == NULL){
perror("[-]Error in Reading File.");
exit(1);
}

char data[SIZE] = {0};

// UDP Reliability
int ack;
char WIN_SPACE[WIN_SIZE][SIZE]={0};
int packet_counter=0;
char sent_message[16] = "======SENT=====\0";
char ack_message[16]={0};

//Confirming connection before starting
printf("\n>Shakehand by Client: Knock Knock");
	sendto(sockfd, "Knock Knock", sizeof("Knock Knock"), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

	ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);
printf("\n>Shakehand by Server: %s\n\n", ack_message);

// Send till END-OF-FILE reached
while(!feof(fp)){ // Overall Sending
	packet_counter = 0;

	// Reading file till 'WIN_SIZE' packets collected in Window of fixed size if data in File
	while(!feof(fp) && packet_counter<WIN_SIZE){ // A window Size

		fread(data,sizeof(char), sizeof(data)-1, fp);

		bzero(WIN_SPACE[packet_counter],SIZE);
		WIN_SPACE[packet_counter][0]=('0'+packet_counter)+1;
		for(int i=1;i<SIZE;i++)
			{
			WIN_SPACE[packet_counter][i]=data[i-1];
			}



		packet_counter++;
		bzero(data,SIZE);
	}

	// Send packets one at a time from WIN_SPACE Array 
	for(int i=0;i<packet_counter;i++){

		sendto(sockfd, WIN_SPACE[i], SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)); 


		}

	// if END-OF-File, Send End Flag
	if(feof(fp)){
		sendto(sockfd, EFLAG, strlen(EFLAG), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)); //Sending a User created EOF Flag
		
		}

	// Send "SENT" message to clear that WINDOW SPACE sent
	sendto(sockfd, sent_message, sizeof(sent_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

	
	if(feof(fp)){ ////// Exit if File read completely (non-check part added)
		break;	
		}

	// Message received from Server (Either Sequence of missing packet or ack-message
	ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);

	if(!strcmp(ack_message,"Claim?\0")){ // Send Packets count (in WIndow Phase) sent
		ack_message[0] = packet_counter+'0';

		sendto(sockfd, ack_message, 1, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
	}


	// Receiving an Acknowledgement Message
	ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);



	if(ack){
		
		// While all WIndows Packets not received by Server, Resend according to IDs specified by Server
		while(!strcmp(ack_message, "===!RECEIVED===\0")){ // While not same

			bzero(ack_message, sizeof(ack_message));
			ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);

			while(strcmp(ack_message, sent_message)){ // While not current Server'cycle's missing packets sequences sent 
				packet_counter = ack_message[0]-'0';

				sendto(sockfd, WIN_SPACE[packet_counter-1], SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
				
				bzero(ack_message, sizeof(ack_message));
				ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);
			}
			
			// Client tells server that it's done for current loop
			sendto(sockfd, sent_message, sizeof(sent_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

			// Waiting to know whether any need for next loop checking if "!RECEIVED" message received
			ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);
		}
	}


	bzero(ack_message, sizeof(ack_message));
	bzero(data, SIZE);
	for(int i=0; i<WIN_SIZE;i++){
		bzero(WIN_SPACE[i],SIZE);
	}


	
}

// Send END-FLAG as File is read completely
sendto(sockfd, EFLAG, strlen(EFLAG), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)); //Sending a User created EOF Flag

printf("[+]File data sent to the Target Socket\n");

close(sockfd);

printf("[+]File Closed\n");

return 0;
}

