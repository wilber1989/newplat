#ifndef _UCI_H_
#define _UCI_H_
#include <uci.h>

struct uci_context *ucix_init(const char *savedir);
struct uci_context *ucix_load_config(struct uci_context *ctx, const char *config, struct uci_package **p);
void ucix_cleanup(struct uci_context *ctx);
const char* ucix_get_option(struct uci_context *ctx,
	const char *p, const char *s, const char *o);
int ucix_get_option_int(struct uci_context *ctx,
	const char *p, const char *s, const char *o, int def);
void ucix_add_section(struct uci_context *ctx,
	const char *p, const char *s, const char *t);
void ucix_add_option(struct uci_context *ctx,
	const char *p, const char *s, const char *o, const char *t);
void ucix_add_option_int(struct uci_context *ctx,
	const char *p, const char *s, const char *o, int t);
void ucix_set(struct uci_context *ctx,
	const char *p, const char *s, const char *o, const char *v);
void ucix_set_state(struct uci_context *ctx,
	const char *p, const char *s, const char *o, const char *v);
void ucix_set_state_ex(struct uci_context *ctx,
	const char *p, const char *s, const char *o, const char *v);
void ucix_set_int(struct uci_context *ctx,
	const char *p, const char *s, const char *o, int v);
void ucix_set_state_int(struct uci_context *ctx,
	const char *p, const char *s, const char *o, int v);
void ucix_set_state_int_ex(struct uci_context *ctx,
	const char *p, const char *s, const char *o, int v);
void ucix_revert(struct uci_context *ctx,
	const char *p, const char *s, const char *o);
void ucix_delete(struct uci_context *ctx, const char *p,
	const char *s, const char *o);
void ucix_save(struct uci_context *ctx, const char *p);
void ucix_save_state(struct uci_context *ctx, struct uci_package *package, const char *p);
int ucix_commit(struct uci_context *ctx, const char *p);
int ucix_unload(struct uci_context *ctx, const char *p);
void ucix_save_nodeinfo(struct uci_context *ctx, int node_id_o, int iface_id_o,
     int c_type_o, float c_para_o, float h_alarm_o, float l_alarm_o);
#endif
