#include "router_internal.h"



void assign_real_path(config_infos_t *cfg_infos, request_t *client_req){

    if(strcmp(client_req->path, "/") == 0){
        snprintf(client_req->path, MAX_PATH_LEN, "%s/%s", cfg_infos->www_root, DEFAULT_PATH);
    } else {
        char new_path[MAX_PATH_LEN];
        snprintf(new_path, MAX_PATH_LEN, "%s%s", cfg_infos->www_root, client_req->path);
        snprintf(client_req->path, MAX_PATH_LEN, "%s", new_path);
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


bool file_exists(char *path){
    return (access(path, F_OK) == 0) ;
}


bool file_access_allowed(config_infos_t *cfg_infos, char *path){

    char resolved_path[MAX_PATH_LEN * 2];
    if(realpath(path, resolved_path) == NULL) return false;

    return strncmp(resolved_path, cfg_infos->www_root, strlen(cfg_infos->www_root)) == 0;
}


http_status_e check_request(config_infos_t *cfg_infos, request_t *client_req){

    char clean_path[MAX_PATH_LEN];
    sscanf(client_req->path, "%[^?]", clean_path); // Getting rid of query string for get cgi-requests

    // -------  Checking request status line -------------------------------------
    if(strlen(client_req->method) == 0) return HTTP_BAD_REQUEST;
    
        // Checking path 
        // Assuming that www_root prefix has been added
    if(strncmp(client_req->path, cfg_infos->www_root, strlen(cfg_infos->www_root)) != 0   
        || !path_is_valid(clean_path)){                           

        return HTTP_BAD_REQUEST;
    }   

    if(!file_exists(clean_path)) return HTTP_NOT_FOUND;
    if(!file_access_allowed(cfg_infos, clean_path)) return HTTP_FORBIDDEN; 

        // checking version 
    int maj = 0, min = 0;
    if(sscanf(client_req->version, "HTTP/%d.%d", &maj, &min) != 2) return HTTP_BAD_REQUEST;
    if(maj != 1 || min != 1) return HTTP_VERSION_NOT_SUPPORTED;


    // ------ Checking headers -----------------------------------------------------
    bool content_length_exists = false; 
    int host_counter = 0;

    for (int i = 0; i < client_req->header_count; i++){

        header_t hd = client_req->headers[i];
        
        if(strcasecmp(hd.key, "Content-Length") == 0){                  // Check Content Length
            content_length_exists = true;
        }

        if(strcasecmp(hd.key, "Host") == 0 && strlen(hd.value) != 0){   // Check Host
            host_counter ++;
        }

        if(strcasecmp(hd.key, "Expect") == 0 && strcmp(hd.value, "100-continue") == 0){  // Expect
            return HTTP_EXPECTATION_FAILED;
        }

        if(strcasecmp(hd.key, "Transfer-Encoding") == 0){               // Transfer Encoding 
            return HTTP_NOT_IMPLEMENTED;
        }

        if(strcasecmp(hd.key, "If-Modified-Since") == 0){               // Check modification date 

            struct tm tm_client_rq = {0};
            
            if(strptime(hd.value, "%a, %d %b %Y %H:%M:%S GMT", &tm_client_rq) != NULL){
                
                time_t client_req_modif_time = timegm(&tm_client_rq);

                if(client_req_modif_time != (time_t)-1){
                    struct stat st;
                    if(stat(clean_path, &st) == -1){
                        return HTTP_NOT_FOUND;
                    }

                    if(st.st_mtime <= client_req_modif_time){
                        return HTTP_NOT_MODIFIED;
                    }
                }   
            }
        }
    }
    
    if(!content_length_exists && client_req->body_len != 0) return HTTP_BAD_REQUEST;
    if(host_counter != 1) return HTTP_BAD_REQUEST;

    return HTTP_OK;
}