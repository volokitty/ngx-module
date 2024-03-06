#include "cJSON.h"

char *json_file_to_buffer(const char *filename);
ngx_uint_t get_status_code(cJSON *declined_params_obj, ngx_str_t method_name, ngx_str_t args);