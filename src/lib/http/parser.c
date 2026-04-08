#include "parser.h"

// attention il va falloir gérer le fait que le curseur peut être plus grand que le nombre de bytes reçus !


void print_request(request *r){
    printf("%s %s %s\n", r->method, r->path, r->version);
    for (int i = 0; i < r->header_count; i++){
        printf("%s: %s\n", r->headers[i].key, r->headers[i].value);
    }
    printf("\n");
    printf("%s\n", r->body);
}


void raise_syntax_error(void){
    printf("mettre une erreur ici qui arrête le truc\n");
}

void check_cursor(int cursor){
    if(cursor >= MAX_REQUEST_LEN) raise_syntax_error();
}

void check_position(int pos, int bound){
    if(pos >= bound) raise_syntax_error();
    
}

void move_to_next_line(char *raw_request, int bytes_received, int *cursor, bool *seen_separator){
        
        char_expect expected_char = EXPECTING_CR;

        while(*cursor < MAX_REQUEST_LEN){

            switch (expected_char)
            {
            case EXPECTING_CR:
                if(raw_request[*cursor] == '\r'){
                    expected_char = EXPECTING_LF; 
                    (*cursor)++;
                }
                else raise_syntax_error();
                break;
            
            case EXPECTING_LF:
                if(raw_request[*cursor] == '\n'){
                    expected_char = ANY;
                    (*cursor)++;
                }
                else raise_syntax_error();
                break;
            
            case ANY:
                if(raw_request[*cursor] == '\r'){
                    expected_char = EXPECTING_FINAL_LF;
                    (*cursor)++;
                }
                else{
                    return;
                }

            case EXPECTING_FINAL_LF:
                if(raw_request[*cursor] == '\n'){
                    (*cursor)++;
                    *seen_separator = true;
                    return; 
                }
            
            default:
                break;
            }
        }
}

request *parse_raw_request(char *raw_request, int bytes_received){

    request *client_req = (request*)malloc(sizeof(request));
    parsing_state current_state = PARSING_METHOD;
    
    int cursor = 0;
    int pos = 0;
    int header_count = 0;
    bool seen_separator = false;


    while(cursor < bytes_received){

        switch (current_state){

        case PARSING_METHOD:

            check_cursor(cursor);

            if(raw_request[cursor] == ' '){

                while(cursor < MAX_REQUEST_LEN && raw_request[cursor] == ' ') cursor ++;

                client_req->path[pos] = '\0';
                pos = 0;
                current_state = PARSING_PATH;

            } else {
                check_position(pos, MAX_METHOD_LEN);

                client_req->method[pos] = raw_request[cursor];
                pos ++;
                cursor ++;
            }
            break;
            

        case PARSING_PATH:

            check_cursor(cursor);

            if(raw_request[cursor] == ' '){

                while(cursor < MAX_REQUEST_LEN && raw_request[cursor] == ' ') cursor ++;

                client_req->path[pos] = '\0';
                pos = 0;
                current_state = PARSING_VERSION;

            } else {
                check_position(pos, MAX_PATH_LEN);

                client_req->path[pos] = raw_request[cursor];
                pos ++;
                cursor ++;
            }
            break;
        

        case PARSING_VERSION:

            check_cursor(cursor);

            if(raw_request[cursor] == '\r'){

                client_req->version[pos] = '\0';
                pos = 0;

                move_to_next_line(raw_request, bytes_received, &cursor, &seen_separator);
                
                if(seen_separator){
                    current_state = PARSING_BODY;
                } else {
                    current_state = PARSING_HEADER_KEY;
                }
            }
            else {
                check_position(pos, MAX_VERSION_LEN);

                client_req->version[pos] = raw_request[cursor];
                pos ++;
                cursor ++;
            }
            break;


        case PARSING_HEADER_KEY:

            check_cursor(cursor);

            if(raw_request[cursor] == ':'){

                cursor ++;
                while(cursor < MAX_REQUEST_LEN && raw_request[cursor] == ' ') cursor ++;

                client_req->headers[header_count].key[pos] = '\0';
                pos = 0;
                current_state = PARSING_HEADER_VALUE;
            }
            else{
                check_position(pos, MAX_HEADER_KEY_SIZE);
                
                client_req->headers[header_count].key[pos] = raw_request[cursor];
                pos ++;
                cursor ++;
            } 
            break;

        case PARSING_HEADER_VALUE:

            check_cursor(cursor);

            if(raw_request[cursor] == '\r'){

                client_req->headers[header_count].value[pos] = '\0';
                pos = 0;

                move_to_next_line(raw_request, bytes_received, &cursor, &seen_separator);
                
                if(seen_separator){
                    current_state = PARSING_BODY;
                } else {
                    current_state = PARSING_HEADER_KEY;
                }

                header_count ++;
            }
            else {
                check_position(pos, MAX_HEADER_VALUE_SIZE);

                client_req->headers[header_count].value[pos] = raw_request[cursor];
                pos ++;
                cursor ++;
            }
            
            break;

        case PARSING_BODY:
            
            check_cursor(cursor);
            check_position(pos, MAX_BODY_LEN);

            client_req->body[pos] = raw_request[cursor];
            pos ++;
            cursor ++;
            break;

        default:
            break;
        }
    }

    client_req->body[pos] = '\0';
    client_req->header_count = header_count;


    return client_req;
}