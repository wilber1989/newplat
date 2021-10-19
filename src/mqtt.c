#include "newplat.h"

static struct mqtt_context g_mqtt_ctx;

struct mqtt_context* get_mqtt_ctx()
{
	return &g_mqtt_ctx;
}

static int mqtt_uav_recv_callback(char *recv_data, int len)
{
	return 0;
}

static void mosquitto_uav_message_process(const struct mosquitto_message *message) 
{
	if (g_mqtt_ctx.uav_msg_callback != NULL) {
		g_mqtt_ctx.uav_msg_callback((char*)message->payload, message->payloadlen);
	}
}

int do_tty_msg_publish(char *data, int len, char *topic)
{
	int ret = -1;

	if (strncmp(topic, "/paas/", strlen("/paas/")) == 0) {
		if (g_mqtt_ctx.connected_uav) {
			LOGD("publish data(len=%d) is:%s\n", len, data);
			LOGD("publish topic:%s\n", topic);
			ret = mosquitto_publish(g_mqtt_ctx.mosq_uav, NULL, topic, len, data, \
					g_mqtt_ctx.publish_topic_upload_qos, g_mqtt_ctx.publish_topic_upload_retain);
			if (ret != MOSQ_ERR_SUCCESS) {
				LOGE("mqtt send error");
				return ret;
			}
		} 
		else {
			LOGE("mosquitto connected uav = %d", g_mqtt_ctx.connected_uav);	
		}
	}
	else {
		LOGE("Unsupport mqtt topic");
	}

	return ret;
}

static void sub_mqtt_topic(struct mosquitto *mosq, char* topic)
{
	int ret = mosquitto_subscribe(mosq, NULL, topic, g_mqtt_ctx.subscribe_topic_qos);
	LOGD("mqtt sub topic: %s", topic);
	if (ret != MOSQ_ERR_SUCCESS) {
		LOGE("mqtt subscribe error:%d", ret);
	}
}

static void mqtt_connect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	if (rc) {
		exit(1);
	}

	char sub_topic[TOPIC_MAX_LEN] = {0};
	char* device_id = get_device_id();

	if (mosq == g_mqtt_ctx.mosq_uav) {
		LOGD("uav mqtt connected");
		g_mqtt_ctx.connected_uav = 1;
		if (strlen(device_id)) {
			snprintf(sub_topic, TOPIC_MAX_LEN, "/paas/%s/%s/%s", get_product_key(), get_device_id(), g_mqtt_ctx.subscribe_topic);
			sub_mqtt_topic(mosq, sub_topic);
		}
	}
}

static void mqtt_disconnect_callback(struct mosquitto *mosq, void *obj, int rc)
{
	if (mosq == g_mqtt_ctx.mosq_uav) {
		LOGD("mqtt_disconnect uav rc=%d",rc);
		g_mqtt_ctx.connected_uav = 0;
	}
}


static void mqtt_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)      
{
	if (message->payloadlen) {
		if (mosq == g_mqtt_ctx.mosq_uav) {
			mosquitto_uav_message_process(message);
		}
	} 
	else {
		LOGE("%s (null)", message->topic);
	}
}

static int read_mqtt_conf()
{
	const char *host = NULL, *port = NULL, *topic = NULL,  *sub_topic = NULL, *head = NULL,*client_id = NULL, 
	*clean_session = NULL, *qos = NULL, *keepalive = NULL, *retain_msg = NULL, *sub_qos = NULL, *manu = NULL;

	host = ucix_get_option(ctx, "newplat", "mqtt", "host");
	if (!host) {
		LOGE("read mqtt host conf err!!!");
		return -1;
	}

	port = ucix_get_option(ctx, "newplat", "mqtt", "port");
	if (!port) {
		LOGE("read mqtt port conf err!!!");
		return -1;
	}

	topic = ucix_get_option(ctx, "newplat", "mqtt", "topic");
	if (!topic) {
		LOGE("read mqtt topic conf err!!!");
		return -1;
	}

	sub_topic = ucix_get_option(ctx, "newplat", "mqtt", "sub_topic");
	if (!sub_topic) {
		LOGE("read mqtt sub_topic conf err!!!");
		return -1;
	}

	head = ucix_get_option(ctx, "newplat", "mqtt", "header");
	if (!head) {
		LOGE("read mqtt head conf err!!!");
		return -1;
	}

	client_id = ucix_get_option(ctx, "newplat", "mqtt", "client_id");

	clean_session = ucix_get_option(ctx, "newplat", "mqtt", "clean_session");
	if (!clean_session) {
		LOGE("read mqtt clean_session conf err!!!");
		return -1;
	}

	qos = ucix_get_option(ctx, "newplat", "mqtt", "qos");
	if (!qos) {
		LOGE("read mqtt qos conf err!!!");
		return -1;
	}

	keepalive = ucix_get_option(ctx, "newplat", "mqtt", "keepalive");
	if (!keepalive) {
		LOGE("read mqtt keepalive conf err!!!");
		return -1;
	}

	retain_msg = ucix_get_option(ctx, "newplat", "mqtt", "retain_msg");
	if (!retain_msg) {
		LOGE("read mqtt retain_msg conf err!!!");
		return -1;
	}

	sub_qos = ucix_get_option(ctx, "newplat", "mqtt", "sub_qos");
	if (sub_qos) {
		g_mqtt_ctx.subscribe_topic_qos = atoi(sub_qos);
	}

	manu = ucix_get_option(ctx, "newplat", "mqtt", "uav_manufacture");
	if (!manu) {
		LOGE("read uav_manufacture conf err!!!\n");
		return -1;
	}

	if (strcmp(manu, "dji") == 0) {
		g_mqtt_ctx.uav_msg_callback = mqtt_uav_recv_callback;
	}
	else {
		g_mqtt_ctx.uav_msg_callback = NULL;
	}
	
	char* sn = NULL;
	sn = get_device_id();
	g_mqtt_ctx.mqtt_name = (char *)sn;
	unsigned char* key = (unsigned char*)get_device_key();
	int keyLen = 16;
	int outLen = 0;
	g_mqtt_ctx.mqtt_password = (char*)AES_ECB_PKCS5_Encrypt((unsigned char*)sn, strlen(sn), key, keyLen, &outLen, true);
	LOGD("mqtt_password:%s\n",g_mqtt_ctx.mqtt_password);	
	g_mqtt_ctx.mqtt_host = (char *)host;
	g_mqtt_ctx.mqtt_port = atoi(port);
	g_mqtt_ctx.publish_topic_upload = (char *)topic;
	g_mqtt_ctx.subscribe_topic = (char *)sub_topic;
	g_mqtt_ctx.mqtt_message_header = (char *)head;
	g_mqtt_ctx.mqtt_clientId = (char *)client_id;
	g_mqtt_ctx.publish_topic_upload_qos = atoi(qos);
	g_mqtt_ctx.mqtt_keepalive = atoi(keepalive);

	if (strncmp(clean_session, "true", 4) == 0) {
		g_mqtt_ctx.mqtt_clean_session = true;
	} else {
		g_mqtt_ctx.mqtt_clean_session = false;
	}
	if (strncmp(retain_msg, "true", 4) == 0) {
		g_mqtt_ctx.publish_topic_upload_retain = true;
	} else {
		g_mqtt_ctx.publish_topic_upload_retain = false;
	}

	return 0;
}

void mqtt_close()
{
	if (mosquitto_disconnect(g_mqtt_ctx.mosq_uav) == MOSQ_ERR_SUCCESS)
		mosquitto_loop_stop(g_mqtt_ctx.mosq_uav, false);
	mosquitto_lib_cleanup();
	mosquitto_destroy(g_mqtt_ctx.mosq_uav);
}

int mqtt_init()
{
	if (read_mqtt_conf() < 0) {
		return -1;
	}

	mosquitto_lib_init();
	//平台mqtt设置
	g_mqtt_ctx.mosq_uav = mosquitto_new(NULL, true, NULL);
	if (!g_mqtt_ctx.mosq_uav) {
		LOGE("uav Out of memory.");
		return -1;
	}
	if (g_mqtt_ctx.mqtt_name && g_mqtt_ctx.mqtt_password){
		mosquitto_username_pw_set(g_mqtt_ctx.mosq_uav, g_mqtt_ctx.mqtt_name, g_mqtt_ctx.mqtt_password);
	}

	mosquitto_connect_callback_set(g_mqtt_ctx.mosq_uav, mqtt_connect_callback);
	mosquitto_disconnect_callback_set(g_mqtt_ctx.mosq_uav, mqtt_disconnect_callback);
	mosquitto_message_callback_set(g_mqtt_ctx.mosq_uav, mqtt_message_callback);
	mosquitto_connect_async(g_mqtt_ctx.mosq_uav, g_mqtt_ctx.mqtt_host, g_mqtt_ctx.mqtt_port, g_mqtt_ctx.mqtt_keepalive);

	//开启一个线程，在线程里不停的调用 mosquitto_loop() 来处理网络信息
	int loop_uav = mosquitto_loop_start(g_mqtt_ctx.mosq_uav);
	if (loop_uav != MOSQ_ERR_SUCCESS) {
		LOGE("uav mosquitto loop err!!");
		return -1;
	}
	return 1;
}

