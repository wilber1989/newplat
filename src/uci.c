#include <string.h>
#include <stdlib.h>

#include <uci2.h>
#include "uci.h"

static struct uci_ptr ptr;

static inline int ucix_get_ptr(struct uci_context *ctx, const char *p, const char *s, const char *o, const char *t)
{
	memset(&ptr, 0, sizeof(ptr));
	ptr.package = p;
	ptr.section = s;
	ptr.option = o;
	ptr.value = t;
	return uci_lookup_ptr(ctx, &ptr, NULL, true);
}

struct uci_context *ucix_init(const char *savedir)
{
	struct uci_context *ctx = uci_alloc_context();
	if (ctx == NULL)
		return NULL;

	if (savedir)
		uci_set_savedir(ctx, savedir);
	return ctx;
}

struct uci_context *ucix_load_config(struct uci_context *ctx, const char *config, struct uci_package **p)
{
	if(config != NULL && uci_load(ctx, config, p) != UCI_OK)
		return NULL;
	return ctx;
}

void ucix_cleanup(struct uci_context *ctx)
{
	if (ctx)
		uci_free_context(ctx);
}

void ucix_save(struct uci_context *ctx, const char *p)
{
	if(ucix_get_ptr(ctx, p, NULL, NULL, NULL) == UCI_OK)
		uci_save(ctx, ptr.p);
}

void ucix_save_state(struct uci_context *ctx, struct uci_package *package, const char *p)
{
	uci_set_savedir(ctx, "/var/state");
	if (package) {
		uci_save(ctx, package);
		return;
	}
	if (ucix_get_ptr(ctx, p, NULL, NULL, NULL) == UCI_OK)
		uci_save(ctx, ptr.p);
}

const char *ucix_get_option(struct uci_context *ctx, const char *p, const char *s, const char *o)
{
	struct uci_element *e = NULL;
	const char *value = NULL;
	if(ucix_get_ptr(ctx, p, s, o, NULL) != UCI_OK)
		return NULL;
	if (!(ptr.flags & UCI_LOOKUP_COMPLETE))
		return NULL;
	e = ptr.last;
	switch (e->type)
	{
	case UCI_TYPE_SECTION:
		value = uci_to_section(e)->type;
		break;
	case UCI_TYPE_OPTION:
		switch(ptr.o->type) {
			case UCI_TYPE_STRING:
				value = ptr.o->v.string;
				break;
			default:
				value = NULL;
				break;
		}
		break;
	default:
		return 0;
	}

	return value;
}

int ucix_get_option_int(struct uci_context *ctx, const char *p, const char *s, const char *o, int def)
{
	const char *tmp = ucix_get_option(ctx, p, s, o);
	int ret = def;

	if (tmp)
		ret = atoi(tmp);
	return ret;
}

void ucix_add_section(struct uci_context *ctx, const char *p, const char *s, const char *t)
{
	if(ucix_get_ptr(ctx, p, s, NULL, t) != UCI_OK)
		return;
	uci_set(ctx, &ptr);
}

void ucix_add_option(struct uci_context *ctx, const char *p, const char *s, const char *o, const char *t)
{
	if(ucix_get_ptr(ctx, p, s, o, (t)?(t):("")) != UCI_OK)
		return;
	uci_set(ctx, &ptr);
}

void ucix_add_option_int(struct uci_context *ctx, const char *p, const char *s, const char *o, int t)
{
	char tmp[64];
	snprintf(tmp, 64, "%d", t);
	ucix_add_option(ctx, p, s, o, tmp);
}

void ucix_set(struct uci_context *ctx, const char *p, const char *s, const char *o, const char *v)
{
	if(ucix_get_ptr(ctx, p, s, o, v) == UCI_OK)
		uci_set(ctx, &ptr);
}

void ucix_set_int(struct uci_context *ctx, const char *p, const char *s, const char *o, int v)
{
	char tmp[12] = {0};
	snprintf(tmp, 12, "%d", v);
	ucix_set(ctx, p, s, o, tmp);
}

void ucix_set_state(struct uci_context *ctx, const char *p, const char *s, const char *o, const char *v)
{
	if(ucix_get_ptr(ctx, p, s, o, v) == UCI_OK) {
		uci_set(ctx, &ptr);
		ucix_save_state(ctx, ptr.p, NULL);
	}
}

void ucix_set_state_int(struct uci_context *ctx, const char *p, const char *s, const char *o, int v)
{
	char tmp[12] = {0};
	snprintf(tmp, 12, "%d", v);
	ucix_set_state(ctx, p, s, o, tmp);
}

void ucix_set_state_ex(struct uci_context *ctx, const char *p, const char *s, const char *o, const char *v)
{
	ucix_revert(ctx, p, s, o);
	ucix_set_state(ctx, p, s, o, v);
}

void ucix_set_state_int_ex(struct uci_context *ctx, const char *p, const char *s, const char *o, int v)
{
	char tmp[12] = {0};
	snprintf(tmp, 12, "%d", v);
	ucix_set_state_ex(ctx, p, s, o, tmp);
}

void ucix_delete(struct uci_context *ctx, const char *p, const char *s, const char *o)
{
	if(ucix_get_ptr(ctx, p, s, o, NULL) == UCI_OK)
		uci_delete(ctx, &ptr);
}

void ucix_revert(struct uci_context *ctx, const char *p, const char *s, const char *o)
{
	if(ucix_get_ptr(ctx, p, s, o, NULL) == UCI_OK) {
		uci_revert(ctx, &ptr);
		ucix_save_state(ctx, ptr.p, NULL);
	}
}

int ucix_commit(struct uci_context *ctx, const char *p)
{
	if(ucix_get_ptr(ctx, p, NULL, NULL, NULL) == UCI_OK)
		return uci_commit(ctx, &ptr.p, false);
	return -1;
}

int ucix_unload(struct uci_context *ctx, const char *p)
{
	if(ucix_get_ptr(ctx, p, NULL, NULL, NULL) == UCI_OK)
		return uci_unload(ctx, ptr.p);
	return -1;
}

