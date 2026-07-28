#include "router.h"

/**
 * @brief Secures the path by adding root directory www as a prefix
 * Replaces root "/" with the default path 
 */
static void assign_real_path(config_infos *cfg_infos, request *client_req){

    if(strcmp(client_req->path, "/") == 0){
        snprintf(client_req->path, MAX_PATH_LEN, "%s/%s", cfg_infos->www_root, DEFAULT_PATH);
    } else {
        char new_path[MAX_PATH_LEN];
        snprintf(new_path, MAX_PATH_LEN, "%s%s", cfg_infos->www_root, client_req->path);
        strlcpy(client_req->path, new_path, MAX_PATH_LEN);
    }
}

/**
 * @brief Checks if path is only made of allowed characters 
 * @param path The path contained in the client request 
 */
static bool path_is_valid(char *path){

    for(int i = 0; path[i] != '\0'; i++){
        char c = path[i];
        bool is_valid = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') 
                     || (c >= '0' && c <= '9') 
                     || c == '/' || c == '-' || c == '_' || c == '.';
        if(!is_valid) return false;
    }
    return true;
}

/**
 * @brief Checks if file pointed to by path exists.
 * @param path The local path to the file (including www prefix)
 */
static bool file_exists(char *path){

    return (access(path, F_OK) == 0) ;
}

/**
 * @brief Checks if file is a descendant of the www_root directory to 
 *  avoid path traversal 
 * @param path The local path to the file (including www_root prefix)
 */
static bool file_access_allowed(config_infos *cfg_infos, char *path){


    char resolved_path[MAX_PATH_LEN * 2];
    if(realpath(path, resolved_path) == NULL) return false;

    return strncmp(resolved_path, cfg_infos->www_root, strlen(cfg_infos->www_root)) == 0;
}

/** 
 * @brief Checks if request content is correct
 * @return HTTP_OK if request content is valid, corresponding http_status otherwise 
 * @note Local path (including www prefix) must have been assigned to client request before calling
 */
static http_status check_request(config_infos *cfg_infos, request *client_req){

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

        header hd = client_req->headers[i];
        
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
            strptime(hd.value, "%a, %d %b %Y %H:%M:%S GMT", &tm_client_rq);
            time_t client_req_modif_time = timegm(&tm_client_rq);

            struct stat st;
            if(stat(clean_path, &st) == -1){
                return HTTP_NOT_FOUND;
            }

            if(st.st_mtime <= client_req_modif_time){
                return HTTP_NOT_MODIFIED;
            }
        }   
    }
    
    if(!content_length_exists && client_req->body_len != 0) return HTTP_BAD_REQUEST;
    if(host_counter != 1) return HTTP_BAD_REQUEST;

    return HTTP_OK;
}



int route_request(config_infos *cfg_infos, request *client_req, 
    http_status error_flag){

    
    // ------ Checking request ----------------------------------------------------
    if(cfg_infos->verbose) print_debug("Checking request\n");

    // Check for error during parsing
    if(error_flag != HTTP_OK){                                          
        int handle_res = handle_error(cfg_infos, error_flag);
        return handle_res;
    }

    // Preparing local path 
    assign_real_path(cfg_infos, client_req);
    if(cfg_infos->verbose) print_debug("Routing - Assigned local path %s\n", client_req->path);

    // Request content validation (HTTP Logic and Headers)
    http_status check_res = check_request(cfg_infos, client_req);       
    if(check_res != HTTP_OK){
        int handle_res = handle_error(cfg_infos, check_res);
        return handle_res;
    }


    // ------ Routing request ----------------------------------------------------
    if(cfg_infos->verbose) print_debug("Request checked, routing request\n");

    char cgi_path[MAX_PATH_LEN];
    snprintf(cgi_path, MAX_PATH_LEN, "%s/cgi-bin/", cfg_infos->www_root);

    if(strncmp(client_req->path, cgi_path, strlen(cgi_path)) == 0){     // Using CGI

        int handle_res = handle_cgi(cfg_infos, client_req);
        return handle_res;
    }
    else if(strcmp(client_req->method, "GET") == 0){            // GET

        int handle_res = handle_get(cfg_infos, client_req, false);
        return handle_res;
    } 
    else if(strcmp(client_req->method, "HEAD") == 0) {    // HEAD 

        int handle_res = handle_get(cfg_infos, client_req, true);
        return handle_res;
    }
    else if(strcmp(client_req->method, "OPTIONS") == 0) { // OPTIONS

        int handle_res = handle_options(cfg_infos);
        return handle_res;
    } 
    else if(strcmp(client_req->method, "POST") == 0) {  // POST

        // wrong use case for POST, only implemented for cgi use 
        return HTTP_METHOD_NOT_ALLOWED;
    }

    else {                                                // Base case 
        int handle_res = handle_error(cfg_infos, HTTP_NOT_IMPLEMENTED);
        return handle_res;
    }

    return HTTP_OK;
}