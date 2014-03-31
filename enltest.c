#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "enl_api.h"
#include "enl_object.h"
#include "enl_common.h"

int main()
{

	unsigned int eoj_code;
	enl_create_eoj(OBJ_DEVICE, 0x02, 0x7d, 0x01, &eoj_code);

	unsigned char edt = 1;
	unsigned char status;
	enl_set_property(eoj_code, 0x80, 1, &edt, &status, RULE_SETGET);

	enl_startup();

	return 0;
}
