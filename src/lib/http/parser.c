#include "parser.h"


http_status parse_raw_request(config_infos *cfg_infos, r_buffer *raw_request_buf, request *parsed_request, 
                            ssize_t bytes_received, size_t *total_bytes_parsed, size_t *pos, 
                            bool *parsing_complete, parsing_request_state *parse_state){

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

                if(cur_char == ' '){
                    
                    *parse_state = REQ_PARSING_METHOD_SEPARATOR;
                    if(cfg_infos->verbose) print_debug("Parser - Parsed method : %s \n", parsed_request->method);

                } else {
                    if(*pos >= MAX_METHOD_LEN) return HTTP_BAD_REQUEST;

                    parsed_request->method[*pos] = cur_char;
                    (*pos) ++;
                }
                break;


            case REQ_PARSING_METHOD_SEPARATOR:
                
                if(cur_char == ' ') return HTTP_BAD_REQUEST; 
                // only one space allowed between method and path 

                parsed_request->method[*pos] = '\0';
                parsed_request->path[0] = cur_char;
                *pos = 1;
                *parse_state = REQ_PARSING_PATH;

                break;
                
            
            case REQ_PARSING_PATH:

                if(cur_char == ' '){

                   *parse_state = REQ_PARSING_PATH_SEPARATOR;
                   if(cfg_infos->verbose) print_debug("Parser - Parsed path : %s \n", parsed_request->path);

                } else {
                    if(*pos >= MAX_PATH_LEN) return HTTP_URI_TOO_LONG;

                    parsed_request->path[*pos] = cur_char;
                    (*pos) ++;
                }
                break;


            case REQ_PARSING_PATH_SEPARATOR:
                
                if(cur_char == ' ') return HTTP_BAD_REQUEST; 
                    // only one space allowed between method and path 

                    parsed_request->path[*pos] = '\0';
                    parsed_request->version[0] = cur_char;
                    *pos = 1;
                    *parse_state = REQ_PARSING_VERSION;

                    break;

            
            case REQ_PARSING_VERSION:

                if(cur_char == '\n') return HTTP_BAD_REQUEST;
                if(cur_char == '\r'){

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
    
                if(cur_char == ':'){

                    if(*pos <= 0) return HTTP_BAD_REQUEST; 
                        // key-less header is forbidden
                    if(parsed_request->headers[header_count].key[*pos - 1] == ' ') 
                        return HTTP_BAD_REQUEST;           
                        // white space before ':' is forbidden 

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
                    parsed_request->header_count ++;

                    *parse_state = REQ_EXPECTING_LF;
                    if(cfg_infos->verbose) print_debug("Parser - Parsed header value : %s \n",
                         parsed_request->headers[header_count - 1].value);
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
                for (int i = 0; i < parsed_request->header_count; i++){
                    header hd = parsed_request->headers[i];
        
                    if(strcasecmp(hd.key, "Connection") == 0){
                        strlcpy(parsed_request->connection_type, hd.value, MAX_HEADER_VALUE_SIZE);
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
                if (*pos >= MAX_BODY_LEN - 1) return HTTP_REQUEST_ENTITY_TOO_LARGE;

                parsed_request->body[*pos] = cur_char;
                (*pos)++;

                if (*pos >= parsed_request->body_len) {
                    parsed_request->body[*pos] = '\0'; 
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



http_status parse_raw_cgi_response(config_infos *cfg_infos, r_buffer *raw_response_buf, 
                            response_head *parsed_response_head, char *parsed_resp_body,
                            ssize_t bytes_received, size_t *total_bytes_parsed, size_t *pos, 
                            bool *parsing_complete, parsing_response_state *parse_state, bool *has_body_length){

    int local_parse_counter = 0;
    char cur_char;

    while(local_parse_counter < bytes_received){

        // Protecting against infinite responses 
        if (*total_bytes_parsed >= MAX_REQUEST_LEN) return HTTP_BAD_GATEWAY;

        if(read_from_r_buffer(raw_response_buf, &cur_char) != 0) return HTTP_BAD_GATEWAY;
        local_parse_counter ++;
        (*total_bytes_parsed) ++;

        int header_count = parsed_response_head->header_count;

        switch (*parse_state){
            
            case RESP_EXPECTING_LF:
                
                if(cur_char == '\n') *parse_state = RESP_PARSING_NEW_LINE;
                else return HTTP_BAD_GATEWAY;
                break;

            
            case RESP_PARSING_NEW_LINE:
                
                if(cur_char == '\r' || cur_char == '\n'){
                    // Assigning crucial header values to corresponding response fields 
                    strlcpy(parsed_response_head->code, "200", MAX_CODE_LEN);
                    strlcpy(parsed_response_head->reason, "OK", MAX_REASON_LEN);
                    
                    for (int i = 0; i < parsed_response_head->header_count; i++){
                        header hd = parsed_response_head->headers[i];
            
                        if(strcasecmp(hd.key, "Connection") == 0){
                            strlcpy(cfg_infos->connection_type, hd.value, MAX_HEADER_VALUE_SIZE);
                        }
                        if(strcasecmp(hd.key, "Content-Length") == 0){
                            size_t len = 0;
                            if(sscanf(hd.value, "%zu", &len) != 1) return HTTP_BAD_GATEWAY;
                            parsed_response_head->content_len = len;
                            *has_body_length = true;
                        }

                        if(strcasecmp(hd.key, "Status") == 0){
                            char code[MAX_CODE_LEN];
                            int offset = 0;
                            if(sscanf(hd.value, "%s %n", code, &offset) < 1) return HTTP_BAD_GATEWAY;
                            strlcpy(parsed_response_head->code, code, MAX_CODE_LEN);
                            strlcpy(parsed_response_head->reason, hd.value + offset, MAX_REASON_LEN);
                        }
                    }

                    if(cur_char == '\r') *parse_state = RESP_EXPECTING_FINAL_LF;
                    else{
                        if(parsed_response_head->content_len == 0){
                            *parsing_complete = true;
                            return HTTP_OK;
                        }
                        *parse_state = RESP_PARSING_BODY;
                    } 
                } 
                
                else {
                    if(header_count >= MAX_HEADER_NB) return HTTP_BAD_GATEWAY;

                    parsed_response_head->headers[parsed_response_head->header_count].key[0] = cur_char;
                    *pos = 1;
                    *parse_state = RESP_PARSING_HEADER_KEY;
                }
                break;


            case RESP_PARSING_HEADER_KEY:
                
                if(header_count >= MAX_HEADER_NB) return HTTP_BAD_GATEWAY;
    
                if(cur_char == ':'){

                    if(*pos <= 0) return HTTP_BAD_GATEWAY; 
                        // key-less header is forbidden
                    if(parsed_response_head->headers[header_count].key[*pos - 1] == ' ') 
                        return HTTP_BAD_GATEWAY;           
                        // white space before ':' is forbidden 

                    parsed_response_head->headers[header_count].key[*pos] = '\0';
                    *pos = 0;
                    *parse_state = RESP_PARSING_HEADER_KEY_SEPARATOR;
                    if(cfg_infos->verbose) print_debug("Parser - Parsed CGI response header key: %s \n", 
                        parsed_response_head->headers[header_count].key);
                }
                else{
                    if(*pos >= MAX_HEADER_KEY_SIZE) return HTTP_BAD_GATEWAY;
                    
                    parsed_response_head->headers[header_count].key[*pos] = cur_char;
                    (*pos) ++;
                } 
                break;

            
            case RESP_PARSING_HEADER_KEY_SEPARATOR:
                
                if(cur_char == '\r' || cur_char == '\n') return HTTP_BAD_GATEWAY;   // empty value

                if(cur_char != ' '){
                    parsed_response_head->headers[header_count].value[0] = cur_char;
                    (*pos) = 1;
                    *parse_state = RESP_PARSING_HEADER_VALUE;
                } 
                break;


            case RESP_PARSING_HEADER_VALUE:

                if(cur_char == '\r' || cur_char == '\n'){

                    parsed_response_head->headers[header_count].value[*pos] = '\0';
                    *pos = 0;
                    parsed_response_head->header_count ++;

                    if(cur_char == '\r') *parse_state = RESP_EXPECTING_LF;
                    else *parse_state = RESP_PARSING_NEW_LINE;

                    if(cfg_infos->verbose) print_debug("Parser - Parsed CGI response header value: %s \n", 
                        parsed_response_head->headers[header_count - 1].value);
                } 
                else {
                    if(*pos >= MAX_HEADER_VALUE_SIZE) return HTTP_BAD_GATEWAY;

                    parsed_response_head->headers[header_count].value[*pos] = cur_char;
                    (*pos) ++;
                    
                }
                break;
                
            
            case RESP_EXPECTING_FINAL_LF:
                
                if(cur_char != '\n') return HTTP_BAD_GATEWAY;

                // Double end of line (\r\n\r\n) has been found 
                // Switching to parsing body 
                *pos = 0;
                if(parsed_response_head->content_len == 0){
                    *parsing_complete = true;
                    return HTTP_OK;
                }

                *parse_state = RESP_PARSING_BODY;
                break;
            

            case RESP_PARSING_BODY:
                if (*pos >= MAX_BODY_LEN - 1) return HTTP_BAD_GATEWAY;

                parsed_resp_body[*pos] = cur_char;
                (*pos)++;

                if (*pos >= parsed_response_head->content_len) {
                    parsed_resp_body[*pos] = '\0'; 
                    *parsing_complete = true;
                    *parse_state = RESP_END_PARSING;
                }
                break;

                
            case RESP_END_PARSING:
                *parsing_complete = true;
                return HTTP_OK;
            
            default:  // This case can't be met 
                return HTTP_INTERNAL_ERROR;
        }
    }

    if(*parse_state == RESP_END_PARSING){
        *parsing_complete = true;
        if(cfg_infos->verbose) print_debug("Parser - Done parsing\n");
    }
    else if(*parse_state == RESP_PARSING_BODY && *pos >= parsed_response_head->content_len){
        *parsing_complete = true;
        if(cfg_infos->verbose) print_debug("Parser - Done parsing\n");
    } else {
        if(cfg_infos->verbose) print_debug("Parser - Parsing body interupted, pos = %zu and bodylen = %zu\n", 
            *pos, parsed_response_head->content_len);
    }
    return HTTP_OK;
}