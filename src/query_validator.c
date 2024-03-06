#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include "cJSON.h"

char *
json_file_to_buffer(const char *filename)
{
  FILE *file;
  char *buffer;
  long file_size;

  file = fopen(filename, "r");
  if (!file)
  {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  rewind(file);

  buffer = (char *)malloc(file_size * sizeof(char));

  if (!buffer)
  {
    return NULL;
  }

  fread(buffer, 1, file_size, file);
  fclose(file);

  return buffer;
}

char *
ngx_str_to_c_str(ngx_str_t str)
{
  char *c_str = malloc(str.len + 1);

  if (!c_str)
  {
    return NULL;
  }

  ngx_memcpy(c_str, str.data, str.len);
  c_str[str.len] = '\0';

  return c_str;
}

u_char *
_ngx_strstr(ngx_str_t *haystack, const char *needle)
{
  size_t needle_len = strlen(needle);
  haystack->len = strlen((const char *)haystack->data);

  size_t i, j;

  if (haystack->len < needle_len)
  {
    return NULL;
  }

  for (i = 0; i <= haystack->len - needle_len; i++)
  {
    for (j = 0; j < needle_len; j++)
    {
      if (haystack->data[i + j] != needle[j])
      {
        break;
      }
    }
    if (j == needle_len)
    {
      return (u_char *)haystack->data + i;
    }
  }

  return NULL;
}

ngx_str_t
create_headers_args(ngx_http_request_t *req)
{
  ngx_list_part_t *part = &req->headers_in.headers.part;
  ngx_table_elt_t *headers = part->elts;
  ngx_uint_t num_headers = 0;

  ngx_uint_t i;
  for (i = 0;; i++)
  {
    if (i >= part->nelts)
    {
      if (!part->next)
      {
        break;
      }
      part = part->next;
      headers = part->elts;
      i = 0;
    }
    num_headers++;
  }

  ngx_str_t headers_args;
  headers_args.len = 0;
  headers_args.data = ngx_pnalloc(req->pool, req->args.len + num_headers * (sizeof(": ") - 1));
  if (!headers_args.data)
  {
    return headers_args;
  }

  u_char *p = headers_args.data;
  for (i = 0; i < num_headers; i++)
  {
    p = ngx_cpymem(p, headers[i].key.data, headers[i].key.len);
    *p++ = ':';
    *p++ = ' ';
    p = ngx_cpymem(p, headers[i].value.data, headers[i].value.len);
  }
  ngx_memcpy(p, req->args.data, req->args.len);
  headers_args.len = p - headers_args.data + req->args.len;

  return headers_args;
}

ngx_int_t
get_status_code(cJSON *declined_params_obj, ngx_http_request_t *req)
{
  char *method = ngx_str_to_c_str(req->method_name);
  cJSON *method_object = cJSON_GetObjectItem(declined_params_obj, method);
  free(method);

  if (!method_object || !cJSON_IsObject(method_object))
  {
    return NGX_ERROR;
  }

  cJSON *content_array = cJSON_GetObjectItem(method_object, "content");

  if (!content_array || !cJSON_IsArray(content_array))
  {
    return NGX_ERROR;
  }

  ngx_str_t headers_args_body = create_headers_args(req);

  if (req->request_body && req->request_body->bufs)
  {
    ngx_buf_t *buf = req->request_body->bufs->buf;
    if (buf)
    {
      headers_args_body.len += buf->last - buf->pos;
      headers_args_body.data = ngx_pnalloc(req->pool, headers_args_body.len);
      if (!headers_args_body.data)
      {
        return NGX_ERROR;
      }
      u_char *p = headers_args_body.data + headers_args_body.len - (buf->last - buf->pos);
      ngx_memcpy(p, buf->pos, buf->last - buf->pos);
    }
  }

  cJSON *param = NULL;
  cJSON_ArrayForEach(param, content_array)
  {
    if (!cJSON_IsString(param))
    {
      continue;
    }

    if (!_ngx_strstr(&headers_args_body, param->valuestring))
    {
      return NGX_ERROR;
    }
  }

  cJSON *code_object = cJSON_GetObjectItem(method_object, "code");
  if (code_object && cJSON_IsNumber(code_object))
  {
    return code_object->valueint;
  }

  return NGX_ERROR;
}
