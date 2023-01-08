#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#define SIZE 500
#define WIN_SIZE 5

char SERVER_SPACE[WIN_SIZE][SIZE] = {0};

char *EFLAG = "=====END=====\0";

int main()
{

	char *ip = "192.168.10.6"; // IPv4 of Server Machine
	int port = 25000;
	int e;

	int sockfd; 
	struct sockaddr_in server_addr, new_addr;
	socklen_t addr_size;
	char buffer[SIZE];

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
	{
		perror("[-]Error in socket.");
		exit(1);
	}
	printf("[+]Server socket created.\n");

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = inet_addr(ip);


	e = bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (sockfd < 0)
	{
		perror("[-]Error in socket.");
		exit(1);
	}
	printf("[+]Binding successful.\n");

	// FILE HANDLING

	int n;
	FILE *fp;
	char *filename = "Revolution_X3.mp4";
	addr_size = sizeof(server_addr);

	fp = fopen(filename, "wb");

	if (fp == NULL)
	{
		perror("[-]Error in Creating File.");
		exit(1);
	}

	printf("[+]Receiving Phase Start...\n");

	// UDP Reliability
	int ack; // not used yet

	int packet[WIN_SIZE] = {0};
	int packet_counter = 0;
	char sent_message[16] = "======SENT=====\0";
	char ack_message[16] = {0};
	int number;
	int exit = 0;
	int claim = WIN_SIZE;

	ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);
	printf("\n>Shakehand by Client: %s", ack_message);

	sendto(sockfd, "Who's there?", sizeof("Who's there?"), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
	printf("\n>Shakehand by Server: %s\n\n", "Who's there?");

	while (1)
	{

		if (exit == 1)
		{ // when End Flag come in last Window space
			break;
		}

		// Receives packet from Client
		n = recvfrom(sockfd, buffer, SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);

		packet_counter = 0; // For Counting Packets received in particular Window Phase

		if (n)
		{

			while (strcmp(buffer, sent_message)) //while received packet is not a "Sent Message" 
			{ // not same
				if (!(strcmp(buffer, EFLAG))) 
				{ //Exit on receiving User EOF Flag
					exit = 1;

					break;
				}

				number = buffer[0] - '0'; // Separating Sequence no. of Packet 
				number = number - 1;

				number++;

				packet[number - 1] = 1; // Setting the Sequence numbered position in a specific array as 1 (means Received)

				bzero(SERVER_SPACE[number-1],SIZE); // Saving the received Packet in the Server's Window_Space Array after clearing that space
				for (int i = 1; i < SIZE; i++)
				{

					SERVER_SPACE[number - 1][i - 1] = buffer[i];
				}

				packet_counter++;

				bzero(buffer, SIZE);

				// Receive Next Packet ( Can be acknowledgement message or Data packet)
				n = recvfrom(sockfd, buffer, SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);

			}

			// Receive total packets sent in window phase from Client if not EOF case by asking
			if (exit != 1) 
			{
				sendto(sockfd, "Claim?\0", sizeof("Claim?\0"), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				bzero(ack_message, SIZE);
				recvfrom(sockfd, ack_message, 1, 0, (struct sockaddr *)&server_addr, &addr_size);
				claim = ack_message[0] - '0';


			}
		}

		if (exit == 1) //  *** Iconic case of Not checking data in last Window Phase and just writing and exiting
		{
			for (int i = 0; i < claim; i++)
			{	
				fwrite(SERVER_SPACE[i], sizeof(char), SIZE-1, fp);

			}

			break;
		}

		// If Packet received in WIndow Phase not equal to Packets sent from Client in Window Phase, Sending missing IDs and re-receiving
		if (packet_counter != claim) 
		{


			while (1)
			{
				strcpy(ack_message, "===!RECEIVED===\0");
				sendto(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
			
				// Checking missing sequence ID marked and sending it, Receiving its relavent data
				for (int i = 0; i < claim; i++)
				{

					// On Missing Packet Sequence ID, Send ID and receive packet relevant to it
					if (packet[i] == 0)
					{
						snprintf(ack_message, 16, "%d======SUS======", i + 1); // puts string into buffer
						sendto(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
						bzero(buffer, SIZE);

						// Relevant Packet particularly received
						n = recvfrom(sockfd, buffer, SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);

						number = buffer[0] - '0'; // Converted it's ID

						packet[number] = 1; // Setting Packet ID based entry to 1 ( received)

						//Saving the Received Packet in Server's Window_Space
						for (int i = 1; i < n; i++)
						{
							SERVER_SPACE[number - 1][i] = buffer[i];
						}
						packet_counter++;
					}
				}

				// Sending Message to clear that all Packets in Window Phase are checked in current Loop Cycle
				sendto(sockfd, sent_message, sizeof(sent_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				
				// Client also acknowledges the Current Loop checking
				ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);


				if ((!strcmp(ack_message, sent_message)) && (packet_counter == claim)) //If Client acknowledges and Packet counts is same as claim now then send "RECEIVED" acknowledgement
				{

					strcpy(ack_message, "====RECEIVED===\0");
					sendto(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				}
				
			
			}
		}
		else  // IF NO PACKET MISSED, SEND RECEIVE ACKNOWLEDGEMENT
		{

			strcpy(ack_message, "====RECEIVED===\0");
			sendto(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
		}

		// Write Window's Space packets into New File Created
		for (int i = 0; i < packet_counter; i++)
		{
			fwrite(SERVER_SPACE[i], sizeof(char), SIZE-1, fp);

		}

		bzero(buffer, SIZE);
		bzero(ack_message, sizeof(ack_message));
		for (int i = 0; i < WIN_SIZE; i++)
		{
			bzero(SERVER_SPACE[i], SIZE);
		}
	}

	fclose(fp);
	printf("[+]File Data Recieved and Written.\n");
}
