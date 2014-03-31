#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "enl_memory.h"

void* enl_malloc(int size)
{
	return malloc(size);
}

void enl_free(void* memory)
{
	free(memory);
}
