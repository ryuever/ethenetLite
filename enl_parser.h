#ifndef ENL_PARSER_H
#define ENL_PARSER_H

#include "enl_common.h"

int enl_parser_recv_data(int ip, unsigned char* data, int data_size);

int enl_parse_property(unsigned char* data, unsigned int data_size, unsigned char opc, enl_frame_prop** prop);

void enl_process_recv_data(int ip,
                                     unsigned char ehd1,
                                     unsigned char ehd2,
                                     unsigned short tid,
                                     void* edata,
                                     int edata_size);

void enl_process_esv_0x60(int ip,
                                    unsigned char ehd1,
                                    unsigned char ehd2,
                                    unsigned short tid,
                                    void* edata,
                                    int edata_size);

void enl_process_esv_0x61(int ip,
                                    unsigned char ehd1,
                                    unsigned char ehd2,
                                    unsigned short tid,
                                    void* edata,
                                    int edata_size);

void enl_process_esv_0x62(int ip,
                                    unsigned char ehd1,
                                    unsigned char ehd2,
                                    unsigned short tid,
                                    void* edata,
                                    int edata_size);

void enl_process_esv_0x63(int ip,
                                    unsigned char ehd1,
                                    unsigned char ehd2,
                                    unsigned short tid,
                                    void* edata,
                                    int edata_size);

void enl_process_esv_0x6e(int ip,
                                    unsigned char ehd1,
                                    unsigned char ehd2,
                                    unsigned short tid,
                                    void* edata,
                                    int edata_size);

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
		                       unsigned char** frame);

#endif  /* ENL_PARSER_H */