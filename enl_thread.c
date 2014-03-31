#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "enl_common.h"
#include "enl_thread.h"
#include "enl_network.h"
#include "enl_parser.h"
#include "enl_memory.h"

pthread_t nwthread_handle;
void* nwthread_status = NULL;
int nwthread_flag = 0;

void* nwthread_proc(void* param)
{
	int sock_fd = 0;
	int ret = 0;
	struct sockaddr_in cli_addr;
	char mcast_ip[16];
	char tmp_ip[16];
	struct in_addr mcast_in_addr;
	struct in_addr local_in_addr;
	int local_port = 3610;

	memset(mcast_ip, 0, sizeof(mcast_ip));
	strcpy(mcast_ip, "224.0.23.0");

	ret = inet_pton(AF_INET, mcast_ip, &(mcast_in_addr.s_addr));
	if(0 >= ret)
	{
		printf("\n[error]inet_pton, failed to convert multicast ip address from text to binary form\n");
		return 0;
	}

	sock_fd = enl_udp_socket();
	if(-1 == sock_fd)
	{
		printf("\n[error]enl_udp_socket, failed to create udp socket\n");
		return 0;
	}

	if(-1 == enl_udp_multicast_bind(sock_fd, 0, local_port, mcast_in_addr.s_addr))
	{
		printf("\n[error]enl_udp_multicast_bind, mip: %s, port: %d\n", inet_ntop(AF_INET, &mcast_in_addr.s_addr, tmp_ip, sizeof(tmp_ip)), local_port);
		enl_udp_close(sock_fd);
		return 0;
	}

	while(1)
	{
		int cli_ip = 0;
		int cli_port = 0;
		unsigned char recv_buff[MAX_PACKET_SIZE];
		int recv_size = enl_udp_recvfrom(sock_fd, &cli_ip, &cli_port, recv_buff, MAX_PACKET_SIZE);
		if(recv_size > 0)
		{
			printf("\n[recv]peerip:%s, peerport:%d\n", inet_ntop(AF_INET, &cli_ip, tmp_ip, sizeof(tmp_ip)), cli_port);
			unsigned char* pdata = (unsigned char*)enl_malloc(recv_size);
			memcpy(pdata, recv_buff, recv_size);
			enl_parser_recv_data(cli_ip, pdata, recv_size);
			enl_free(pdata);
		}
		else
		{
			printf("\n[error]enl_udp_recvfrom, sock_fd: %d\n", sock_fd);
			enl_udp_close(sock_fd);
			return 0;
		}

		//usleep(10);
	}
}

int enl_nwthread_create()
{
	int ret = 0;

	if(nwthread_flag != 0)
	{
		return 0;
	}

	ret = pthread_create(&nwthread_handle, NULL, nwthread_proc, NULL);
	if(ret != 0)
	{
		return -1;
	}

	nwthread_flag = 1;

	return 0;
}

void enl_nwthread_join()
{
	pthread_join(nwthread_handle, &nwthread_status);
}