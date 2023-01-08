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
char *filename = "beeping.mp3";

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

printf("\n>Shakehand by Client: Knock Knock");
	sendto(sockfd, "Knock Knock", sizeof("Knock Knock"), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

	ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);
printf("\n>Shakehand by Server: %s\n\n", ack_message);

while(!feof(fp)){ // Overall Sending
	packet_counter = 0;
//printf("\n************CHECK****************\n");

	while(!feof(fp) && packet_counter<WIN_SIZE){ // A window Size

		fread(data,sizeof(char), sizeof(data)-1, fp);

		bzero(WIN_SPACE[packet_counter],SIZE);
		WIN_SPACE[packet_counter][0]=('0'+packet_counter)+1;
		for(int i=1;i<SIZE;i++)
			{
			WIN_SPACE[packet_counter][i]=data[i-1];
			}
		//printf("\nDATA GOTTEN (data): ' %s '\n",data); // Testing
		//printf("\nDATA Saved (WIN_SPACE[i]): ' %s '\n",WIN_SPACE[packet_counter]); // Testing

		packet_counter++;
		bzero(data,SIZE);
	}

	for(int i=0;i<packet_counter;i++){

		sendto(sockfd, WIN_SPACE[i], SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
		printf("\nDATA SENDING: '%s'\n",WIN_SPACE[i]);

		ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);
//printf("\n************MATE****************\n");
		}
	if(feof(fp)){
		sendto(sockfd, EFLAG, strlen(EFLAG), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)); //Sending a User created EOF Flag
		
		}

	sendto(sockfd, sent_message, sizeof(sent_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
// COMMENT HERE************		
	if(feof(fp)){
		break;	
		}
	ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);

	if(!strcmp(ack_message,"Claim?\0")){
		ack_message[0] = packet_counter+'0';
		//printf("\nXXXXXXXXXX CLAIM SENDING: ' %c ' from packet_counter: %d\nXXXXXXX",ack_message[0],packet_counter);// Testing
		sendto(sockfd, ack_message, 1, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
	}

//printf("\n******************** HERE********************\n");

	//
//*************************

	ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);



	if(ack){

		while(!strcmp(ack_message, "===!RECEIVED===\0")){ // While not same

			bzero(ack_message, sizeof(ack_message));
			ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);

			while(strcmp(ack_message, sent_message)){ // While not same
				packet_counter = ack_message[0]-'0';

				sendto(sockfd, WIN_SPACE[packet_counter-1], SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr)); 
				
				bzero(ack_message, sizeof(ack_message));
				ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);
			}
			sendto(sockfd, sent_message, sizeof(sent_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
			ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);
		}
	}


	bzero(ack_message, sizeof(ack_message));
	bzero(data, SIZE);
	for(int i=0; i<WIN_SIZE;i++){
		bzero(WIN_SPACE[i],SIZE);
	}


	
}


sendto(sockfd, EFLAG, strlen(EFLAG), 0, (struct sockaddr *)&server_addr, sizeof(server_addr)); //Sending a User created EOF Flag

printf("[+]File data sent to the Target Socket\n");

close(sockfd);

printf("[+]File Closed\n");

return 0;
}

