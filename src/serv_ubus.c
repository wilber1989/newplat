#include "newplat.h"

static char *ubus_name = NULL;
static int   ubus_timeout = 600; /* default 600 ms */
static struct ubus_context *ubus_ctx = NULL;

static struct list_head event_list  = LIST_HEAD_INIT(event_list);

struct event_item_s {
	struct list_head head;
	const  char *pattern;
	struct ubus_event_handler *ev;
};
static struct ubus_method *main_object_methods = NULL;
static int main_object_methods_size = 0;

static struct ubus_object_type *get_object_type()
{
	static struct ubus_object_type main_object_type;

	main_object_type.name = ubus_name;
	main_object_type.id   = 0;
	main_object_type.methods = main_object_methods;
	main_object_type.n_methods = main_object_methods_size;

	return &main_object_type;
}

static struct ubus_object *get_main_object()
{
	static struct ubus_object main_object;

	main_object.name = ubus_name;
	main_object.type = get_object_type();
	main_object.methods = main_object_methods;
	main_object.n_methods = main_object_methods_size;

	return &main_object;
}

static void ubus_connect_handler(struct ubus_context *ctx)
{
	ubus_ctx = ctx;
	struct event_item_s *e = NULL;

	ubus_add_uloop(ubus_ctx);
	ubus_add_object(ubus_ctx, get_main_object());

	list_for_each_entry(e, &event_list, head) {
		ubus_register_event_handler(ubus_ctx, e->ev, e->pattern);
	}
}
static struct ubus_auto_conn conn = { .cb = ubus_connect_handler };

static void recv_call_reply(struct ubus_request *req, int type, struct blob_attr *msg)
{
	struct blob_attr **ret = (struct blob_attr **)req->priv;
	if (ret != NULL) {
		*ret = blob_memdup(msg); 
	}
}

static struct blob_attr *ubus_get_option(const char *path, const char *method, const char *param, struct blob_attr *
data)
{
	int rem;
	struct blob_attr *ret = NULL;
	struct blob_attr *cur = NULL;
	struct blob_attr *result = NULL;

	if (ubus_call(path, method, data, &result)!=0 || !result) {
		goto out;
	}

	blobmsg_for_each_attr(cur, result, rem) {
		if (!strcmp(param, blobmsg_name(cur))) {
			ret = blob_memdup(cur);
			break;
		}
	}
out:
	if (result) {
		free(result);
	}
	return ret;
}

int ubus_get_int(const char *path, const char *method, const char *param, struct blob_attr *data, int def)
{
	int ret = def;
	struct blob_attr *result = NULL;

	result = ubus_get_option(path, method, param, data);
	if (!result) {
		goto out;
	}

	switch(blobmsg_type(result)) {
		case BLOBMSG_TYPE_INT8:
			ret = (int)blobmsg_get_u8(result);
			break;
		case BLOBMSG_TYPE_INT16:
			ret = (int)blobmsg_get_u16(result);
			break;
		case BLOBMSG_TYPE_INT32:
			ret = (int)blobmsg_get_u32(result);
			break;
		case BLOBMSG_TYPE_INT64:
			ret = (int)blobmsg_get_u64(result);
			break;
		default:
//			LOGE("blog msg type:%d is not integer", blobmsg_type(result));
			break;
	}
out:
	if (result) {
		free(result);
	}
	return ret;
}

char *ubus_get_string(const char *path, const char *method, const char *param, struct blob_attr *data)
{
	static char *ret = NULL;
	static char size = 128;
	char *value = NULL;
	struct blob_attr *result = NULL;

	if (ret == NULL) {
		ret = malloc(size);
	}
	memset(ret, 0, size);

	result = ubus_get_option(path, method, param, data);
	if (!result) {
		goto out;
	}
	if (blobmsg_type(result) == BLOBMSG_TYPE_STRING) {
		value = blobmsg_get_string(result);
		if (value && strlen(value)>0) {
			if (strlen(value) >= size) {
				ret = realloc(ret, strlen(value)+1);
				size = strlen(value)+1;
			}
			strncpy(ret, value, size);
		}
	}
out:
	if (result) {
		free(result);
	}
	if (strlen(ret) > 0)
		return ret;
	return NULL;
}

/***********************************************************************
* 函数作用：发送ubus广播事件
* 函数参数：id: 广播事件ID
*           data: 广播事件数据
***********************************************************************/
int ubus_send(const char *id, struct blob_attr *data)
{
	if (!ubus_ctx || !id || !data)
		return -1;
	return ubus_send_event(ubus_ctx, id, data);
}

/***********************************************************************
* 函数作用：调用ubus总线上提供的方法
* 函数参数：path: 提供该方法的模块
*           method: 具体要调用的方法
*           data: 提供给该方法的参数，可以为空
*           ret: 调用该方法的返回值
***********************************************************************/
int ubus_call(const char *path, const char *method,
		struct blob_attr *data, struct blob_attr **ret)
{
	int _ret = 0;
	uint32_t id = 0;

	if (ubus_ctx == NULL) {
		LOGE("ubus call error");
		return -1;
	}
	_ret = ubus_lookup_id(ubus_ctx, path, &id);
	if (_ret) {
//		LOGE("lookup ubus:%s error", path);
		return -1;
	}
	return ubus_invoke(ubus_ctx, id, method, data, recv_call_reply, ret, ubus_timeout);
}

/***********************************************************************
* 函数作用：响应ubus总线上对我们提供方法的调用
* 函数参数：req: 请求数据
*			msg: 响应数据
***********************************************************************/
int ubus_reply(struct ubus_request_data * req, struct blob_attr * msg)
{
	if (!ubus_ctx) {
		return -1;
	}
	return ubus_send_reply(ubus_ctx, req, msg);
}

/***********************************************************************
* 函数作用：增加一个程序对外提供的ubus方法
* 函数参数：name: 方法名
*           handler: 对应的处理函数指针
*           policy: 参数值要求
*           n_policy: 参数个数
***********************************************************************/
void add_ubus_method(const char *name, ubus_handler_t handler,
		const struct blobmsg_policy *policy, int n_policy)
{
	if (main_object_methods_size <= 0) {
		main_object_methods_size = 1;
		main_object_methods = calloc(1, sizeof(struct ubus_method)*main_object_methods_size);
	} else {
		main_object_methods_size++;
		main_object_methods = realloc(main_object_methods, sizeof(struct ubus_method)*main_object_methods_size);
	}
	main_object_methods[main_object_methods_size-1].name = name;
	main_object_methods[main_object_methods_size-1].handler = handler;
	main_object_methods[main_object_methods_size-1].mask = 0;
	main_object_methods[main_object_methods_size-1].tags = 0;
	main_object_methods[main_object_methods_size-1].policy = policy;
	main_object_methods[main_object_methods_size-1].n_policy = n_policy;	
}

/***********************************************************************
* 函数作用：向ubus总线注册关心的广播事件
* 函数参数：pattern: 事件ID
*			handler: 事件的处理函数指针
***********************************************************************/
void reg_ubus_event(const char *pattern, ubus_event_handler_t handler)
{
	struct event_item_s *event = NULL;

	event = calloc(1, sizeof(struct event_item_s));
	event->pattern = pattern;
	event->ev      = calloc(1, sizeof(struct ubus_event_handler));
	event->ev->cb  = handler;

	list_add_tail(&event->head, &event_list);
}

/***********************************************************************
* 函数作用：向ubus总线注册关心的广播事件
* 函数参数：pattern: 事件ID
*			handler: 事件的处理函数指针
***********************************************************************/
void set_ubus_appname(const char *name)
{
	ubus_name = strdup(name);
}

void set_ubus_timeout(int ms)
{
	ubus_timeout = ms;
}

bool serv_ubus_init()
{
	if (!ubus_name) {
		LOGE("not set the ubus name");
		return false;
	}
	ubus_auto_connect(&conn);

	return true;
}

