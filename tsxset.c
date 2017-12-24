/******************************************************************************
	Tool to set Date & Time on the Yamaha TSX-B235 and similar radios.
	by Mark Street <marksmanuk@gmail.com>

	Version 1.00 December 2017
	
	gcc -O3 -o tsxset tsxset.c -l bluetooth

	$ hcitool scan
	Scanning ...
        00:1F:47:EC:0D:AB       TSX-B235 Yamaha

	$ rfcomm connect hci0 00:1F:47:EC:0D:AB
******************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

#define BTCLIENT "00:1F:47:EC:0D:AB";

uint8_t crc(unsigned char const *data, size_t len)
{
	int sum = 0;
	for (int i=0; i<len; i++)
		sum += *data++;
	return ~sum + 1;
}

unsigned int readmsg(int fd, uint8_t *buffer, unsigned int len)
{
	fd_set fdset;

	FD_ZERO(&fdset);
	FD_SET(fd, &fdset);

	struct timeval tv;
	tv.tv_sec = 5;
	tv.tv_usec = 0;
	
	int ret = select(fd+1, &fdset, NULL, NULL, &tv);

	if (ret < 0)	// Fail
	{
		printf("select() failed %d\n", ret);
		return 0;
	}

	if (!ret)		// Timeout
	{
		printf("IO timeout!\n");
		return 0;
	}

	// Data is present, ok to read from socket:
	ret = read(fd, buffer, len);
	if (ret < 0)
	{
		printf("read() error %d\n", ret);
		return 0;
	}
	else
		return ret;
}

int main(int argc, char **argv)
{
    struct sockaddr_rc addr = { 0 };
    int s, status;
	int verbose = 0;
    char dest[] = BTCLIENT;

	int args;
	while ((args = getopt(argc, argv, "a:v")) != EOF)
	{
		switch(args)
		{
			case 'v':
				verbose = 1;
				break;
			case 'a':
				if (strlen(optarg) == 17)
					memcpy(dest, optarg, 17);
				else
				{
					fprintf(stderr, "Invalid device address format (%s)\n", optarg);
					return 1;
				}
				break;	
			default:
				fprintf(stderr, "Usage:\n  txset [options]\n"\
						"    -a xx:xx:xx:xx:xx:xx BT address\n"\
						"    -v Verbose\n");
				return 1;
		}
	}

    // Allocate a socket
    s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

    // Set the connection parameters (who to connect to)
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_channel = (uint8_t) 2;
    str2ba( dest, &addr.rc_bdaddr );

    // Connect to server
	if (verbose)
		printf("Connecting to host %s\n", dest);
    status = connect(s, (struct sockaddr *)&addr, sizeof(addr));

    if (status < 0)
	{	
		perror("Bluetooth connect failed");
		return 1;
	}

	// Set socket options
	fcntl(s, F_SETFL, O_NONBLOCK);

	// Wait for 00 01 message to be received on connect:
	if (verbose)
		printf("Connected OK, waiting for message 1...\n");

	uint8_t rxbuffer[1024];
	unsigned int len = readmsg(s, rxbuffer, sizeof(rxbuffer));

	if (verbose)
	{
		printf("Received (%d):\n", len);
		for (int i=0; i<len; i++)
			printf("%02X ", rxbuffer[i]);
		printf("\n");
	}

	if (len != 13)
	{
		printf("Error, expecting 13 byte message, got %d bytes!\n", len);
		return 1;
	}

	// Send 0x20 0x01 / Set Date & Time:
	uint8_t message2[] = { 0xcc, 0xaa, 0x09, 0x20, 0x01,
		0x11,	// year 
		0x0b,	// month 
		0x0e,	// day 
		0x17,	// hh 
		0x00, 	// mm 
		0x12, 	// ss
		0x02,	// day of week
		0x00	// cksum
	};

	// Get system date & time:
	struct tm *tm_now;
	struct timeval tv;
	char buffer[36];

	do
	{
		gettimeofday(&tv, NULL);
		if (tv.tv_usec > 5000 && tv.tv_usec < 999000)
			usleep(1000);
	}
	while (tv.tv_usec > 10000);		// Align to second
	tm_now = localtime(&tv.tv_sec);
	strftime(buffer, 36, "%A %b %d %Y %H:%M:%S", tm_now);
	printf("Sending time: %s.%03ld\n", buffer, tv.tv_usec);

	message2[5] = tm_now->tm_year - 100;
	message2[6] = tm_now->tm_mon + 1;
	message2[7] = tm_now->tm_mday;
	message2[8] = tm_now->tm_hour;
	message2[9] = tm_now->tm_min;
	message2[10] = tm_now->tm_sec;
	message2[11] = tm_now->tm_wday;
	message2[12] = crc(message2+2, sizeof(message2)-3);

	if (verbose)
	{
		printf("Sending (%ld):\n", (unsigned long int) sizeof(message2)-1);
		for (int i=0; i<sizeof(message2); i++)
			printf("%02X ", message2[i]);
		printf("\n");
	}

	status = write(s, message2, sizeof(message2));

    if (status < 0)
		perror("failed");

	// Wait for reply:
	len = readmsg(s, rxbuffer, sizeof(rxbuffer));

	if (len != 9)
	{
		printf("Error, expecting 9 byte message, got %d bytes!\n", len);
		return 1;
	}

	if (verbose)
	{
		printf("Received (%d):\n", len);
		for (int i=0; i<len; i++)
			printf("%02X ", rxbuffer[i]);
		printf("\n");
	}

	// Close connection:
    close(s);
    return 0;
}

