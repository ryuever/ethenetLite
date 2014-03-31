#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "enl_common.h"
#include "enl_parser.h"
#include "enl_object.h"
#include "enl_memory.h"
#include "enl_api.h"

int get_one_byte(void* data, int data_size, intptr_t pos, unsigned char* ret)
{
	unsigned char* p = 0;
	unsigned char* offset = (unsigned char*)pos;

	if(pos + 1 > data_size)
	{
		return -1;
	}

	p = (unsigned char*)((intptr_t)data + (intptr_t)offset);

	*ret = *p;

	return 0;
}

int get_two_bytes(void* data, int data_size, intptr_t pos, unsigned short* ret)
{
	unsigned char* p = 0;
	unsigned char* offset = (unsigned char*)pos;
	unsigned char* b0 = (unsigned char*)ret;
	unsigned char* b1 = (unsigned char*)ret + 1;

	if(pos + 2 > data_size)
	{
		return -1;
	}

	p = (unsigned char*)((intptr_t)data + (intptr_t)offset);

	*b0 = *p;
	*b1 = *(p + 1);

	return 0;
}

int get_three_bytes(void* data, int data_size, intptr_t pos, unsigned int* ret)
{
	unsigned char* p = 0;
	unsigned char* offset = (unsigned char*)pos;
	unsigned char* b0 = (unsigned char*)ret;
	unsigned char* b1 = (unsigned char*)ret + 1;
	unsigned char* b2 = (unsigned char*)ret + 2;
	unsigned char* b3 = (unsigned char*)ret + 3;

	if(pos + 3 > data_size)
	{
		return -1;
	}

	p = (unsigned char*)((intptr_t)data + (intptr_t)offset);

	*b0 = *p;
	*b1 = *(p + 1);
	*b2 = *(p + 2);
	*b3 = 0;

	return 0;
}

int get_multiple_bytes(void* data, int data_size, intptr_t pos, int ret_size, void* ret)
{
	unsigned char* p = 0;
	unsigned char* offset = (unsigned char*)pos;
	//unsigned char* retb = (unsigned char*)ret;

	if(ret_size < 0)
	{
		return -1;
	}

	if(pos + ret_size > data_size)
	{
		return -1;
	}

	p = (unsigned char*)((intptr_t)data + (intptr_t)offset);

	memcpy(ret, p, ret_size);

	return 0;
}

int write_bytes(void*buff, void* data, intptr_t pos, int size)
{
	unsigned char* p = 0;
	unsigned char* offset = (unsigned char*)pos;

	if(size == 0)
	{
		return 0;
	}

	p = (unsigned char*)((intptr_t)buff + (intptr_t)offset);

	memcpy(p, data, size);

	return 0;
}

int enl_parser_edata(unsigned char* edata, int edata_size, enl_frame_edata* edata_formated)
{
	unsigned int seoj = 0;
	unsigned int deoj = 0;
	unsigned char esv = 0;
	unsigned char opc = 0;

	if(-1 == get_three_bytes(edata, edata_size, EDATA_POS_SEOJ, &seoj))
	{
		//data error
		return -1;
	}

	if(-1 == get_three_bytes(edata, edata_size, EDATA_POS_DEOJ, &deoj))
	{
		//data error
		return -1;
	}

	if(-1 == get_one_byte(edata, edata_size, EDATA_POS_ESV, &esv))
	{
		//data error
		return -1;
	}

	if(-1 == get_one_byte(edata, edata_size, EDATA_POS_OPC, &opc))
	{
		//data error
		return -1;
	}

	edata_formated->seoj = seoj;
	edata_formated->deoj = deoj;
	edata_formated->esv = esv;
	edata_formated->opc = opc;

	if(opc == 0)
	{
		//data error
		return -1;//Not Completed
	}

	enl_frame_prop* epc_formated = NULL;
	unsigned char* data = edata + FRAME_POS_EPC;
	if(-1 == enl_parse_property(data, edata_size - FRAME_POS_EPC, opc, &epc_formated))
	{
		//data parsing error
		return -1;
	}

	edata_formated->prop = epc_formated;

	return 0;
}


int enl_parser_recv_data(int ip, unsigned char* data, int data_size)
{
	unsigned char ehd1 = 0;
	unsigned char ehd2 = 0;
	unsigned short tid = 0;

	if(-1 == get_one_byte(data, data_size, FRAME_POS_EHD1, &ehd1))
	{
		//data error
		return -1;
	}

	if(((ehd1 & 0xF0) != 0x10) && ((ehd1 & 0x80) != 0x80))
	{
		//unknown protocol
		return -1;
	}

	if(-1 == get_one_byte(data, data_size, FRAME_POS_EHD2, &ehd2))
	{
		//data error
		return -1;
	}

	if((ehd2 != 0x81) && (ehd2 != 0x82))
	{
      if((ehd2 & 0x80) == 0x80)
		{
			//echonet protocol unsupported
			return -1;
		}

		//unknown frame format
		return -1;
	}

	if(-1 == get_two_bytes(data, data_size, FRAME_POS_TID, &tid))
	{
		//data error
		return -1;
	}

	//arbitrary message format
	if(ehd2 == 0x82)
	{
      unsigned char* edata = data + (intptr_t)FRAME_POS_EDATA;
		//enl_free_text_callback(ip, ehd1, ehd2, tid, edata, data_size - FRAME_POS_EDATA);//
	}
	//specified message format
	else if(ehd2 == 0x81)
	{
		enl_frame_edata* edata_formated = (enl_frame_edata*)enl_malloc(sizeof(enl_frame_edata));
		unsigned char* edata = data + FRAME_POS_EDATA;
		if(-1 == enl_parser_edata(edata, data_size - FRAME_POS_EDATA, edata_formated))
		{
			//edata parsing error
			enl_free(edata_formated);
			return -1;
		}
		enl_process_recv_data(ip, ehd1, ehd2, tid, edata_formated, sizeof(enl_frame_edata));
		//enl_spec_text_callback(ip, ehd1, ehd2, tid, edata_formated, sizeof(enl_frame_edata));//
		enl_free(edata_formated);
	}

	return 0;
}


int enl_parse_property(unsigned char* data, unsigned int data_size, unsigned char opc, enl_frame_prop** prop)
{
	unsigned char opc_num = 1;
	//enl_frame_prop* phead = *prop;
	//enl_frame_prop* p = phead;
	enl_frame_prop* p = NULL;
	int offset = 0;

	while(opc_num <= opc)
	{
		unsigned char epc = 0;
		unsigned char pdc = 0;
		unsigned char* edt = NULL;
		//enl_frame_prop* p = phead;

		if(-1 == get_one_byte(data, data_size, 0 + offset, &epc))
		{
			//data error
			return -1;
		}

		if((epc > 0) && (-1 == get_one_byte(data, data_size, 1 + offset, &pdc)))
		{
			//data error
			return -1;
		}

		if(pdc > 0)
		{
			edt = (unsigned char*)enl_malloc(pdc);
		}

		if(-1 == get_multiple_bytes(data, data_size, 2 + offset, pdc, edt))
		{
			//data error
			return -1;
		}

		enl_frame_prop* prop_new = (enl_frame_prop*)enl_malloc(pdc);
		if(p == NULL)
		{
			p = prop_new;
			//p->access_rule =
			p->epc = epc;
			p->pdc = pdc;
			p->edt = edt;
			p->prev = NULL;
			p->next = NULL;
		}
		else
		{
			while(p->next != NULL)
			{
				p = p->next;
			}

			p->next = prop_new;
			p->next->epc = epc;
			p->next->pdc = pdc;
			p->next->edt = edt;
			p->next->prev = p;
			p->next->next = NULL;
		}

		offset += (pdc + 2);
		opc_num++;

	}

	*prop = p;
}

void enl_process_recv_data(int ip,
                                     unsigned char ehd1,
                                     unsigned char ehd2,
                                     unsigned short tid,
                                     void* edata,
                                     int edata_size)
{
	enl_frame_edata* pedata = (enl_frame_edata*)edata;

	switch(pedata->esv)
	{
		case ESV_W_NO_RES_REQ:
		{
			enl_process_esv_0x60(ip, ehd1, ehd2, tid, edata, edata_size);
		}
		break;
		case ESV_W_REQ:
		{
			enl_process_esv_0x61(ip, ehd1, ehd2, tid, edata, edata_size);
		}
		break;
		case ESV_R_REQ:
		{
			enl_process_esv_0x62(ip, ehd1, ehd2, tid, edata, edata_size);
		}
		break;
		case ESV_WR_REQ:
		{
			enl_process_esv_0x6e(ip, ehd1, ehd2, tid, edata, edata_size);
		}
		break;
		case ESV_INF_REQ:
		{
			enl_process_esv_0x63(ip, ehd1, ehd2, tid, edata, edata_size);
		}
		break;
		case ESV_INF_RESP_REQ:
		{
			//enl_process_esv_0x74(ip, ehd1, ehd2, tid, edata, edata_size);
		}
		break;
		default:
		break;
	}
}

void enl_process_esv_0x60(int ip,
                                    unsigned char ehd1,
                                    unsigned char ehd2,
                                    unsigned short tid,
                                    void* edata,
                                    int edata_size)
{
	enl_frame_edata* pedata = (enl_frame_edata*)edata;
	enl_frame_prop* p = pedata->prop;
	int max_prop_size = MAX_PACKET_SIZE - 12;
	int cur_frame_pos = 12;
	int cur_esv = 0;
	int prop_count = 0;
	unsigned char* buff = (unsigned char*)enl_malloc(MAX_PACKET_SIZE);

	write_bytes(buff, &ehd1, 0, 1);
	write_bytes(buff, &ehd2, 1, 1);
	write_bytes(buff, &tid, 2, 2);
	write_bytes(buff, &pedata->deoj, 4, 3);
	write_bytes(buff, &pedata->seoj, 7, 3);
	write_bytes(buff, &pedata->esv, 10, 1);//Not Completed
	write_bytes(buff, &pedata->opc, 11, 1);//Not Completed

	while(p != NULL)
	{
		int current_prop_size = p->pdc + 2;
		if(max_prop_size - 2 < 0)
		{
			//too many target properties
			cur_esv = ESV_W_NO_RES_UNRESP_REP;
			break;
		}
		else
		{
			unsigned char* edc = NULL;
			unsigned char status = 0;
			unsigned char pdc = 0;
			unsigned char access_rule;
			enl_get_property_status(pedata->deoj, p->epc, &pdc, &edc, &status, &access_rule);
			if((status == STATUS_PROP_NOT_EXISTED) || ((access_rule != RULE_SET)&&(access_rule != RULE_SETGET)))
			{
				cur_esv = ESV_W_NO_RES_UNRESP_REP;
				if(max_prop_size - current_prop_size < 0)
				{
					break;
				}
				write_bytes(buff, &p->epc, cur_frame_pos, 1);
				write_bytes(buff, &p->pdc, cur_frame_pos + 1, 1);
				write_bytes(buff, p->edt, cur_frame_pos + 2, p->pdc);
				cur_frame_pos += current_prop_size;
			}
			else
			{
				enl_set_property_status(pedata->deoj, p->epc, p->pdc, p->edt, &status);
				write_bytes(buff, &p->epc, cur_frame_pos, 1);
				write_bytes(buff, &p->pdc, cur_frame_pos + 1, 1);
				cur_frame_pos += 2;
			}
			prop_count++;
		}
		p = p->next;
		max_prop_size = MAX_PACKET_SIZE - cur_frame_pos;
	}

	if(cur_esv == ESV_W_NO_RES_UNRESP_REP)
	{
		write_bytes(buff, &cur_esv, 10, 1);
		write_bytes(buff, &prop_count, 11, 1);
		enl_send_response_msg(ip, buff, cur_frame_pos);
	}

	enl_free(buff);
}

void enl_process_esv_0x61(int ip,
                                    unsigned char ehd1,
                                    unsigned char ehd2,
                                    unsigned short tid,
                                    void* edata,
                                    int edata_size)
{
	enl_frame_edata* pedata = (enl_frame_edata*)edata;
	enl_frame_prop* p = pedata->prop;
	int max_prop_size = MAX_PACKET_SIZE - 12;
	int cur_frame_pos = 12;
	int cur_esv = ESV_W_ACK;
	int prop_count = 0;
	unsigned char* buff = (unsigned char*)enl_malloc(MAX_PACKET_SIZE);

	write_bytes(buff, &ehd1, 0, 1);
	write_bytes(buff, &ehd2, 1, 1);
	write_bytes(buff, &tid, 2, 2);
	write_bytes(buff, &pedata->deoj, 4, 3);
	write_bytes(buff, &pedata->seoj, 7, 3);
	write_bytes(buff, &pedata->esv, 10, 1);//Not Completed
	write_bytes(buff, &pedata->opc, 11, 1);//Not Completed

	while(p != NULL)
	{
		int current_prop_size = p->pdc + 2;
		if(max_prop_size - 2 < 0)
		{
			//too many target properties
			cur_esv = ESV_W_UNRESP_REP;
			break;
		}
		else
		{
			unsigned char* edc = NULL;
			unsigned char status = 0;
			unsigned char pdc = 0;
			unsigned char access_rule = 0;
			enl_get_property_status(pedata->deoj, p->epc, &pdc, &edc, &status, &access_rule);
			//if(status == STATUS_PROP_NOT_EXISTED)
			if((status == STATUS_PROP_NOT_EXISTED) || ((access_rule != RULE_SET)&&(access_rule != RULE_SETGET)))
			{
				cur_esv = ESV_W_UNRESP_REP;
				if(max_prop_size - current_prop_size < 0)
				{
					break;
				}
				write_bytes(buff, &p->epc, cur_frame_pos, 1);
				write_bytes(buff, &p->pdc, cur_frame_pos + 1, 1);
				write_bytes(buff, p->edt, cur_frame_pos + 2, p->pdc);
				cur_frame_pos += current_prop_size;
			}
			else
			{
				enl_set_property_status(pedata->deoj, p->epc, p->pdc, p->edt, &status);
				write_bytes(buff, &p->epc, cur_frame_pos, 1);
				write_bytes(buff, &p->pdc, cur_frame_pos + 1, 1);
				cur_frame_pos += 2;
			}
			prop_count++;
		}
		p = p->next;
		max_prop_size = MAX_PACKET_SIZE - cur_frame_pos;
	}

	write_bytes(buff, &cur_esv, 10, 1);
	write_bytes(buff, &prop_count, 11, 1);
	enl_send_response_msg(ip, buff, cur_frame_pos);
	enl_free(buff);
}

void enl_process_esv_0x62(int ip,
                                    unsigned char ehd1,
                                    unsigned char ehd2,
                                    unsigned short tid,
                                    void* edata,
                                    int edata_size)
{
	enl_frame_edata* pedata = (enl_frame_edata*)edata;
	enl_frame_prop* p = pedata->prop;
	int max_prop_size = MAX_PACKET_SIZE - 12;
	int cur_frame_pos = 12;
	int cur_esv = ESV_R_ACK;
	int prop_count = 0;
	int processpnum = 0;
	unsigned char* buff = (unsigned char*)enl_malloc(MAX_PACKET_SIZE);

	write_bytes(buff, &ehd1, 0, 1);
	write_bytes(buff, &ehd2, 1, 1);
	write_bytes(buff, &tid, 2, 2);
	write_bytes(buff, &pedata->deoj, 4, 3);
	write_bytes(buff, &pedata->seoj, 7, 3);
	write_bytes(buff, &pedata->esv, 10, 1);//Not Completed
	write_bytes(buff, &pedata->opc, 11, 1);//Not Completed

	while(prop_count < pedata->opc)
	{
		//int current_prop_size = p->pdc + 2;
		if(max_prop_size - 2 < 0)
		{

			//too many target properties
			cur_esv = ESV_R_UNRESP_REP;
			break;
		}
		else
		{
			unsigned char* edc = NULL;
			unsigned char status = 0;
			unsigned char pdc = 0;
			unsigned char access_rule = 0;

			enl_get_property_status(pedata->deoj, p->epc, &pdc, &edc, &status, &access_rule);

			//if(status == STATUS_PROP_NOT_EXISTED)
			if((status == STATUS_PROP_NOT_EXISTED) || ((access_rule != RULE_GET)&&(access_rule != RULE_SETGET)))
			{

				cur_esv = ESV_R_UNRESP_REP;
				if(max_prop_size - pdc - 2 < 0)
				{
					break;
				}
				write_bytes(buff, &p->epc, cur_frame_pos, 1);
				write_bytes(buff, &p->pdc, cur_frame_pos + 1, 1);
				//write_bytes(buff, p->edc, cur_frame_pos + 2, p->pdc);
				cur_frame_pos += 2;
			}
			else
			{

				write_bytes(buff, &p->epc, cur_frame_pos, 1);
				write_bytes(buff, &pdc, cur_frame_pos + 1, 1);
				write_bytes(buff, edc, cur_frame_pos + 2, pdc);
				cur_frame_pos += (pdc + 2);
			}
			prop_count++;
		}
		p = p->next;
		max_prop_size = MAX_PACKET_SIZE - cur_frame_pos;
	}

	write_bytes(buff, &cur_esv, 10, 1);
	write_bytes(buff, &prop_count, 11, 1);
	enl_send_response_msg(ip, buff, cur_frame_pos);
	enl_free(buff);
}

void enl_process_esv_0x6e(int ip,
                                    unsigned char ehd1,
                                    unsigned char ehd2,
                                    unsigned short tid,
                                    void* edata,
                                    int edata_size)
{
	enl_frame_edata* pedata = (enl_frame_edata*)edata;
	enl_frame_prop* p = pedata->prop;
	int cur_frame_pos = 13;
	int cur_esv = ESV_WR_UNRESP_REP;
	int OPCSet = 0;
	int OPCGet = 0;
	unsigned char* buff = (unsigned char*)enl_malloc(MAX_PACKET_SIZE);

	write_bytes(buff, &ehd1, 0, 1);
	write_bytes(buff, &ehd2, 1, 1);
	write_bytes(buff, &tid, 2, 2);
	write_bytes(buff, &pedata->deoj, 4, 3);
	write_bytes(buff, &pedata->seoj, 7, 3);
	write_bytes(buff, &pedata->esv, 10, 1);
	write_bytes(buff, &OPCSet, 11, 1);
	write_bytes(buff, &OPCGet, 12, 1);

	enl_send_response_msg(ip, buff, cur_frame_pos);
	enl_free(buff);
}

void enl_process_esv_0x63(int ip,
                                    unsigned char ehd1,
                                    unsigned char ehd2,
                                    unsigned short tid,
                                    void* edata,
                                    int edata_size)
{
	enl_frame_edata* pedata = (enl_frame_edata*)edata;
	enl_frame_prop* p = pedata->prop;
	int max_prop_size = MAX_PACKET_SIZE - 12;
	int cur_frame_pos = 12;
	int cur_esv = ESV_INF;
	int prop_count = 0;
	unsigned char* buff = (unsigned char*)enl_malloc(MAX_PACKET_SIZE);

	write_bytes(buff, &ehd1, 0, 1);
	write_bytes(buff, &ehd2, 1, 1);
	write_bytes(buff, &tid, 2, 2);
	write_bytes(buff, &pedata->deoj, 4, 3);
	write_bytes(buff, &pedata->seoj, 7, 3);
	write_bytes(buff, &pedata->esv, 10, 1);//Not Completed
	write_bytes(buff, &pedata->opc, 11, 1);//Not Completed

	while(p != NULL)
	{
		//int current_prop_size = p->pdc + 2;
		if(max_prop_size - 2 < 0)
		{
			//too many target properties
			cur_esv = ESV_INF_UNRESP_REP;
			break;
		}
		else
		{
			unsigned char* edc = NULL;
			unsigned char status = 0;
			unsigned char pdc = 0;
			unsigned char access_rule = 0;
			enl_get_property_status(pedata->deoj, p->epc, &pdc, &edc, &status, &access_rule);
			//if(status == STATUS_PROP_NOT_EXISTED)
			if((status == STATUS_PROP_NOT_EXISTED) || (access_rule != RULE_ANNO))
			{
				cur_esv = ESV_INF_UNRESP_REP;
				if(max_prop_size - pdc - 2 < 0)
				{
					break;
				}
				write_bytes(buff, &p->epc, cur_frame_pos, 1);
				write_bytes(buff, &p->pdc, cur_frame_pos + 1, 1);
				//write_bytes(buff, p->edc, cur_frame_pos + 2, p->pdc);
				cur_frame_pos += 2;
			}
			else
			{
				write_bytes(buff, &p->epc, cur_frame_pos, 1);
				write_bytes(buff, &pdc, cur_frame_pos + 1, 1);
				write_bytes(buff, edc, cur_frame_pos + 2, pdc);
				cur_frame_pos += (pdc + 2);
			}
			prop_count++;
		}
		p = p->next;
		max_prop_size = MAX_PACKET_SIZE - cur_frame_pos;
	}

	write_bytes(buff, &cur_esv, 10, 1);
	write_bytes(buff, &prop_count, 11, 1);
	enl_send_broadcast_msg(buff, cur_frame_pos);
	enl_free(buff);
}

int enl_generate_frame(unsigned char ehd1,
		                       unsigned char ehd2,
		                       unsigned short tid,
		                       unsigned int seoj,
		                       unsigned int deoj,
		                       unsigned char esv,
		                       unsigned char opc,
		                       unsigned char epc,
		                       unsigned char pdc,
		                       unsigned char* edt,
		                       unsigned int* frame_size,
		                       unsigned char** frame)
{
	unsigned char* buff = (unsigned char*)enl_malloc(MAX_PACKET_SIZE);
	write_bytes(buff, &ehd1, 0, 1);
	write_bytes(buff, &ehd2, 1, 1);
	write_bytes(buff, &tid, 2, 2);
	write_bytes(buff, &seoj, 4, 3);
	write_bytes(buff, &deoj, 7, 3);
	write_bytes(buff, &esv, 10, 1);
	write_bytes(buff, &opc, 11, 1);
	write_bytes(buff, &epc, 12, 1);
	write_bytes(buff, &pdc, 13, 1);
	write_bytes(buff, edt, 11, pdc);
	*frame_size = 14 + pdc;
	*frame = buff;

	return 0;
}
