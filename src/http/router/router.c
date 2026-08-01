#include "router.h"
#include "router_internal.h"


int route_request(config_infos_t *cfg_infos, http_session_t *session, http_response_t *serv_resp){

    if(!cfg_infos || !session || !serv_resp) return -1;

    // ------ Checking request ----------------------------------------------------
    if(cfg_infos->verbose) print_debug("Checking request\n");

    // Check for error during parsing
    if(session->parse_res != HTTP_OK){                                          
        return handle_error(cfg_infos, session, serv_resp, session->parse_res);
    }

    // Preparing local path 
    assign_real_path(cfg_infos, session->client_req);
    if(cfg_infos->verbose) print_debug("Routing - Assigned local path %s\n", session->client_req->path);

    // Request content validation (HTTP Logic and Headers)
    http_status_e check_res = check_request(cfg_infos, session->client_req);
    if(check_res != HTTP_OK){
        return handle_error(cfg_infos, session, serv_resp, check_res);
    }


    // ------ Routing request ----------------------------------------------------
    if(cfg_infos->verbose) print_debug("Request checked, routing request\n");

    char cgi_path[MAX_PATH_LEN];
    snprintf(cgi_path, MAX_PATH_LEN, "%s/cgi-bin/", cfg_infos->www_root);

    if(strncmp(session->client_req->path, cgi_path, strlen(cgi_path)) == 0){     // Using CGI

        return handle_cgi(cfg_infos, session->client_req);
    }
    else if(strcmp(session->client_req->method, "GET") == 0){            // GET

        return handle_get(cfg_infos, session, serv_resp, false);
    } 
    else if(strcmp(session->client_req->method, "HEAD") == 0) {    // HEAD 

        return handle_get(cfg_infos, session, serv_resp, true);
    }
    else if(strcmp(session->client_req->method, "OPTIONS") == 0) { // OPTIONS

        return handle_options(cfg_infos, session, serv_resp);
    } 
    else if(strcmp(session->client_req->method, "POST") == 0) {  // POST

        // wrong use case for POST, only implemented for cgi use 
        return handle_error(cfg_infos, session, serv_resp, HTTP_METHOD_NOT_ALLOWED);
    }

    else {                                                // Base case 
        int handle_res = handle_error(cfg_infos, session, serv_resp, HTTP_NOT_IMPLEMENTED);
        return handle_res;
    }

    return HTTP_OK;
}