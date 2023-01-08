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

	char *ip = "192.168.10.11"; // IPv4 of Server Machine
	int port = 25000;
	int e;

	int sockfd; //, new_sock;
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
	//server_addr.sin_addr.s_addr = htonl(INADDR_ANY); Something I was trying, Involving all local IPs.

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
	char *filename = "beeping_new.mp3";
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

		n = recvfrom(sockfd, buffer, SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);
		printf("\n***PACKET RECEIVED Out: '%s'\n",buffer); // testing
		packet_counter = 0;

		if (n)
		{

			while (strcmp(buffer, sent_message))
			{ // not same
				if (!(strcmp(buffer, EFLAG)))
				{ //Action on receiving User EOF Flag
					exit = 1;

					break;
				}

				number = buffer[0] - '0'; // Converted number
				number = number - 1;

				number++;

				packet[number - 1] = 1;

				for (int i = 1; i < SIZE; i++)
				{
					//bzero(SERVER_SPACE[number-1],SIZE);
					SERVER_SPACE[number - 1][i - 1] = buffer[i];
				}
				sendto(sockfd, "GotPacket\0", sizeof("GotPacket\0"), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				packet_counter++;

				bzero(buffer, SIZE);

				n = recvfrom(sockfd, buffer, SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);
				printf("\n***PACKET RECEIVED Inzz: '%s'\n",buffer); // testing
			}
			// COmment this
			if (exit != 1)
			{
				sendto(sockfd, "Claim?\0", sizeof("Claim?\0"), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				bzero(ack_message, SIZE);
				recvfrom(sockfd, ack_message, 1, 0, (struct sockaddr *)&server_addr, &addr_size);
				claim = ack_message[0] - '0';
				//printf("\n*********CLAIM RECEIVE%d\n", claim);
				// sdlfjadso
			}
		}

		if (exit == 1)
		{
			for (int i = 0; i < claim; i++)
			{	//DONE IN A HURRY TO NOT CHECK MISSING PACKETS
				fwrite(SERVER_SPACE[i], sizeof(char), SIZE-1, fp);
				//fprintf(fp, "%s", SERVER_SPACE[i]); //Testing
			}

			break;
		}

		if (packet_counter != claim)
		{

			int counter = 0;
			while (1)
			{
				strcpy(ack_message, "===!RECEIVED===\0");
				sendto(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

				for (int i = 0; i < claim; i++)
				{

					if (packet[i] == 0)
					{
						snprintf(ack_message, 16, "%d======SUS======", i + 1); // puts string into buffer
						sendto(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
						bzero(buffer, SIZE);

						n = recvfrom(sockfd, buffer, SIZE, 0, (struct sockaddr *)&server_addr, &addr_size);

						number = buffer[0] - '0'; // Converted number

						packet[number] = 1;
						for (int i = 1; i < n; i++)
						{
							SERVER_SPACE[number - 1][i] = buffer[i];
						}
						packet_counter++;
					}
				}

				sendto(sockfd, sent_message, sizeof(sent_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				ack = recvfrom(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, &addr_size);
				++counter;

				if ((!strcmp(ack_message, sent_message)) && (packet_counter == claim))
				{

					strcpy(ack_message, "====RECEIVED===\0");
					sendto(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				}
				/*	else if(counter>=3){
				strcpy(ack_message, "====RECEIVED===\0");
printf(" USE WRONG MOVE");
				sendto(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
				break;
			}*/
			}
		}
		else
		{

			strcpy(ack_message, "====RECEIVED===\0");
			sendto(sockfd, ack_message, sizeof(ack_message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
		}

		for (int i = 0; i < packet_counter; i++)
		{
			fwrite(SERVER_SPACE[i], sizeof(char), SIZE-1, fp);
			//fprintf(fp, "%s", SERVER_SPACE[i]); // TESTING
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
