#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

// Structure for DNS header
typedef struct
{
	uint16_t id;	  // Packet Identifier
	uint16_t flags;	  // Flags (QR, OPCODE, AA, TC, RD, RA, Z, RCODE)
	uint16_t qdcount; // Question count
	uint16_t ancount; // Answer record count
	uint16_t nscount; // Authority record count
	uint16_t arcount; // Additional record count
} dns_header_t;

int main()
{
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	int udpSocket, client_addr_len;
	struct sockaddr_in clientAddress;

	udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket == -1)
	{
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting REUSE_PORT
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(udpSocket, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEPORT failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(2053),
		.sin_addr = {htonl(INADDR_ANY)},
	};

	if (bind(udpSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int bytesRead;
	char buffer[512];
	socklen_t clientAddrLen = sizeof(clientAddress);

	while (1)
	{
		// Receive data
		bytesRead = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddress, &clientAddrLen);
		if (bytesRead == -1)
		{
			perror("Error receiving data");
			break;
		}

		printf("Received %d bytes\n", bytesRead);

		// Create DNS response header
		dns_header_t header;

		// Set the ID to 1234 (same as the query)
		header.id = htons(1234);

		// Set the flags:
		// QR = 1 (response)
		// OPCODE = 0 (standard query)
		// AA = 0 (not authoritative)
		// TC = 0 (not truncated)
		// RD = 0 (recursion not desired)
		// RA = 0 (recursion not available)
		// Z = 0 (reserved bits)
		// RCODE = 0 (no error)
		header.flags = htons(0x8000); // Binary: 10000000 00000000

		// Set counts
		header.qdcount = htons(0); // No questions
		header.ancount = htons(0); // No answers
		header.nscount = htons(0); // No authority records
		header.arcount = htons(0); // No additional records

		// Send response (header only, 12 bytes)
		if (sendto(udpSocket, &header, sizeof(header), 0, (struct sockaddr *)&clientAddress, sizeof(clientAddress)) == -1)
		{
			perror("Failed to send response");
		}
		else
		{
			printf("Sent DNS response header\n");
		}
	}

	close(udpSocket);

	return 0;
}