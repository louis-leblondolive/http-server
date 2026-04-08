#include "router.h"

int route_request(request *client_req, response *serv_resp, int error_flag){

    printf("routing request\n");

    if(error_flag != 0){
        int handle_res = handle_error(serv_resp, error_flag);
        return handle_res;
    }

    if(strcmp(client_req->method, "GET") == 0){ // GET

        // need to implement get method
        int handle_res = handle_error(serv_resp, 501);
        return handle_res;
    } 
    else {
        int handle_res = handle_error(serv_resp, 501);
        return handle_res;
    }

    return 0;
}