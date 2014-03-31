#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "enl_network.h"

#ifndef SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

int enl_udp_socket()
{
	int ret = 0;
	int sock_fd = 0;
	int option = 0;

	sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sock_fd < 0)
	{
		return -1;
	}

	option = 1;
	ret = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, (char*)&option, sizeof(option));
	if(ret < 0)
	{
		return enl_udp_close(sock_fd);
	}

	option = ENL_UDP_SOCK_BUFF_SIZE;
	ret = setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (char*)&option, sizeof(option));
	if(ret < 0)
	{
		return enl_udp_close(sock_fd);
	}

	ret = setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (char*)&option, sizeof(option));
	if(ret < 0)
	{
		return enl_udp_close(sock_fd);
	}

	return sock_fd;
}

int enl_udp_broadcast_socket()
{
	int ret = 0;
	int sock_fd = 0;
	int option = 0;

	sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
	if(sock_fd < 0)
	{
		return -1;
	}

	option = 1;
	ret = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, (char*)&option, sizeof(option));
	if(ret < 0)
	{
		return enl_udp_close(sock_fd);
	}

	option = ENL_UDP_SOCK_BUFF_SIZE;
	ret = setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, (char*)&option, sizeof(option));
	if(ret < 0)
	{
		return enl_udp_close(sock_fd);
	}

	ret = setsockopt(sock_fd, SOL_SOCKET, SO_RCVBUF, (char*)&option, sizeof(option));
	if(ret < 0)
	{
		return enl_udp_close(sock_fd);
	}

	ret = setsockopt(sock_fd, SOL_SOCKET, SO_BROADCAST, (char*)&option, sizeof(option));
	if(ret < 0)
	{
		return enl_udp_close(sock_fd);
	}

	return sock_fd;
}

int enl_udp_bind(int sock_fd, int ip, int port)
{
	int ret = 0;
	struct sockaddr_in sock_addr;
	memset(&sock_addr, 0, sizeof(sock_addr));

	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_addr.sin_port = htons(port);

	ret = bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
	if(ret < 0)
	{
		return -1;
	}

	return ret;
}

int enl_udp_eport_bind(int sock_fd, int ip, int* port)
{
	int ret = 0;
	struct sockaddr_in sock_addr;
	struct sockaddr_in act_sock_addr;
	unsigned int sock_addr_len = sizeof(act_sock_addr);

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_addr.sin_port = htons(0);

	ret = bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
	if(ret < 0)
	{
		return -1;
	}

	ret = getsockname(sock_fd, (struct sockaddr*)&act_sock_addr, &sock_addr_len);
	if(ret < 0)
	{
		return -1;
	}

	*port = ntohs(act_sock_addr.sin_port);

	return ret;
}

int enl_udp_multicast_bind(int sock_fd, int server_ip, int server_port, int mcast_ip)
{
	int ret = 0;
	struct ip_mreq mreq;
	struct sockaddr_in sock_addr;

	if(!IN_MULTICAST(ntohl(mcast_ip)))
	{
		return -1;
	}

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sock_addr.sin_port = htons(server_port);

	ret = bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
	if(ret < 0)
	{
		return -1;
	}

	mreq.imr_multiaddr.s_addr = mcast_ip;
	mreq.imr_interface.s_addr = htonl(INADDR_ANY);

	ret = setsockopt(sock_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*)&mreq, sizeof(mreq));
	if(ret < 0)
	{
		return -1;
	}

	return ret;
}

int enl_udp_connect(int sock_fd, int ip, int port)
{
	int ret = 0;
	struct sockaddr_in sock_addr;

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = ip;
	sock_addr.sin_port = htons(port);

	ret = connect(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
	if(ret < 0)
	{
		return -1;
	}

	return ret;
}

int enl_udp_send(int sock_fd, void* data, int data_size)
{
	int ret = 0;

	ret = send(sock_fd, data, data_size, 0);
	if(ret < 0)
	{
		return -1;
	}

	return ret;
}

int enl_udp_recv(int sock_fd, unsigned char* data, int data_size)
{
	int ret = 0;

	ret = recv(sock_fd, data, data_size, 0);
	if(ret < 0)
	{
		return -1;
	}

	return ret;
}

int enl_udp_sendto(int sock_fd, int ip, int port, void* data, int data_size)
{
	int ret = 0;
	struct sockaddr_in sock_addr;

	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_addr.s_addr = ip;
	sock_addr.sin_port = htons(port);

	ret = sendto(sock_fd, data, data_size, 0, (struct sockaddr*)&sock_addr, sizeof(sock_addr));
	if(ret < 0)
	{
		return -1;
	}

	return ret;
}

int enl_udp_recvfrom(int sock_fd, int* ip, int* port, unsigned char* data, int data_size)
{
	int ret = 0;
	struct sockaddr_in sock_addr;
	unsigned int sock_addr_len = sizeof(sock_addr);
	memset(&sock_addr, 0, sock_addr_len);

	ret = recvfrom(sock_fd, data, data_size, 0, (struct sockaddr*)&sock_addr, &sock_addr_len);
	if(ret < 0)
	{
		return -1;
	}

	*ip = sock_addr.sin_addr.s_addr;
	*port = ntohs(sock_addr.sin_port);

	return ret;
}

int enl_udp_close(int sock_fd)
{
	int ret = 0;

	ret = close(sock_fd);
	if(ret < 0)
	{
		return -1;
	}

	return ret;
}
