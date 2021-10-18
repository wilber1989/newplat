#include <byteswap.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/socket.h>
#include <math.h>
#include "newplat.h"
#include <openssl/md5.h>

#define BYTE_SWAP16(x) bswap_16(x)
#define BYTE_SWAP32(x) bswap_32(x)
#define BYTE_SWAP64(x) bswap_64(x)
//#define PI 3.1415926535898

#if __BYTE_ORDER == __BIG_ENDIAN
int pack_be64(char *p, unsigned long long val)
{
	*((unsigned long long *)p)=val;
	return sizeof(unsigned long long);
}

int unpack_be64(char *p, unsigned long long *val)
{
	*val = *((unsigned long long *)p);
	return sizeof(unsigned long long);
}

int pack_be32(char *p, unsigned int val)
{
	*((unsigned int *)p)=val;
	return sizeof(unsigned int);
}

int unpack_be32(char *p, unsigned int *val)
{
	*val = *((unsigned int *)p);
	return sizeof(unsigned int);
}

int pack_be16(char *p, unsigned short val)
{
	*((unsigned short *)p)=val;
	return sizeof(unsigned short);
}

int unpack_be16(char *p, unsigned short *val)
{
	*val = *((unsigned short *)p);
	return sizeof(unsigned short);
}

int pack_le64(char *p, unsigned long long val)
{
	*((unsigned long long *)p)=BYTE_SWAP64(val);
	return sizeof(unsigned long long);
}

int unpack_le64(char *p, unsigned long long *val)
{
	*val = BYTE_SWAP64( *((unsigned long long *)p) );
	return sizeof(unsigned long long);
}

int pack_le32(char *p, unsigned int val)
{
	*((unsigned int *)p)=BYTE_SWAP32(val);
	return sizeof(unsigned int);
}

int unpack_le32(char *p, unsigned int *val)
{
	*val = BYTE_SWAP32( *((unsigned int *)p) );
	return sizeof(unsigned int);
}

int pack_le16(char *p, unsigned short val)
{
	*((unsigned short *)p)=BYTE_SWAP16(val);
	return sizeof(unsigned short);
}

int unpack_le16(char *p, unsigned short *val)
{
	*val = BYTE_SWAP16( *((unsigned short *)p) );
	return sizeof(unsigned short);
}
#else
int pack_be64(char *p, unsigned long long val)
{
	*((unsigned long long *)p)=BYTE_SWAP64(val);
	return sizeof(unsigned long long);
}

int unpack_be64(char *p, unsigned long long *val)
{
	*val = BYTE_SWAP64( *((unsigned long long *)p) );
	return sizeof(unsigned long long);
}

int pack_be32(char *p, unsigned int val)
{
	*((unsigned int *)p)=BYTE_SWAP32(val);
	return sizeof(unsigned int);
}

int unpack_be32(char *p, unsigned int *val)
{
	*val = BYTE_SWAP32( *((unsigned int *)p) );
	return sizeof(unsigned int);
}

int pack_be16(char *p, unsigned short val)
{
	*((unsigned short *)p)=BYTE_SWAP16(val);
	return sizeof(unsigned short);
}

int unpack_be16(char *p, unsigned short *val)
{
	*val = BYTE_SWAP16( *((unsigned short *)p) );
	return sizeof(unsigned short);
}

int pack_le64(char *p, unsigned long long val)
{
	*((unsigned long long *)p)=val;
	return sizeof(unsigned long long);
}

int unpack_le64(char *p, unsigned long long *val)
{
	*val = *((unsigned long long *)p);
	return sizeof(unsigned long long);
}

int pack_le32(char *p, unsigned int val)
{
	*((unsigned int *)p)=val;
	return sizeof(unsigned int);
}

int unpack_le32(char *p, unsigned int *val)
{
	*val = *((unsigned int *)p);
	return sizeof(unsigned int);
}

int pack_le16(char *p, unsigned short val)
{
	*((unsigned short *)p)=val;
	return sizeof(unsigned short);
}

int unpack_le16(char *p, unsigned short *val)
{
	*val = *((unsigned short *)p);
	return sizeof(unsigned short);
}
#endif

int pack_u8(char *p, unsigned char val)
{
	*p=val;
	return sizeof(unsigned char);
}

int unpack_u8(char *p, unsigned char *val)
{
	*val = *p;
	return sizeof(unsigned char);
}

int pack_str(char *p, const char *str, int len)
{
	memset(p, 0, len);
	memcpy(p, str, len);
	return len;
}

int pack_zero(char *p, char fill, int len)
{
	memset(p, fill, len);
	return len;
}

int unpack_str(char *p, char *str, int len)
{
	memset(str, 0, len);
	memcpy(str, p, len);
	return len;
}

unsigned short crc_le16(unsigned char *data, int len)
{
	unsigned short tmp, crc16, crc;
	crc16 = 0xffff;
	for (int i = 0; i < len; i++) {
		crc16 = *data ^ crc16;
		for (int j = 0; j < 8; j++) {
			tmp = crc16 & 0x0001;
			crc16 = crc16 >> 1;
			if (tmp)
				crc16 = crc16 ^ 0xa001;
		}
		data++;
	}
	crc = ((crc16 & 0x00ff) << 8) | ((crc16 & 0xff00) >> 8);

	return crc;
}

unsigned short crc_be16(unsigned char *data, int len)
{
	unsigned short tmp, crc16, crc;
	crc16 = 0xffff;
	for (int i = 0; i < len; i++) {
		crc16 = *data ^ crc16;
		for (int j = 0; j < 8; j++) {
			tmp = crc16 & 0x0001;
			crc16 = crc16 >> 1;
			if (tmp)
				crc16 = crc16 ^ 0xa001;
		}
		data++;
	}
	crc = crc16;

	return crc;
}

void hexdump(unsigned char *data, int len)
{
	printf("hexdump %d bytes:", len);
	for (int i = 0; i < len; i++) {
		printf("%02x ", data[i]);
	}
	printf("\n");
}

/********************************************************************  
 * 名称：  time_to_14utc 
 * 功能：  时间戳转14位YYYYMMDDhhmmss
 * 参数：  time
 * 返回值：None
 *******************************************************************/
void time_to_14utc(unsigned int time, char *utc)
{
	struct tm *lt;
    char nowtime[24] = {0};

	lt = localtime((time_t*)&time);
    strftime(nowtime, 24, "%Y%m%d%H%M%S", lt);
    memcpy(utc, nowtime, 14);
}

int exec_process(char *cmd, char *result, int size)
{   
	size_t ret;
	FILE *fp = NULL;

	memset(result, 0, size);
	fp = popen(cmd, "r");
	if(fp == NULL){
		LOGE("exec_process: error exec cmd: %s\n",cmd);
		return -1;
	}   
	ret = fread(result, 1, size - 1, fp);
	if(ferror(fp)){
		LOGE("exec_process: %s\n",strerror(errno));
		pclose(fp);
		return -1;
	}       
	if(ret == 0 ){
		pclose(fp);
		return -1;
	}   
	pclose(fp);
	return 0;
}

void get_dev_sn(char* sn)
{
	char cmd[128] = {0};
	char reply[16] = {0};
	int ret = -1;
	
	sprintf(cmd, "sn_mac=`cat /sys/class/net/eth0/address`;sn_mac=${sn_mac//:/};sn=${sn_mac:6:6};echo -n \"$sn\"");
	ret = exec_process(cmd, reply, sizeof(reply));

	if (ret == -1) {
		LOGE("get SN error\n");
		return;
	}

	if (sn)
		memcpy(sn, reply, strlen(reply));
}

long timestamp()
{
    struct timeval tv;
 
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

