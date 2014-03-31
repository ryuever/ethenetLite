#ifndef E_NETWORK_H
#define E_NETWORK_H

#define ENL_UDP_SOCK_BUFF_SIZE 8192

int enl_udp_socket();
int enl_udp_broadcast_socket();
int enl_udp_bind(int sock_fd, int ip, int port);
int enl_udp_eport_bind(int sock_fd, int ip, int* port);
int enl_udp_multicast_bind(int sock_fd, int server_ip, int server_port, int mcast_ip);
int enl_udp_connect(int sock_fd, int ip, int port);
int enl_udp_send(int sock_fd, void* data, int data_size);
int enl_udp_recv(int sock_fd, unsigned char* data, int data_size);
int enl_udp_sendto(int sock_fd, int ip, int port, void* data, int data_size);
int enl_udp_recvfrom(int sock_fd, int* ip, int* port, unsigned char* data, int data_size);
int enl_udp_close(int sock_fd);

#endif  /* E_NETWORK_H */