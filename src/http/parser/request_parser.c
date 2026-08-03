#include "parser.h"



http_status_e parse_raw_request(config_infos_t *cfg_infos, ring_buffer_t *raw_request_buf, request_t *parsed_request, 
                            ssize_t bytes_received, size_t *total_bytes_parsed, size_t *pos, 
                            bool *parsing_complete, parsing_request_state_e *parse_state){

    int local_parse_counter = 0;
    char cur_char;

    while(local_parse_counter < bytes_received){

        // Protecting against infinite requests 
        if (*total_bytes_parsed >= MAX_REQUEST_LEN) return HTTP_BAD_REQUEST;

        if(read_from_r_buffer(raw_request_buf, &cur_char) != 0) return HTTP_BAD_REQUEST;
        local_parse_counter ++;
        (*total_bytes_parsed) ++;

        int header_count = parsed_request->header_count;

        switch (*parse_state){

            case REQ_PARSING_METHOD:

                if(cur_char == '\r' || cur_char == '\n')
                    return HTTP_BAD_REQUEST;

                if(cur_char == ' '){    
                    *parse_state = REQ_PARSING_METHOD_SEPARATOR;
                    
                } else {
                    if(*pos >= MAX_METHOD_LEN) return HTTP_BAD_REQUEST;

                    parsed_request->method[*pos] = cur_char;
                    (*pos) ++;
                }
                break;


            case REQ_PARSING_METHOD_SEPARATOR:
                
                if(cur_char == '\r' || cur_char == '\n')
                    return HTTP_BAD_REQUEST;

                if(cur_char == ' ') return HTTP_BAD_REQUEST; 
                // only one space allowed between method and path 

                if(*pos >= MAX_METHOD_LEN) return HTTP_BAD_REQUEST;

                parsed_request->method[*pos] = '\0';
                parsed_request->path[0] = cur_char;
                *pos = 1;
                *parse_state = REQ_PARSING_PATH;

                if(cfg_infos->verbose) print_debug("Parser - Parsed method : %s \n", parsed_request->method);
                
                break;
                
            
            case REQ_PARSING_PATH:

                if(cur_char == '\r' || cur_char == '\n')
                    return HTTP_BAD_REQUEST;

                if(cur_char == ' '){
                   *parse_state = REQ_PARSING_PATH_SEPARATOR;

                } else {
                    if(*pos >= MAX_PATH_LEN) return HTTP_URI_TOO_LONG;

                    parsed_request->path[*pos] = cur_char;
                    (*pos) ++;
                }
                break;


            case REQ_PARSING_PATH_SEPARATOR:
                
                if(cur_char == '\r' || cur_char == '\n')
                    return HTTP_BAD_REQUEST;
                
                if(cur_char == ' ') return HTTP_BAD_REQUEST; 
                    // only one space allowed between method and path 

                if(*pos >= MAX_PATH_LEN) return HTTP_BAD_REQUEST;

                parsed_request->path[*pos] = '\0';
                parsed_request->version[0] = cur_char;
                *pos = 1;
                *parse_state = REQ_PARSING_VERSION;

                if(cfg_infos->verbose) print_debug("Parser - Parsed path : %s \n", parsed_request->path);

                break;

            
            case REQ_PARSING_VERSION:

                if(cur_char == '\n') return HTTP_BAD_REQUEST;
                if(cur_char == '\r'){

                    if(*pos >= MAX_VERSION_LEN) return HTTP_BAD_REQUEST;

                    parsed_request->version[*pos] = '\0';
                    *pos = 0;

                    *parse_state = REQ_EXPECTING_LF;
                    if(cfg_infos->verbose) print_debug("Parser - Parsed version : %s \n", parsed_request->version);
                }
                else {
                    if(*pos >= MAX_VERSION_LEN) return HTTP_BAD_REQUEST;

                    parsed_request->version[*pos] = cur_char;
                    (*pos) ++;
                }
                break;

            
            case REQ_EXPECTING_LF:
                
                if(cur_char == '\n') *parse_state = REQ_PARSING_NEW_LINE;
                else return HTTP_BAD_REQUEST;
                break;

            
            case REQ_PARSING_NEW_LINE:
                
                if(cur_char == '\r') *parse_state = REQ_EXPECTING_FINAL_LF;
                else {
                    if(header_count >= MAX_HEADER_NB) return HTTP_BAD_REQUEST;

                    parsed_request->headers[parsed_request->header_count].key[0] = cur_char;
                    *pos = 1;
                    *parse_state = REQ_PARSING_HEADER_KEY;
                }
                break;


            case REQ_PARSING_HEADER_KEY:
                
                if(header_count >= MAX_HEADER_NB) return HTTP_BAD_REQUEST;

                if(cur_char == '\r' || cur_char == '\n')
                    return HTTP_BAD_REQUEST;
    
                if(cur_char == ':'){

                    if(*pos <= 0) return HTTP_BAD_REQUEST; 
                        // key-less header is forbidden
                    if(parsed_request->headers[header_count].key[*pos - 1] == ' ') 
                        return HTTP_BAD_REQUEST;           
                        // white space before ':' is forbidden 

                    if(*pos >= MAX_HEADER_KEY_SIZE) return HTTP_BAD_REQUEST;

                    parsed_request->headers[header_count].key[*pos] = '\0';
                    *pos = 0;
                    *parse_state = REQ_PARSING_HEADER_KEY_SEPARATOR;

                    if(cfg_infos->verbose) print_debug("Parser - Parsed header key : %s \n",
                         parsed_request->headers[header_count].key);
                }
                else{
                    if(*pos >= MAX_HEADER_KEY_SIZE) return HTTP_HEADER_TOO_LARGE;
                    
                    parsed_request->headers[header_count].key[*pos] = cur_char;
                    (*pos) ++;
                } 
                break;

            
            case REQ_PARSING_HEADER_KEY_SEPARATOR:
                
                if(cur_char == '\r' || cur_char == '\n') return HTTP_BAD_REQUEST;   // empty value

                if(cur_char != ' '){
                    parsed_request->headers[header_count].value[0] = cur_char;
                    (*pos) = 1;
                    *parse_state = REQ_PARSING_HEADER_VALUE;
                } 
                break;


            case REQ_PARSING_HEADER_VALUE:

                if(cur_char == '\r'){

                    parsed_request->headers[header_count].value[*pos] = '\0';
                    *pos = 0;
                    *parse_state = REQ_EXPECTING_LF;
                    
                    if(cfg_infos->verbose) print_debug("Parser - Parsed header value : %s \n",
                         parsed_request->headers[header_count].value);

                    parsed_request->header_count ++;
                }
                else {
                    if(*pos >= MAX_HEADER_VALUE_SIZE) return HTTP_HEADER_TOO_LARGE;

                    parsed_request->headers[header_count].value[*pos] = cur_char;
                    (*pos) ++;
                    
                }
                break;
                
            
            case REQ_EXPECTING_FINAL_LF:
                
                if(cur_char != '\n') return HTTP_BAD_REQUEST;

                // Double end of line (\r\n\r\n) has been found 
                // Assigning crucial header values to corresponding request fields 
                for (size_t i = 0; i < parsed_request->header_count; i++){
                    header_t hd = parsed_request->headers[i];
        
                    if(strcasecmp(hd.key, "Connection") == 0){
                        snprintf(parsed_request->connection_type, MAX_HEADER_VALUE_SIZE, "%s", hd.value);
                    }
                    if(strcasecmp(hd.key, "Content-Length") == 0){
                        size_t len = 0;
                        if(sscanf(hd.value, "%zu", &len) != 1) return HTTP_BAD_REQUEST;
                        parsed_request->body_len = len;
                    }
                }               

                // Switching to parsing body 
                *pos = 0;
                if(parsed_request->body_len == 0){
                    *parsing_complete = true;
                    return HTTP_OK;
                }

                *parse_state = REQ_PARSING_BODY;
                break;
            

            case REQ_PARSING_BODY:
                if (*pos >= MAX_BODY_LEN) return HTTP_REQUEST_ENTITY_TOO_LARGE;

                parsed_request->body[*pos] = cur_char;
                (*pos)++;

                if (*pos >= parsed_request->body_len) {
                    *parsing_complete = true;
                    *parse_state = REQ_END_PARSING;
                    if(cfg_infos->verbose) print_debug("Parser - Parsed body : %s \n", parsed_request->body);
                }
                break;

                
            case REQ_END_PARSING:
                *parsing_complete = true;
                if(cfg_infos->verbose) print_debug("Parser - Done parsing\n");
                return HTTP_OK;
            
            default:  // This case can't be met 
                return HTTP_INTERNAL_ERROR;
        }
    }

    if(*parse_state == REQ_END_PARSING){
        *parsing_complete = true;
        if(cfg_infos->verbose) print_debug("Parser - Done parsing\n");
    }
    else if(*parse_state == REQ_PARSING_BODY && *pos >= parsed_request->body_len){
        *parsing_complete = true;
        if(cfg_infos->verbose) print_debug("Done parsing\n");
    } else {
        if(cfg_infos->verbose) print_debug("Parser - Parsing body interupted, pos = %zu and bodylen = %zu\n", 
            *pos, parsed_request->body_len);
    }

    return HTTP_OK;
}