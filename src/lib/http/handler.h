#ifndef HANDLER
#define HANDLER

#include <string.h>

#include "structures.h"
#include "response.h"

int handle_error(response *serv_resp, int err_code);

#endif