#include <ngx_core.h>

static char *ngx_http_decline(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void *ngx_http_decline_create_loc_conf(ngx_conf_t *cf);