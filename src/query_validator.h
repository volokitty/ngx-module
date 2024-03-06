#include <ngx_http.h>
#include "cJSON.h"

char *json_file_to_buffer(const char *filename);
ngx_uint_t get_status_code(cJSON *declined_params_obj, ngx_http_request_t *req);