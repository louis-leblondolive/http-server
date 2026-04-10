#include "parser.h"


void print_request(request *r){
    printf("%s %s %s\n", r->method, r->path, r->version);
    for (int i = 0; i < r->header_count; i++){
        printf("%s: %s\n", r->headers[i].key, r->headers[i].value);
    }
    printf("\n");
    printf("%s\n", r->body);
}


http_status move_to_next_line(char *raw_request, int bytes_received, int *cursor, bool *seen_separator){
        
        char_expect expected_char = EXPECTING_CR;
        *seen_separator = false;

        while(*cursor < bytes_received && *cursor < MAX_REQUEST_LEN){

            switch (expected_char)
            {
            case EXPECTING_CR:
                if(raw_request[*cursor] == '\r'){
                    expected_char = EXPECTING_LF; 
                    (*cursor)++;
                }
                else return HTTP_BAD_REQUEST;    
                break;
            
            case EXPECTING_LF:
                if(raw_request[*cursor] == '\n'){
                    expected_char = ANY;
                    (*cursor)++;
                }
                else return HTTP_BAD_REQUEST;    
                break;
            
            case ANY:
                if(raw_request[*cursor] == '\r'){
                    expected_char = EXPECTING_FINAL_LF;
                    (*cursor)++;
                }
                else{
                    return HTTP_OK;
                }
                break;

            case EXPECTING_FINAL_LF:
                if(raw_request[*cursor] == '\n'){
                    (*cursor)++;
                    *seen_separator = true;
                    return HTTP_OK; 
                } else {
                    return HTTP_BAD_REQUEST;
                }
            
            default:
                return HTTP_INTERNAL_ERROR;
            }
        }
    return HTTP_OK; // reached the end of the request, handled in parse_raw_request 
}


http_status parse_raw_request(char *raw_request, request *client_req, int bytes_received){

    parsing_state current_state = PARSING_METHOD;
    
    int cursor = 0;
    int pos = 0;
    int header_count = 0;
    bool seen_separator = false;


    while(cursor < bytes_received){

        if (cursor >= MAX_REQUEST_LEN) return HTTP_BAD_REQUEST;

        switch (current_state){

        case PARSING_METHOD:

            if(raw_request[cursor] == ' '){

                cursor ++;
                if( (cursor >= MAX_REQUEST_LEN || cursor >= bytes_received) 
                    || raw_request[cursor] == ' ') return HTTP_BAD_REQUEST;
                // only one space allowed between method and path

                client_req->method[pos] = '\0';
                pos = 0;
                current_state = PARSING_PATH;

            } else {
                if(pos >= MAX_METHOD_LEN) return HTTP_BAD_REQUEST;

                client_req->method[pos] = raw_request[cursor];
                pos ++;
                cursor ++;
            }
            break;
            

        case PARSING_PATH:

            if(raw_request[cursor] == ' '){

                cursor ++;
                if( (cursor >= MAX_REQUEST_LEN || cursor >= bytes_received) 
                    || raw_request[cursor] == ' ') return HTTP_BAD_REQUEST;
                // only one space allowed between path and version

                client_req->path[pos] = '\0';
                pos = 0;
                current_state = PARSING_VERSION;

            } else {
                if(pos >= MAX_PATH_LEN) return HTTP_URI_TOO_LONG;

                client_req->path[pos] = raw_request[cursor];
                pos ++;
                cursor ++;
            }
            break;
        

        case PARSING_VERSION:

            if(raw_request[cursor] == '\n') return HTTP_BAD_REQUEST;
            if(raw_request[cursor] == '\r'){

                client_req->version[pos] = '\0';
                pos = 0;

                int try_move = move_to_next_line(raw_request, bytes_received, &cursor, &seen_separator);
                if(try_move != HTTP_OK) return try_move;

                if(seen_separator){
                    current_state = PARSING_BODY;
                } else {
                    current_state = PARSING_HEADER_KEY;
                }
            }
            else {
                if(pos >= MAX_VERSION_LEN) return HTTP_BAD_REQUEST;

                client_req->version[pos] = raw_request[cursor];
                pos ++;
                cursor ++;
            }
            break;


        case PARSING_HEADER_KEY:

            if(header_count >= MAX_HEADER_NB) return HTTP_BAD_REQUEST;

            if(raw_request[cursor] == ':'){

                if(pos <= 0) return HTTP_BAD_REQUEST; 
                    // key-less header is forbidden
                if(client_req->headers[header_count].key[pos - 1] == ' ') 
                    return HTTP_BAD_REQUEST;           
                    // white space before ':' is forbidden 

                cursor ++;
                while(cursor < bytes_received && raw_request[cursor] == ' ') cursor ++;

                client_req->headers[header_count].key[pos] = '\0';
                pos = 0;
                current_state = PARSING_HEADER_VALUE;
            }
            else{
                if(pos >= MAX_HEADER_KEY_SIZE) return HTTP_HEADER_TOO_LARGE;
                
                client_req->headers[header_count].key[pos] = raw_request[cursor];
                pos ++;
                cursor ++;
            } 
            break;

        case PARSING_HEADER_VALUE:

            if(raw_request[cursor] == '\r'){

                client_req->headers[header_count].value[pos] = '\0';
                pos = 0;

                int try_move = move_to_next_line(raw_request, bytes_received, &cursor, &seen_separator);
                if(try_move != HTTP_OK) return try_move;
                
                if(seen_separator){
                    current_state = PARSING_BODY;
                } else {
                    current_state = PARSING_HEADER_KEY;
                }

                header_count ++;
            }
            else {
                if(pos >= MAX_HEADER_VALUE_SIZE) return HTTP_HEADER_TOO_LARGE;

                client_req->headers[header_count].value[pos] = raw_request[cursor];
                pos ++;
                cursor ++;
            }
            
            break;

        case PARSING_BODY:
            
            if(pos >= MAX_BODY_LEN) return HTTP_REQUEST_ENTITY_TOO_LARGE;

            client_req->body[pos] = raw_request[cursor];
            pos ++;
            cursor ++;
            break;

        default:
            return HTTP_INTERNAL_ERROR; // current_state must be in one of the above states
        }
    }

    client_req->body[pos] = '\0';
    client_req->header_count = header_count;


    return HTTP_OK;
}