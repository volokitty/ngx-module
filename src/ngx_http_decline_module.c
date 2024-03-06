#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "ngx_http_decline_module.h"
#include "query_validator.h"
#include "cJSON.h"

typedef struct
{
  ngx_str_t declined_params_file;
  cJSON *declined_params_obj;
} ngx_http_decline_loc_conf_t;

static ngx_command_t ngx_http_decline_commands[] = {
    {ngx_string("decline"),
     NGX_HTTP_LOC_CONF | NGX_CONF_TAKE1,
     ngx_http_decline,
     NGX_HTTP_LOC_CONF_OFFSET,
     0,
     NULL},
    ngx_null_command};

static ngx_http_module_t ngx_http_decline_module_ctx = {
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    ngx_http_decline_create_loc_conf,
    NULL};

static void *
ngx_http_decline_create_loc_conf(ngx_conf_t *cf)
{
  ngx_http_decline_loc_conf_t *conf;

  conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_decline_loc_conf_t));

  if (conf == NULL)
  {
    return NGX_CONF_ERROR;
  }

  conf->declined_params_file.len = 0;
  conf->declined_params_file.data = NULL;
  conf->declined_params_obj = NULL;

  return conf;
}

ngx_module_t ngx_http_decline_module = {
    NGX_MODULE_V1,
    &ngx_http_decline_module_ctx,
    ngx_http_decline_commands,
    NGX_HTTP_MODULE,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NGX_MODULE_V1_PADDING};

static ngx_int_t
ngx_http_decline_handler(ngx_http_request_t *req)
{
  ngx_http_decline_loc_conf_t *decline_loc_conf;
  decline_loc_conf = ngx_http_get_module_loc_conf(req, ngx_http_decline_module);

  ngx_int_t status_code = get_status_code(decline_loc_conf->declined_params_obj, req->method_name, req->uri);

  if (status_code == NGX_ERROR)
  {
    status_code = NGX_HTTP_NO_CONTENT;
  }

  req->headers_out.status = status_code;

  return status_code;
}

static char *
ngx_http_decline(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
  ngx_http_core_loc_conf_t *core_loc_conf;
  ngx_str_t *value;

  core_loc_conf = ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);

  value = cf->args->elts;

  ngx_http_decline_loc_conf_t *decline_loc_conf = conf;
  decline_loc_conf->declined_params_file = value[1];

  core_loc_conf->handler = ngx_http_decline_handler;

  char *buffer = json_file_to_buffer((const char *)decline_loc_conf->declined_params_file.data);

  if (!buffer)
  {
    ngx_log_error(NGX_LOG_ERR, cf->log, 1, "invalid path: %V", &decline_loc_conf->declined_params_file);
    return NGX_CONF_ERROR;
  }

  cJSON *json_config = cJSON_Parse(buffer);

  free(buffer);

  if (!json_config)
  {
    ngx_log_error(NGX_LOG_ERR, cf->log, 1, "can't parse json config: %V", &decline_loc_conf->declined_params_file);
    return NGX_CONF_ERROR;
  }

  decline_loc_conf->declined_params_obj = json_config;

  return NGX_CONF_OK;
}
