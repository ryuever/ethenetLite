#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "enl_object.h"
#include "enl_memory.h"

enl_object_list* dev_obj_list = NULL;
enl_object_list* prof_obj_list = NULL;

int enl_add_obj_to_list(enl_object* eoj){

	enl_object* head = dev_obj_list->eoj;
	if(dev_obj_list->eoj == NULL){
		dev_obj_list->eoj = eoj;
		//head->next = NULL;
	}else{
        // the recently added enl_object will be the header, Then update the next pointer
        // of the new header and prev pointer of the old header.
		enl_object* next_obj = head;
		head = eoj;
		head->next = next_obj;
		enl_object* headnext = head->next;
		headnext->prev = head;
	}
    return 0;
}

int enl_find_obj_in_list(unsigned char class_group_code, 
                         unsigned char class_code, 
                         unsigned char instance_code){
	if((dev_obj_list == NULL) || (dev_obj_list->eoj == NULL)){
		return -1;
	}

	enl_object* head = dev_obj_list->eoj;
	enl_object* p = head;
	while((p->cls_gcode != class_group_code) &&
          (p->cls_code != class_code) &&
          (p->ins_code != instance_code)){
		p = p->next;
		if(p == NULL){
			return -1;
		}
	}

	return 0;
}

int enl_create_eoj(unsigned char obj_type, 
                   unsigned char class_group_code, 
                   unsigned char class_code, 
                   unsigned char instance_code, 
                   unsigned int* eoj_code){
	enl_object* eoj = NULL;

    // first create a list pointer, length is 8 bytes.
	if(dev_obj_list == NULL){
		dev_obj_list = enl_malloc(sizeof(enl_object_list));
		dev_obj_list->eoj = NULL;
	}

	if(0 == enl_find_obj_in_list(class_group_code, class_code, instance_code)){
		//object existed
		return 0;
	}

	eoj = (enl_object*)enl_malloc(sizeof(enl_object));
	eoj->type = obj_type;
	eoj->cls_gcode = class_group_code;
	eoj->cls_code = class_code;
	eoj->ins_code = instance_code;
	eoj->prop = NULL;
	eoj->prev = NULL;
	eoj->next = NULL;

	enl_add_obj_to_list(eoj);

	*eoj_code = enl_convert_eoj_code(class_group_code, class_code, instance_code);
    return 0;
}

unsigned int enl_convert_eoj_code(unsigned char class_group_code, 
                                  unsigned char class_code, 
                                  unsigned char instance_code){
	unsigned int ret = 0;
	unsigned int* p = &ret;

	unsigned char* b0 = (unsigned char*)p;
	unsigned char* b1 = b0 + 1;
	unsigned char* b2 = b0 + 2;
	unsigned char* b3 = b0 + 3;

	*b0 = class_group_code;
	*b1 = class_code;
	*b2 = instance_code;
    // below assignment should be 0 or 0x0 ????????????????????
	*b3 = 0;

	return ret;
}

void enl_split_eoj_code(unsigned int eoj_code, 
                        unsigned char* class_group_code, 
                        unsigned char* class_code, 
                        unsigned char* instance_code){
	unsigned int* p = &eoj_code;

	unsigned char* b0 = (unsigned char*)p;
	unsigned char* b1 = b0 + 1;
	unsigned char* b2 = b0 + 2;

	*class_group_code = *b0;
	*class_code = *b1;
	*instance_code = *b2;
}

enl_object* enl_get_object_location(unsigned int eoj_code){
	unsigned char class_group_code = 0;
	unsigned char class_code = 0;
	unsigned char instance_code = 0;

	if((dev_obj_list == NULL) || (dev_obj_list->eoj == NULL)){
		return NULL;
	}

	enl_split_eoj_code(eoj_code, &class_group_code, &class_code, &instance_code);

	enl_object* head = dev_obj_list->eoj;
	enl_object* p = head;
	while((p) &&
          (p->cls_gcode != class_group_code) &&
          (p->cls_code != class_code) &&
          (p->ins_code != instance_code)){
		p = p->next;
	}

	if(p == NULL){
		return NULL;
	}

	return p;
}

int enl_get_property_status(unsigned int eoj_code, 
                            unsigned char epc, 
                            unsigned char* pdc, 
                            unsigned char** edt, 
                            unsigned char* status, 
                            unsigned char* access_rule){
	enl_object* obj = enl_get_object_location(eoj_code);
	if((obj == NULL) || (obj->prop == NULL)){
		*status = STATUS_PROP_NOT_EXISTED;
		return -1;
	}

	enl_object_prop* head = obj->prop;
	enl_object_prop* p = head;
	while(p->epc != epc){
		p = p->next;
		if(p == NULL){
			*status = STATUS_PROP_NOT_EXISTED;
			return -1;
		}
	}

	*pdc = p->pdc;
	*edt = (unsigned char*)enl_malloc(*pdc);
	memcpy(*edt, p->edt, *pdc);
	*status = STATUS_PROP_EXISTED;
	*access_rule = p->access_rule;

	return 0;
}

int enl_set_property_status(unsigned int eoj_code, 
                            unsigned char epc, 
                            unsigned char pdc, 
                            unsigned char* edt, 
                            unsigned char* status){
	enl_object* obj = enl_get_object_location(eoj_code);
	if(obj == NULL){
		*status = STATUS_OBJ_NOT_EXISTED;
		return -1;
	}

	enl_object_prop* head = obj->prop;
	enl_object_prop* p = head;

	if(head == NULL){
		head = (enl_object_prop*)enl_malloc(sizeof(enl_object_prop));
		head->epc = epc;
		head->pdc = pdc;
		head->edt = (unsigned char*)enl_malloc(pdc);
		memcpy(head->edt, edt, pdc);
	}
	else{
		while(p->epc != epc){
			p = p->next;
			if(p == NULL){
				enl_object_prop* next_prop = head;
				head = (enl_object_prop*)enl_malloc(sizeof(enl_object_prop));
				head->epc = epc;
				head->pdc = pdc;
				head->edt = (unsigned char*)enl_malloc(pdc);
				memcpy(head->edt, edt, pdc);
				head->next = next_prop;
				head->next->prev = head;
				return 0;
			}
		}

		p->epc = epc;
		if(p->pdc != pdc){
			enl_free(p->edt);
			p->edt = (unsigned char*)enl_malloc(pdc);
		}
		memcpy(p->edt, edt, pdc);
		p->pdc = pdc;
	}

	return 0;
}

int enl_set_property(unsigned int eoj_code, 
                     unsigned char epc, 
                     unsigned char pdc, 
                     unsigned char* edt, 
                     unsigned char* status, 
                     unsigned char access_rule){
	enl_object* obj = enl_get_object_location(eoj_code);
	if(obj == NULL){
		*status = STATUS_OBJ_NOT_EXISTED;
		return -1;
	}

	enl_object_prop* head = obj->prop;
	enl_object_prop* p = head;

	if(obj->prop == NULL){
		enl_object_prop* ptemp = (enl_object_prop*)enl_malloc(sizeof(enl_object_prop));
		obj->prop = ptemp;
		head = obj->prop;
		head->access_rule = access_rule;
		head->epc = epc;
		head->pdc = pdc;
		head->edt = (unsigned char*)enl_malloc(pdc);
		memcpy(head->edt, edt, pdc);
	}
	else{
		while(p->epc != epc){
			p = p->next;
			if(p == NULL){
				enl_object_prop* next_prop = head;
				head = (enl_object_prop*)enl_malloc(sizeof(enl_object_prop));
				head->access_rule = access_rule;
				head->epc = epc;
				head->pdc = pdc;
				head->edt = (unsigned char*)enl_malloc(pdc);
				memcpy(head->edt, edt, pdc);
				head->next = next_prop;
				head->next->prev = head;
				return 0;
			}
		}

		p->epc = epc;
		if(p->pdc != pdc){
			enl_free(p->edt);
			p->edt = (unsigned char*)enl_malloc(pdc);
		}
		memcpy(p->edt, edt, pdc);
		p->pdc = pdc;
	}

	return 0;
}


int enl_get_object_num(unsigned char* edt_size){
	unsigned char obj_num = 0;
	if((dev_obj_list == NULL) || (dev_obj_list->eoj == NULL)){
		*edt_size = 0;
		return 0;
	}

	enl_object* p = dev_obj_list->eoj;
	while(p != NULL){
		if(p->type == 0){
			obj_num++;
			if(obj_num == 255){
				*edt_size = 255;
				return 0;
			}
		}
		p = p->next;
	}

	*edt_size = obj_num;

	return 0;
}

int enl_get_object_list(unsigned char* edt_size, unsigned char** edt){
	unsigned char obj_num = 0;
	unsigned char edt_size_real = 0;
	unsigned char* pedt = NULL;
	unsigned char* pos1 = NULL;
	unsigned char* pos2 = NULL;
	unsigned char* pos3 = NULL;

	enl_get_object_num(&edt_size_real);
	if(edt_size_real == 0){
		*edt_size = 0;
		return 0;
	}

	obj_num = (edt_size_real >= 84) ? 84 : edt_size_real;
	*edt_size = 1+ obj_num * 3;

	pedt = enl_malloc(*edt_size);
	*edt = pedt;
	//pedt = *edt;
	//*pedt = obj_num;
	obj_num = 0;

	enl_object* p = dev_obj_list->eoj;
	while(p != NULL){
		obj_num++;
		pos1 = (unsigned char*)(pedt + 1 + (obj_num - 1) * 3);
		pos2 = (unsigned char*)(pedt + 2 + (obj_num - 1) * 3);
		pos3 = (unsigned char*)(pedt + 3 + (obj_num - 1) * 3);
		*pos1 = p->cls_gcode;
		*pos2 = p->cls_code;
		*pos3 = p->ins_code;
		if(obj_num == 84){
			return 0;
		}

		p = p->next;

	}

	return 0;
}

int enl_get_class_num(unsigned char* edt_size, unsigned char** edt){
	return 0;
}

int enl_get_class_list(unsigned char* edt_size, unsigned char** edt){
	return 0;
}
