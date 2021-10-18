#ifndef _NEWPLAT_H__
#define _NEWPLAT_H__

#include <sys/stat.h>       
#include <fcntl.h>      /*文件控制定义*/    
#include <termios.h>    /*PPSIX 终端控制定义*/    
#include <errno.h>      /*错误号定义*/ 
#include <sys/types.h>  
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <syslog.h>
#include <pthread.h>
#include <mosquitto.h>
#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubus.h>
#include <cjson/cJSON.h>
#include <arpa/inet.h>
#include "loopbuf.h"
#include "uci.h"
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/bio.h>  
#include <openssl/buffer.h> 

/************************调试信息相关*****************************/
extern int g_debug;
//#define LOGD(...) if (g_debug) syslog(LOG_DEBUG, __VA_ARGS__)
#if 1
#define LOGD(fmt, ...) \
do {\
	if(g_debug)\
		syslog(LOG_DEBUG, fmt, ##__VA_ARGS__);\
	else\
		printf(fmt "\n", ##__VA_ARGS__);\
} while (0)
#endif
//#define LOGI(...) syslog(LOG_INFO, __VA_ARGS__)
#define LOGI(fmt, ...) \
do {\
	syslog(LOG_INFO, fmt, ##__VA_ARGS__);\
	printf(fmt "\n", ##__VA_ARGS__);\
} while (0)
#define LOGN(...) syslog(LOG_NOTICE, __VA_ARGS__)
#define LOGW(...) syslog(LOG_WARNING, __VA_ARGS__)
#define LOGE(fmt, ...) \
do {\
	syslog(LOG_ERR, fmt, ##__VA_ARGS__);\
	printf(fmt "\n", ##__VA_ARGS__);\
} while (0)

#define CMD(fmt, ...) \
do { \
	char cmd[256] = {0}; \
	char reply[16] = {0}; \
	sprintf(cmd, fmt, ##__VA_ARGS__); \
	exec_process(cmd, reply, sizeof(reply)); \
} while(0)

struct dev_data{
	unsigned char coordinate;			//坐标系类型
	int flight_time;					//飞行时长
	int lat;							//纬度
	int lng;							//经度
	int alt;							//海拔
	int height;							//高度
	char time[16];						//无人机时间:YYYYMMDDhhmmss
	int course;							//航向角
	short spd;							//速度
};

/************************MQTT相关函数**************************/
#define TOPIC_MAX_LEN 80
struct mqtt_context {
	int connected_uav;							//0未连接 1连接		
	struct mosquitto *mosq_uav;
	char *mqtt_name;							//mqtt用户名
	char *mqtt_password;						//mqtt密码
	char *mqtt_host;							//服务器地址
	int  mqtt_port;								//端口号
	int  mqtt_keepalive;						//keepalive
	char *mqtt_clientId;						//客户端id
	bool mqtt_clean_session; 
	char *mqtt_message_header;					//消息头	
	int (*uav_msg_callback)(char *data, int len);
	
	char *publish_topic_upload;					//透传串口数据主题
	char publish_topic_upload_qos;
	bool publish_topic_upload_retain;
	
	char *subscribe_topic;						//保存需要订阅的主题
	char subscribe_topic_qos; 
};

struct net_info_s {
	int pci;
	unsigned int freq;
	int rsrp;
	int rsrq;
	int snr;
	int net;
	char *net_str;
};

int mqtt_init();
void mqtt_close();
int mqtt_beat_init();
void mosquitto_message_process(const struct mosquitto_message *message); 
int do_tty_msg_publish(char *data, int len, char *topic);


/*************************cJSON*******************************/
void cjson_init();

char* get_device_id();
char* get_uasid();
char* get_product_key();
char* get_uuid();

/****************************uci******************************/
extern struct uci_context *ctx;

/****************************ubus*****************************/
#define REG_UBUS_METHOD(name, handler, policy, n_policy) \
static void __attribute((constructor)) __reg_ubus_method_##name() \
{ \
	add_ubus_method(#name, handler, policy, n_policy); \
}
void ubus_app_init();
bool serv_ubus_init();
void set_ubus_appname(const char *name);
int ubus_get_int(const char *path, const char *method, const char *param, struct blob_attr *data, int def);
char *ubus_get_string(const char *path, const char *method, const char *param, struct blob_attr *data);


/*************************数据解析相关************************/
#define E10_1  10
#define E10_2  100
#define E10_3  1000
#define E10_7  10000000


int pack_be64(char *p, unsigned long long val);
int unpack_be64(char *p, unsigned long long *val);
int pack_be32(char *p, unsigned int val);
int unpack_be32(char *p, unsigned int *val);
int pack_be16(char *p, unsigned short val);
int unpack_be16(char *p, unsigned short *val);
int pack_le64(char *p, unsigned long long val);
int unpack_le64(char *p, unsigned long long *val);
int pack_le32(char *p, unsigned int val);
int unpack_le32(char *p, unsigned int *val);
int pack_le16(char *p, unsigned short val);
int unpack_le16(char *p, unsigned short *val);
int pack_be64(char *p, unsigned long long val);
int unpack_be64(char *p, unsigned long long *val);
int pack_be32(char *p, unsigned int val);
int unpack_be32(char *p, unsigned int *val);
int pack_be16(char *p, unsigned short val);
int unpack_be16(char *p, unsigned short *val);
int pack_le64(char *p, unsigned long long val);
int unpack_le64(char *p, unsigned long long *val);
int pack_le32(char *p, unsigned int val);
int unpack_le32(char *p, unsigned int *val);
int pack_le16(char *p, unsigned short val);
int unpack_le16(char *p, unsigned short *val);
int pack_u8(char *p, unsigned char val);
int unpack_u8(char *p, unsigned char *val);
int pack_str(char *p, const char *str, int len);
int pack_zero(char *p, char fill, int len);
int unpack_str(char *p, char *str, int len);

unsigned short crc_le16(unsigned char *data, int len);
unsigned short crc_be16(unsigned char *data, int len);
void hexdump(unsigned char *data, int len);
void time_to_14utc(unsigned int time, char *utc);
long timestamp();
int exec_process(char *cmd, char *result, int size);
void get_dev_sn(char* sn);

unsigned char* AES_ECB_PKCS5_Encrypt(unsigned char *src, int srcLen, unsigned char *key, int keyLen, int *outLen, bool base64_en);
unsigned char* AES_ECB_PKCS5_Decrypt(unsigned char *src, int srcLen, unsigned char *key, int keyLen, int *outLen, bool base64_en);
#endif
