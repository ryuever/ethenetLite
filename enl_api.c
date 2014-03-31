#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "enl_api.h"
#include "enl_thread.h"
#include "enl_network.h"
#include "enl_parser.h"


int enl_startup()
{
	int ret = 0;
	unsigned char edt_size = 0;
	unsigned char* edt = NULL;
	unsigned char frame_size = 0;
	unsigned char* frame = NULL;

	if(-1 == enl_nwthread_create())
	{;
		return -1;
	}

	enl_get_object_list(&edt_size, &edt);

	if(edt_size > 0)
	{
		enl_generate_frame(ENL_EHD1,
		                           ENL_EHD2_SPEC,
		                           0,
		                           SEOJ_BROADCAST,
		                           DEOJ_BROADCAST,
		                           ESV_INF,
		                           1,
		                           EPC_PROPERTY_INF,
		                           edt_size,
		                           edt,
		                           &frame_size,
		                           &frame);

		ret = enl_send_broadcast_msg(frame, frame_size);
	}

	enl_nwthread_join();

	return ret;
}

int enl_send_broadcast_msg(unsigned char* frame, unsigned int frame_size)
{
	int sock_fd = 0;
	int ret = 0;
	struct in_addr mcast_in_addr;
	struct in_addr local_in_addr;
	struct sockaddr_in mcast_addr;
	char local_ip[16];
	char mcast_ip[16];
	char tmp_ip[16];

	int server_port = ECHONETLITE_PORT;
	int local_port = 0;

	memset(mcast_ip, 0, sizeof(mcast_ip));
	strcpy(mcast_ip, ECHONETLITE_MULTCAST_IP);

	ret = inet_pton(AF_INET, mcast_ip, &(mcast_in_addr.s_addr));
	if(0 >= ret)
	{
		printf("\n[error]inet_pton, failed to convert multicast ip address from text to binary form\n");
		return -1;
	}

	sock_fd = enl_udp_socket();
	if(-1 == sock_fd)
	{
		printf("\n[error]enl_udp_socket, failed to create udp socket\n");
		return -1;
	}

	if(-1 == enl_udp_eport_bind(sock_fd, 0, &local_port))
	{
		printf("\n[error]enl_udp_eport_bind, port: %d\n", local_port);
		enl_udp_close(sock_fd);
		return -1;
	}

	ret = enl_udp_sendto(sock_fd, mcast_in_addr.s_addr, server_port, frame, frame_size);
	if(ret < 0)
	{
		printf("\n[error]enl_udp_sendto, mip: %s, port: %d\n", inet_ntop(AF_INET, &mcast_in_addr.s_addr, tmp_ip, sizeof(tmp_ip)), server_port);
	}
	else
	{
		printf("\n[ok]enl_udp_sendto, mip: %s, port: %d\n", inet_ntop(AF_INET, &mcast_in_addr.s_addr, tmp_ip, sizeof(tmp_ip)), server_port);
	}

	enl_udp_close(sock_fd);

	return 0;
}

int enl_send_response_msg(int ip, unsigned char* frame, unsigned int frame_size)
{
	int sock_fd = 0;
	int ret = 0;
	int server_port = ECHONETLITE_PORT;
	int local_port = 0;
	char tmp_ip[16];

	sock_fd = enl_udp_socket();
	if(-1 == sock_fd)
	{
		printf("\n[error]enl_udp_socket, failed to create udp socket\n");
		return 0;
	}

	if(-1 == enl_udp_eport_bind(sock_fd, 0, &local_port))
	{
		printf("\n[error]enl_udp_eport_bind, port: %d\n", local_port);
		enl_udp_close(sock_fd);
		return 0;
	}

	ret = enl_udp_sendto(sock_fd, ip, server_port, frame, frame_size);
	if(ret < 0)
	{
		printf("\n[error]enl_udp_sendto, ip: %s, port: %d\n", inet_ntop(AF_INET, &ip, tmp_ip, sizeof(tmp_ip)), server_port);
	}
	else
	{
		printf("\n[ok]enl_udp_sendto, ip: %s, port: %d\n", inet_ntop(AF_INET, &ip, tmp_ip, sizeof(tmp_ip)), server_port);
	}

	enl_udp_close(sock_fd);

	return 0;
}
