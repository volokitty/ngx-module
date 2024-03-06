#include <ngx_config.h>
#include <ngx_core.h>
#include "cJSON.h"

char *
json_file_to_buffer(const char *filename)
{
  FILE *file;
  char *buffer;
  long file_size;

  file = fopen(filename, "r");
  if (file == NULL)
  {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  file_size = ftell(file);
  rewind(file);

  buffer = (char *)malloc(file_size * sizeof(char));

  if (buffer == NULL)
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

  if (c_str == NULL)
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

ngx_int_t
get_status_code(cJSON *declined_params_obj, ngx_str_t method_name, ngx_str_t args)
{
  char *method = ngx_str_to_c_str(method_name);
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

  cJSON *param = NULL;
  cJSON_ArrayForEach(param, content_array)
  {
    if (!cJSON_IsString(param))
    {
      continue;
    }

    if (!_ngx_strstr(&args, param->valuestring))
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
