#ifndef	ENL_API_H
#define	ENL_API_H

#include "enl_common.h"

/* ECHONET Lite service API */

int enl_startup();

int enl_send_broadcast_msg(unsigned char* frame, unsigned int frame_size);

int enl_send_response_msg(int ip, unsigned char* frame, unsigned int frame_size);

/* object function */
/*
int enl_add_obj_to_list(enl_object* eoj);

int enl_find_obj_in_list(unsigned char class_group_code, unsigned char class_code, unsigned char instance_code);

int enl_create_eoj(unsigned char obj_type, unsigned char class_group_code, unsigned char class_code, unsigned char instance_code, unsigned int* eoj_code);

unsigned int enl_convert_eoj_code(unsigned char class_group_code, unsigned char class_code, unsigned char instance_code);

void enl_split_eoj_code(unsigned int eoj_code, unsigned char* class_group_code, unsigned char* class_code, unsigned char* instance_code);

int enl_get_property_status(unsigned int eoj_code, unsigned char epc, unsigned char* pdc, unsigned char** edt, unsigned char* status, unsigned char* access_rule);

int enl_set_property_status(unsigned int eoj_code, unsigned char epc, unsigned char pdc, unsigned char* edt, unsigned char* status);

int enl_set_property(unsigned int eoj_code, unsigned char epc, unsigned char pdc, unsigned char* edt, unsigned char* status, unsigned char access_rule);

int enl_get_object_num(unsigned char* edt_size);

int enl_get_object_list(unsigned char* edt_size, unsigned char** edt);

int enl_get_class_num(unsigned char* edt_size, unsigned char** edt);

int enl_get_class_list(unsigned char* edt_size, unsigned char** edt);
*/
/*
int enl_property_write_no_response_req();

int enl_property_write_response_req();

int enl_property_read();

int enl_property_write_read();

int enl_property_notify();

int enl_property_notify_response_req();
*/


/*
int enl_property_read_offline();

int enl_property_write_offlinel();
*/

#endif  /* ENL_API_H */