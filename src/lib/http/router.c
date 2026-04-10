#include "router.h"

void assign_real_path(request *client_req){
    if(strcmp(client_req->path, "/") == 0){
        strlcpy(client_req->path, "www/index.html", MAX_PATH_LEN);
    } else {
        char new_path[MAX_PATH_LEN];
        snprintf(new_path, MAX_PATH_LEN, "www/%s", client_req->path);
        strlcpy(client_req->path, new_path, MAX_PATH_LEN);
    }
}

bool file_is_reachable(char *path){
    return true;
}


http_status check_request(request *client_req){

    if(!file_is_reachable(client_req->path)) return HTTP_FORBIDDEN; 

    return HTTP_OK;
}



http_status route_request(config_infos *cfg_infos, request *client_req, response *serv_resp, http_status error_flag){

    if(cfg_infos->verbose) printf("[DEBUG] routing request\n");

    if(error_flag != HTTP_OK){
        http_status handle_res = handle_error(serv_resp, error_flag);
        return handle_res;
    }

    assign_real_path(client_req);

    http_status check_res = check_request(client_req);
    if(check_res != HTTP_OK){
        http_status handle_res = handle_error(serv_resp, check_res);
        return handle_res;
    }

    if(strcmp(client_req->method, "GET") == 0){ // GET

        http_status handle_res = handle_get(client_req, serv_resp);
        return handle_res;
    } 
    else {
        http_status handle_res = handle_error(serv_resp, HTTP_NOT_IMPLEMENTED);
        return handle_res;
    }

    return HTTP_OK;
}