/*
 * main.c
 *
 *  Created on: Oct 18, 2018
 *      Author: majian
 */
#include "newplat.h"

int g_debug = 0;
struct uci_context *ctx = NULL;

static int conf_uci_init(void)
{
	ctx = ucix_init(NULL);	//uci配置文件默认保存在/etc/config/下面
	if (!ctx) {
		LOGE("Uci init error!!!\n");
		return -1;
	}

	if (!ucix_load_config(ctx, "newplat", NULL)) {
		LOGE("Failed to load /etc/config/newplat !!!\n");
		return -1;
	}

	return 0;
}

int main(int argc , char *argv[])
{
	int opt;

	while ((opt = getopt(argc, argv, "d")) != -1) {
		switch (opt) {                                                               
			case 'd':
				g_debug = 1;
				break;
		} 
	}
	openlog("newplat", LOG_PID, LOG_DAEMON);
	uloop_init();
	//1、ubus初始化
	set_ubus_appname("newplat");
	serv_ubus_init();
	//2、uci配置初始化
	if (conf_uci_init() < 0) {
		LOGE("init uci error!");
		exit(1);
	}
	//4、mqtt初始化
	mqtt_init();
	//mqtt应用初始化
	if (mqtt_beat_init() < 0) {
		LOGE("init mqtt app error!");
		exit(1);
	}

	uloop_run();
	uloop_done();
	closelog();
	return 0;
}
