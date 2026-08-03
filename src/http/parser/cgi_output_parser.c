#include "parser.h"



http_status_e parse_raw_cgi_output(config_infos_t *cfg_infos, ring_buffer_t *raw_response_buf, 
                            http_cgi_output_t *parsed_cgi_output,  
                            ssize_t bytes_received, size_t *total_bytes_parsed, size_t *pos, 
                            bool *parsing_complete, parsing_cgi_output_state_e *parse_state, bool *has_body_length){

    int local_parse_counter = 0;
    char cur_char;
    
    size_t header_count = parsed_cgi_output->header_count;

    while(local_parse_counter < bytes_received){

        // Protecting against infinite responses 
        if (*total_bytes_parsed >= MAX_REQUEST_LEN) return HTTP_BAD_GATEWAY;

        if(read_from_r_buffer(raw_response_buf, &cur_char) != 0) return HTTP_BAD_GATEWAY;
        local_parse_counter ++;
        (*total_bytes_parsed) ++;
        
        switch (*parse_state){
            
            case CGI_OUT_EXPECTING_LF:
                
                if(cur_char == '\n') *parse_state = CGI_OUT_PARSING_NEW_LINE;
                else return HTTP_BAD_GATEWAY;
                break;

            
            case CGI_OUT_PARSING_NEW_LINE:
                
                if(cur_char == '\r' || cur_char == '\n'){
                    // CRLF reached 

                    // Assigning crucial header values to corresponding response fields 
                    for (size_t i = 0; i < parsed_cgi_output->header_count; i++){
                        header_t hd = parsed_cgi_output->headers[i];
            
                        if(strcasecmp(hd.key, "Connection") == 0){
                            snprintf(parsed_cgi_output->connection_type, MAX_HEADER_VALUE_SIZE, "%s", hd.value);
                        }

                        if(strcasecmp(hd.key, "Content-Length") == 0){
                            size_t len = 0;
                            if(sscanf(hd.value, "%zu", &len) != 1) return HTTP_BAD_GATEWAY;
                            parsed_cgi_output->body_len = len;
                            *has_body_length = true;
                        }
                    }

                    if(cur_char == '\r') *parse_state = CGI_OUT_EXPECTING_FINAL_LF;
                    else{ // '\n'
                        if(parsed_cgi_output->body_len == 0){
                            *parsing_complete = true;
                            return HTTP_OK;
                        }
                        *parse_state = CGI_OUT_PARSING_BODY;
                    } 
                } 
                
                else {
                    if(header_count >= MAX_HEADER_NB) return HTTP_BAD_GATEWAY;

                    parsed_cgi_output->headers[parsed_cgi_output->header_count].key[0] = cur_char;
                    *pos = 1;
                    *parse_state = CGI_OUT_PARSING_HEADER_KEY;
                }
                break;


            case CGI_OUT_PARSING_HEADER_KEY:
                
                if(header_count >= MAX_HEADER_NB) return HTTP_BAD_GATEWAY;
                
                if(cur_char == '\r' || cur_char == '\n')
                    return HTTP_BAD_REQUEST;

                if(cur_char == ':'){

                    if(*pos <= 0) return HTTP_BAD_GATEWAY; 
                        // key-less header is forbidden
                    if(parsed_cgi_output->headers[header_count].key[*pos - 1] == ' ') 
                        return HTTP_BAD_GATEWAY;           
                        // white space before ':' is forbidden 

                    parsed_cgi_output->headers[header_count].key[*pos] = '\0';
                    *pos = 0;
                    *parse_state = CGI_OUT_PARSING_HEADER_KEY_SEPARATOR;
                    if(cfg_infos->verbose) print_debug("Parser - Parsed CGI response header key: %s \n", 
                        parsed_cgi_output->headers[header_count].key);
                }
                else{
                    if(*pos >= MAX_HEADER_KEY_SIZE) return HTTP_BAD_GATEWAY;
                    
                    parsed_cgi_output->headers[header_count].key[*pos] = cur_char;
                    (*pos) ++;
                } 
                break;

            
            case CGI_OUT_PARSING_HEADER_KEY_SEPARATOR:
                
                if(cur_char == '\r' || cur_char == '\n') return HTTP_BAD_GATEWAY;   // empty value

                if(cur_char != ' '){
                    parsed_cgi_output->headers[header_count].value[0] = cur_char;
                    (*pos) = 1;
                    *parse_state = CGI_OUT_PARSING_HEADER_VALUE;
                } 
                break;


            case CGI_OUT_PARSING_HEADER_VALUE:

                if(cur_char == '\r' || cur_char == '\n'){

                    parsed_cgi_output->headers[header_count].value[*pos] = '\0';
                    *pos = 0;

                    if(cfg_infos->verbose) print_debug("Parser - Parsed CGI response header value: %s \n", 
                        parsed_cgi_output->headers[header_count].value);
                    
                    // Skipping status header copy 
                    if(strcasecmp(parsed_cgi_output->headers[header_count].key, "Status") == 0){

                            int code; 
                            if(sscanf(parsed_cgi_output->headers[header_count].value, "%d", &code) < 1) return HTTP_BAD_GATEWAY;
                            parsed_cgi_output->status = get_status_from_code(code);

                            if(parsed_cgi_output->status == HTTP_INTERNAL_ERROR) return HTTP_BAD_GATEWAY;

                            parsed_cgi_output->headers[header_count].key[0] = '\0';
                            parsed_cgi_output->headers[header_count].value[0] = '\0';
                    }
                    else {
                        parsed_cgi_output->header_count ++;
                        header_count ++;
                    }
                    
                    if(cur_char == '\r') *parse_state = CGI_OUT_EXPECTING_LF;
                    else *parse_state = CGI_OUT_PARSING_NEW_LINE;
                } 

                else {
                    if(*pos >= MAX_HEADER_VALUE_SIZE) return HTTP_BAD_GATEWAY;

                    parsed_cgi_output->headers[header_count].value[*pos] = cur_char;
                    (*pos) ++;
                    
                }
                break;
                
            
            case CGI_OUT_EXPECTING_FINAL_LF:
                
                if(cur_char != '\n') return HTTP_BAD_GATEWAY;

                // Double end of line (\r\n\r\n) has been found 
                // Switching to parsing body 
                *pos = 0;
                if(parsed_cgi_output->body_len == 0){
                    *parsing_complete = true;
                    return HTTP_OK;
                }

                *parse_state = CGI_OUT_PARSING_BODY;
                break;
            

            case CGI_OUT_PARSING_BODY:
                if (*pos >= MAX_BODY_LEN - 1) return HTTP_BAD_GATEWAY;

                parsed_cgi_output->body[*pos] = cur_char;
                (*pos)++;

                if (*pos >= parsed_cgi_output->body_len) {
                    parsed_cgi_output->body[*pos] = '\0'; 
                    *parsing_complete = true;
                    *parse_state = CGI_OUT_END_PARSING;
                }
                break;

                
            case CGI_OUT_END_PARSING:
                *parsing_complete = true;
                return HTTP_OK;
            
            default:  // This case can't be met 
                return HTTP_INTERNAL_ERROR;
        }
    }

    if(*parse_state == CGI_OUT_END_PARSING){
        *parsing_complete = true;
        if(cfg_infos->verbose) print_debug("Parser - Done parsing\n");
    }
    else if(*parse_state == CGI_OUT_PARSING_BODY && *pos >= parsed_cgi_output->body_len){
        *parsing_complete = true;
        if(cfg_infos->verbose) print_debug("Parser - Done parsing\n");
    } else {
        if(cfg_infos->verbose) print_debug("Parser - Parsing body interupted, pos = %zu and bodylen = %zu\n", 
            *pos, parsed_cgi_output->body_len);
    }
    return HTTP_OK;
}