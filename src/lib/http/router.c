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

bool path_is_valid(char *path){
    for(int i = 0; path[i] != '\0'; i++){
        char c = path[i];
        bool is_valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') 
                     || (c >= '0' && c <= '9') 
                     || c == '/' || c == '-' || c == '_' || c == '.';
        if(!is_valid) return false;
    }
    return true;
}

bool file_is_reachable(char *path){

    char absolute_path[MAX_PATH_LEN]; 
    if(getcwd(absolute_path, MAX_PATH_LEN - 4) == NULL) return false;

    char full_path[MAX_PATH_LEN * 2];
    snprintf(full_path, 2 * MAX_PATH_LEN, "%s/%s", absolute_path, path);

    char resolved_path[MAX_PATH_LEN * 2];
    if(realpath(full_path, resolved_path) == NULL) return false;

    snprintf(absolute_path, MAX_PATH_LEN, "%s/www", absolute_path);

    return strncmp(absolute_path, resolved_path, strlen(absolute_path)) == 0;
}


http_status check_request(request *client_req){

    // Checking request line
        // checking method
    if(strlen(client_req->method) == 0) return HTTP_BAD_REQUEST;
    
        // checking path 
        
    // syntax is checked before assigning real path
    // path traversal protection 
    if(!file_is_reachable(client_req->path)) return HTTP_FORBIDDEN; 

        // checking version 
    int maj = 0, min = 0;
    if(sscanf(client_req->version, "HTTP/%d.%d", &maj, &min) != 2) return HTTP_BAD_REQUEST;
    if(maj != 1 || min != 1) return HTTP_VERSION_NOT_SUPPORTED;


    // Checking headers
    bool content_length_exists = false; 
    bool host_exists = false;

    for (int i = 0; i < client_req->header_count; i++){

        header hd = client_req->headers[i];
        
        if(strcasecmp(hd.key, "Content-Length") == 0){                  // Check Content Length

            size_t len = 0;
            if(sscanf(hd.value, "%zu", &len) != 1) return HTTP_BAD_REQUEST;
            if(len != client_req->body_len) return HTTP_BAD_REQUEST;
            content_length_exists = true;
        }

        if(strcasecmp(hd.key, "Host") == 0 && strlen(hd.value) != 0){   // Check Host
            host_exists = true;
        }

        if(strcasecmp(hd.key, "Expect") == 0 && strcmp(hd.value, "100-continue") == 0){  // Expect
            return HTTP_EXPECTATION_FAILED;
        }

        if(strcasecmp(hd.key, "Transfer-Encoding") == 0){               // Transfer Encoding 
            return HTTP_NOT_IMPLEMENTED;
        }

        if(strcasecmp(hd.key, "If-Modified-Since") == 0){               // Check modification date 

            struct tm tm_client_rq = {0};
            strptime(hd.value, "%a, %d %b %Y %H:%M:%S GMT", &tm_client_rq);
            time_t client_req_modif_time = timegm(&tm_client_rq);

            struct stat st;
            if(stat(client_req->path, &st) == -1){
                return HTTP_NOT_FOUND;
            }

            if(st.st_mtime <= client_req_modif_time){
                return HTTP_NOT_MODIFIED;
            }
        }   
    }
    
    if(!content_length_exists && client_req->body_len != 0) return HTTP_BAD_REQUEST;
    if(!host_exists) return HTTP_BAD_REQUEST;

    return HTTP_OK;
}



http_status route_request(config_infos *cfg_infos, request *client_req, 
    response *serv_resp, http_status error_flag){

    if(cfg_infos->verbose) printf("[DEBUG] routing request\n");
   
    // Checking request 
    if(error_flag != HTTP_OK){
        http_status handle_res = handle_error(serv_resp, error_flag);
        return handle_res;
    }

    if(strlen(client_req->path) == 0 || client_req->path[0] != '/' 
        || !path_is_valid(client_req->path)){

        http_status handle_res = handle_error(serv_resp, HTTP_BAD_REQUEST);
        return handle_res;
    }   // need to check path syntax before assigning real path 

    assign_real_path(client_req);

    http_status check_res = check_request(client_req);
    if(check_res != HTTP_OK){
        http_status handle_res = handle_error(serv_resp, check_res);
        return handle_res;
    }


    // Routing request 
    if(strcmp(client_req->method, "GET") == 0){         // GET

        http_status handle_res = handle_get(client_req, serv_resp, false);
        return handle_res;
    } 
    else if(strcmp(client_req->method, "HEAD") == 0) {  // HEAD 

        http_status handle_res = handle_get(client_req, serv_resp, true);
        return handle_res;
    }
    else {
        http_status handle_res = handle_error(serv_resp, HTTP_NOT_IMPLEMENTED);
        return handle_res;
    }

    return HTTP_OK;
}