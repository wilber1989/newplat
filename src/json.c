#include "newplat.h"

#define PROTOCOL_VERSION "1.0"
#define DEVICE_ID_MAX 7
#define PRODUCT_KEY_MAX 20
#define DEVICE_KEY_MAX 20
#define UUID_MAX 40

static char g_device_id[DEVICE_ID_MAX] = {0};
static char g_product_key[PRODUCT_KEY_MAX] = "7060j90u98y928y98y";
static char g_device_key[DEVICE_KEY_MAX] = "hb0iojui87890oi9";
static char g_uuid[UUID_MAX] = {0};

static pthread_mutex_t g_mutex;
struct msg_queue {
	struct list_head head;
	char *msg;
	char *send_topic;
	int len;
};
static struct list_head g_msg_list = LIST_HEAD_INIT(g_msg_list);
static int g_fly_time = 0;
static struct net_info_s g_net_info;
static struct dev_data g_uav_data;

char* get_device_id()
{
	get_dev_sn(g_device_id);
	//memcpy(&g_device_id, "d72e64", 6);
	return g_device_id;
}

char* get_product_key()
{
	return g_product_key;
}

char* get_device_key()
{
	return g_device_key;
}

static void random_uuid(char* buf)
{
	uuid_t uu;
 	uuid_generate_random(uu);
	uuid_unparse(uu,buf);
}

char* get_uuid()
{
	memset(g_uuid, 0, UUID_MAX);
	random_uuid(g_uuid);
	return g_uuid;
}

static void cjson_msg_add(char *data)
{
	struct msg_queue *m = NULL;

	if (!strlen(g_device_id))
		return;
	struct mqtt_context* mqtt_ctx = NULL;
	mqtt_ctx = get_mqtt_ctx();
	char topic[TOPIC_MAX_LEN] = {0};
	snprintf(topic, TOPIC_MAX_LEN, "/paas/%s/%s/%s", get_product_key(), get_device_id(), mqtt_ctx->publish_topic_upload);

	m = (struct msg_queue *)malloc(sizeof(struct msg_queue));
	if (m == NULL) {
		LOGE("malloc error");
		return ;
	}
	m->msg = strdup(data);
	m->send_topic = strdup(topic);

	pthread_mutex_lock(&g_mutex);
	list_add_tail(&m->head, &g_msg_list);
	pthread_mutex_unlock(&g_mutex);
}

static cJSON *cjson_create_uav_data()
{
	int fly_time = 0; //飞行时间
	if(g_uav_data.lng) {
		fly_time = g_fly_time;
	}
	if (g_uav_data.flight_time) {
		fly_time = g_uav_data.flight_time;
	}
	long time = 0;//时间戳
	if(g_uav_data.time)
	{
		time = atol(g_uav_data.time);
	}
	cJSON *uav = cJSON_CreateObject();
	cJSON *value = cJSON_CreateObject();

	cJSON_AddItemToObject(uav, "type", cJSON_CreateString("property"));
	cJSON_AddItemToObject(uav, "name", cJSON_CreateString("uav"));
	cJSON_AddItemToObject(uav, "deviceCode", cJSON_CreateString(get_device_id()));

	cJSON_AddItemToObject(value, "type", cJSON_CreateString("ubox"));
	cJSON_AddItemToObject(value, "flightTime", cJSON_CreateNumber(fly_time));
	cJSON_AddItemToObject(value, "coordinate", cJSON_CreateNumber(g_uav_data.coordinate));
	cJSON_AddItemToObject(value, "longitude", cJSON_CreateNumber(g_uav_data.lng));
	cJSON_AddItemToObject(value, "latitude", cJSON_CreateNumber(g_uav_data.lat));
	cJSON_AddItemToObject(value, "height", cJSON_CreateNumber(g_uav_data.height));
	cJSON_AddItemToObject(value, "altitude", cJSON_CreateNumber(g_uav_data.alt));
	cJSON_AddItemToObject(value, "gs", cJSON_CreateNumber(g_uav_data.spd));
	cJSON_AddItemToObject(value, "course", cJSON_CreateNumber(g_uav_data.course));
	cJSON_AddItemToObject(uav, "value", value);
	cJSON_AddItemToObject(uav, "time", cJSON_CreateNumber(time));

	return uav;
}

static cJSON *cjson_create_dev_data()
{
	cJSON *dev = cJSON_CreateObject();
	cJSON *value = cJSON_CreateObject();

	cJSON_AddItemToObject(dev, "type", cJSON_CreateString("property"));
	cJSON_AddItemToObject(dev, "name", cJSON_CreateString("terminal_network"));
	cJSON_AddItemToObject(dev, "deviceCode", cJSON_CreateString(get_device_id()));

	cJSON_AddItemToObject(value, "type", cJSON_CreateString("ubox"));
	cJSON_AddItemToObject(value, "rsrp", cJSON_CreateNumber(g_net_info.rsrp));
	cJSON_AddItemToObject(value, "snr", cJSON_CreateNumber(g_net_info.snr));
	cJSON_AddItemToObject(value, "rsrq", cJSON_CreateNumber(g_net_info.rsrq));
	cJSON_AddItemToObject(value, "pci", cJSON_CreateNumber(g_net_info.pci));
	cJSON_AddItemToObject(value, "freq", cJSON_CreateNumber(g_net_info.freq));
	if (g_net_info.net == 4) 
		cJSON_AddItemToObject(value, "netMode", cJSON_CreateString("4G"));
	else if (g_net_info.net == 5)
		cJSON_AddItemToObject(value, "netMode", cJSON_CreateString("5G"));

	cJSON_AddItemToObject(dev, "value", value);
	cJSON_AddItemToObject(dev, "time", cJSON_CreateNumber(timestamp()));

	return dev;
}

static cJSON *cjson_create_params_data()
{
	cJSON *data = cJSON_CreateArray();
	cJSON *dev = NULL;
	cJSON *uav = NULL;

	dev = cjson_create_dev_data();
	uav = cjson_create_uav_data();

	cJSON_AddItemToArray(data, dev);
	cJSON_AddItemToArray(data, uav);
	
	return data;
}


static void cjson_creat_post_data()
{

	char *out = NULL;
	unsigned char* out_base64 = NULL;
	int base64_len =0;
	cJSON *root = NULL;
	cJSON *data = NULL;

	root = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "id", cJSON_CreateString(get_uuid()));
	cJSON_AddItemToObject(root, "deviceCode", cJSON_CreateString(get_device_id()));
	cJSON_AddItemToObject(root, "version", cJSON_CreateString(PROTOCOL_VERSION));
	
	data = cjson_create_params_data();
	cJSON_AddItemToObject(root, "params", data);	
	out = cJSON_Print(root);
	cJSON_Delete(root);
	unsigned char* key = (unsigned char*)get_device_key();
    out_base64 = AES_ECB_PKCS5_Encrypt((unsigned char *)out, strlen(out), key, 16, &base64_len, true);
	cjson_msg_add((char*)out_base64);
	free(out_base64);
	free(out);
}

static void mqtt_post_run(struct uloop_timeout *time)
{
	memset( &g_uav_data, 0, sizeof(g_uav_data));
	g_fly_time += 1;
	g_net_info.pci = ubus_get_int("modem", "modeminfo", "pci", NULL, 0);
	g_net_info.freq = ubus_get_int("modem", "modeminfo", "freq", NULL, 0);
	g_net_info.rsrp = ubus_get_int("modem", "modeminfo", "rsrp", NULL, 0);
	g_net_info.rsrq = ubus_get_int("modem", "modeminfo", "rsrq", NULL, 0);
	g_net_info.snr = ubus_get_int("modem", "modeminfo", "snr", NULL, 0);
	g_net_info.net = ubus_get_int("modem", "modeminfo", "net", NULL, 0);
	g_net_info.net_str = ubus_get_string("modem", "modeminfo", "net_style", NULL);
	g_uav_data.coordinate = ubus_get_int("uartd", "uavinfo", "coordinate", NULL, 0);
	g_uav_data.flight_time = ubus_get_int("uartd", "uavinfo", "flight_time", NULL, 0);
	g_uav_data.lat = atof(ubus_get_string("uartd", "uavinfo", "lat", NULL)) * E10_7 ;
	g_uav_data.lng = atof(ubus_get_string("uartd", "uavinfo", "lng", NULL)) * E10_7 ;
	g_uav_data.alt = ubus_get_int("uartd", "uavinfo", "alt", NULL, 0);
	g_uav_data.height = atof(ubus_get_string("uartd", "uavinfo", "height", NULL)) * E10_1 ;
	char* uavtime = ubus_get_string("uartd", "uavinfo", "time", NULL);
	if( uavtime ) {
		memcpy( g_uav_data.time, uavtime, strlen(uavtime));
	} 
	g_uav_data.course = ubus_get_int("uartd", "uavinfo", "cog", NULL, 0) * E10_1;
	g_uav_data.spd = atof(ubus_get_string("uartd", "uavinfo", "speed", NULL)) * E10_1 ;
	cjson_creat_post_data();
	uloop_timeout_set(time, 1000);
}
static struct uloop_timeout mqtt_post_timeout = {.cb = mqtt_post_run};

static void mqtt_send_run(struct uloop_timeout *time)
{
	struct msg_queue *m = NULL, *tmp = NULL;

	list_for_each_entry_safe(m, tmp, &g_msg_list, head) {
		if (m->msg) {
			do_tty_msg_publish(m->msg, strlen(m->msg), m->send_topic);
			free(m->msg);
			free(m->send_topic);
		}
		list_del(&(m->head));
		free(m);
	}

	uloop_timeout_set(time, 10);
}
static struct uloop_timeout mqtt_run_timeout = {.cb = mqtt_send_run};

int mqtt_beat_init()
{
	g_fly_time = ubus_get_int("system", "info", "uptime", NULL, 0);	
	uloop_timeout_set(&mqtt_post_timeout, 1000);
	uloop_timeout_set(&mqtt_run_timeout, 1000);	
	return 0;
}

