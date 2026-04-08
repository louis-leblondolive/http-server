#include "router.h"

int route_request(config_infos *cfg_infos, request *client_req, response *serv_resp, int error_flag){

    if(cfg_infos->verbose) printf("[DEBUG] routing request\n");

    if(error_flag != 0){
        int handle_res = handle_error(serv_resp, error_flag);
        return handle_res;
    }

    if(strcmp(client_req->method, "GET") == 0){ // GET

        int handle_res = handle_get(client_req, serv_resp);
        return handle_res;
    } 
    else {
        int handle_res = handle_error(serv_resp, 501);
        return handle_res;
    }

    return 0;
}