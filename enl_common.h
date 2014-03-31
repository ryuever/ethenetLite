#ifndef	ENL_COMMON_H
#define	ENL_COMMON_H

#define MAX_PACKET_SIZE 1024

#define ENL_EHD1 0x10

#define ENL_EHD2_SPEC 0x81
#define ENL_EHD2_FREE 0x82

#define SEOJ_BROADCAST 0x0EF001
#define DEOJ_BROADCAST SEOJ_BROADCAST
#define SEOJ_BROADCAST_SPECIAL 0x0EF002

#define ESV_W_NO_RES_REQ 0x60
#define ESV_W_NO_RES_UNRESP_REP 0x50

#define ESV_W_REQ 0x61
#define ESV_W_ACK 0x71
#define ESV_W_UNRESP_REP 0x51

#define ESV_R_REQ 0x62
#define ESV_R_ACK 0x72
#define ESV_R_UNRESP_REP 0x52

#define ESV_WR_REQ 0x6E
#define ESV_WR_ACK 0x7E
#define ESV_WR_UNRESP_REP 0x5E

#define ESV_INF_REQ 0x63
#define ESV_INF 0x73
#define ESV_INF_UNRESP_REP 0x53

#define ESV_INF_RESP_REQ 0x74
#define ESV_INF_RESP 0x7A

#define EPC_PROPERTY_INF 0xD5

#define RULE_GET 0
#define RULE_SET 1
#define RULE_SETGET 2
#define RULE_ANNO 3

#define OBJ_DEVICE 0
#define OBJ_PROFILE 1

#define FRAME_POS_EHD1 0
#define FRAME_POS_EHD2 1
#define FRAME_POS_TID 2
#define FRAME_POS_EDATA 4

#define EDATA_POS_SEOJ 0
#define EDATA_POS_DEOJ 3
#define EDATA_POS_ESV 6
#define EDATA_POS_OPC 7
#define FRAME_POS_EPC 8

#define STATUS_PROP_NOT_EXISTED 1
#define STATUS_PROP_EXISTED 0

#define STATUS_OBJ_NOT_EXISTED 1


#define ECHONETLITE_PORT 3610
#define ECHONETLITE_MULTCAST_IP "224.0.23.0"

typedef struct st_enl_su_param
{
	int id;
}enl_su_param;

typedef struct st_enl_frame_prop
{
	unsigned char access_rule;
	unsigned char epc;
	unsigned char pdc;
	unsigned char* edt;
	struct st_enl_frame_prop* prev;
	struct st_enl_frame_prop* next;
}enl_frame_prop, enl_object_prop;

typedef struct st_enl_frame_edata
{
	unsigned int seoj;
	unsigned int deoj;
	unsigned char esv;
	unsigned char opc;
	struct enl_frame_prop* prop;
}enl_frame_edata;

typedef struct st_enl_frame
{
	unsigned char ehd1;
	unsigned char ehd2;
	unsigned short tid;
	enl_frame_edata edata;
}enl_frame;

typedef struct st_enl_obj
{
	unsigned char type; //object type
	unsigned char cls_gcode; //class group code
	unsigned char cls_code; //class code
	unsigned char ins_code; //instance code
	struct enl_object_prop* prop;
	struct enl_object* prev;
	struct enl_object* next;
}enl_object;

typedef struct st_enl_obj_list
{
	struct enl_object* eoj;
}enl_object_list;

#endif  /* ENL_COMMON_H */
