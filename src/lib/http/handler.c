#include "handler.h"

extern char **environ;

// --- UTILS ---------------   

static const mime_type mime_types[] = {
    { "html", "text/html; charset=utf-8" },
    { "css",  "text/css" },
    { "js",   "application/javascript" },
    { "png",  "image/png" },
    { "jpg",  "image/jpeg" },
    { "ico",  "image/x-icon" },
    { NULL,   NULL }
};

char *get_mime_type(char *path){
    char *ext = strrchr(path, '.');
    if(!ext){
        return "application/octet-stream";
    }
    ext++;
    for (int i = 0; mime_types[i].ext != NULL; i++){
        if(strcmp(ext, mime_types[i].ext) == 0){
            return mime_types[i].mime;
        }
    }
    return "application/octet-stream";
}

static char *copy_file(FILE *stream){

    char cache;
    int len = 0;

    fseek(stream, 0, SEEK_SET);
    while(fscanf(stream, "%c", &cache) != EOF){
        len ++;
    }

    char *copy = (char*)malloc(sizeof(char) * len);
    if(!copy) return NULL;

    fseek(stream, 0, SEEK_SET);
    int cursor = 0;
    while(fscanf(stream, "%c", &cache) != EOF){
        copy[cursor] = cache;
        cursor ++;
    }

    copy[cursor] = '\0';

    return copy;
}

// --- HANDLE FUNCTIONS ---------------    


http_status handle_error(config_infos *cfg_infos, response *serv_resp, http_status err_status){

    if(cfg_infos->verbose) print_debug("Handler - Handling error\n");

    reset_response(serv_resp);

    if (init_response_status(serv_resp, err_status) != HTTP_OK) return HTTP_INTERNAL_ERROR;

    if (init_response_default_headers(serv_resp) != HTTP_OK) return HTTP_INTERNAL_ERROR;

    const http_reason_code *reason_code = get_http_reason(err_status);

    if (add_header(serv_resp, "Content-Type", "text/html") != HTTP_OK) return HTTP_INTERNAL_ERROR;

    if (add_header(serv_resp, "Connection", "close") != HTTP_OK) return HTTP_INTERNAL_ERROR;
    strlcpy(serv_resp->connection_type, "close", MAX_HEADER_VALUE_SIZE);

    if(err_status != HTTP_NO_CONTENT && err_status != HTTP_NOT_MODIFIED
        && reason_code->code / 100 != 1){
    
        char body[128];
        snprintf(body, sizeof(body), "<h1>%d - %s</h1>", reason_code->code, reason_code->reason);
        strcpy(serv_resp->body, body);

        serv_resp->body_len = strlen(serv_resp->body);
    }
    else {
        serv_resp->body_len = 0;
    }

    if (init_response_content_length(serv_resp) != HTTP_OK) return HTTP_INTERNAL_ERROR;

    return HTTP_OK;
}


http_status handle_get(config_infos *cfg_infos, request *client_req, response *serv_resp, bool head_only){

    if(!head_only){

        // read and copy files
        FILE *stream = fopen(client_req->path, "rb");

        if(stream == NULL) return handle_error(cfg_infos, serv_resp, HTTP_NOT_FOUND);

        char *copy = copy_file(stream);
        if(!copy) return handle_error(cfg_infos, serv_resp, HTTP_INTERNAL_ERROR);

        fclose(stream);
        
        
        // add body to response 
        strlcpy(serv_resp->body, copy, MAX_BODY_LEN);
        free(copy);
    }

    // add headers
    http_status cache_res;

    cache_res = init_response_status(serv_resp, HTTP_OK);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    cache_res = init_response_default_headers(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Content-Type", get_mime_type(client_req->path));
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    struct stat st;
    if(stat(client_req->path, &st) == -1){
        return handle_error(cfg_infos, serv_resp, HTTP_NOT_FOUND);
    }
    serv_resp->body_len = st.st_size;
    
    char last_modified[128];
    struct tm *tm_info = gmtime(&st.st_mtime);
    strftime(last_modified, sizeof(last_modified), "%a, %d %b %Y %H:%M:%S GMT", tm_info);
    cache_res = add_header(serv_resp, "Last-Modified", last_modified);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    if(strcasecmp(client_req->connection_type, "keep-alive") == 0){
        strlcpy(serv_resp->connection_type, "keep-alive", MAX_HEADER_VALUE_SIZE);
    } else {
        strlcpy(serv_resp->connection_type, "close", MAX_HEADER_VALUE_SIZE);
    }
    cache_res = add_header(serv_resp, "Connection", serv_resp->connection_type);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    cache_res = init_response_content_length(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    return HTTP_OK;
}


http_status handle_options(config_infos *cfg_infos, response *serv_resp){

    http_status cache_res;

    cache_res = init_response_status(serv_resp, HTTP_OK);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    cache_res = init_response_default_headers(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    cache_res = add_header(serv_resp, "Allow", ALLOWED_METHODS);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    serv_resp->body_len = 0;
    cache_res = init_response_content_length(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    return HTTP_OK;
}


http_status handle_cgi(config_infos *cfg_infos, request *client_req, response *serv_resp){

    // Asserts method is GET or POST
    if(strcmp(client_req->method, "GET") != 0 && strcmp(client_req->method, "POST") != 0){
        return handle_error(cfg_infos, serv_resp, HTTP_METHOD_NOT_ALLOWED);
    }

    // ----- Setup environment variables -------------------------------------------------------

    *environ = NULL;

    // Default variables
    setenv("REQUEST_METHOD", client_req->method, 1);              
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("PATH", "/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin", 1);

    char len_str[32];
    snprintf(len_str, sizeof(len_str), "%zu", client_req->body_len);
    setenv("CONTENT_LENGTH", len_str, 1);

    char exec_path[MAX_PATH_LEN];
    char query[MAX_PATH_LEN];
    if(strcmp(client_req->method, "GET") == 0){
        
        if (sscanf(client_req->path, "%[^?]?%s", exec_path, query) < 1) {
            strncpy(exec_path, client_req->path, MAX_PATH_LEN);
        }

        setenv("SCRIPT_FILENAME", exec_path, 1);
        setenv("QUERY_STRING", query, 1);

    } else {
        strncpy(exec_path, client_req->path, MAX_PATH_LEN);
        strncpy(query, "", MAX_PATH_LEN);
        setenv("SCRIPT_FILENAME", exec_path, 1);
        setenv("QUERY_STRING", "", 1);
    }

    // Client headers 
    for (int i = 0; i < client_req->header_count; i++){

        header hd = client_req->headers[i];
        
        // Name exception for content-type 
        if(strcasecmp(hd.key, "Content-Type") == 0){
            setenv("CONTENT_TYPE", hd.value, 1);
        }

        // Other headers
        else if(strcasecmp(hd.key, "Content-Length") != 0){

            char maj_hd_key[MAX_HEADER_KEY_SIZE];
            for (size_t j = 0; j < strlen(hd.key) && j < MAX_HEADER_KEY_SIZE - 1; j++){
                if (hd.key[j] == '-') maj_hd_key[j] = '_'; 
                else maj_hd_key[j] = toupper((unsigned char)hd.key[j]);
                maj_hd_key[j+1] = '\0'; 
            }

            char env_hd_key[sizeof(maj_hd_key) + 5];
            snprintf(env_hd_key, sizeof(maj_hd_key) + 5, "HTTP_%s", maj_hd_key);

            setenv(env_hd_key, hd.value, 1);
        }
    }


    // ----- Pipe and fork -------------------------------------------------------
    int pipe_in[2], pipe_out[2];

    if( pipe(pipe_in) < 0 || pipe(pipe_out) < 0) return handle_error(cfg_infos, serv_resp, HTTP_INTERNAL_ERROR);

    pid_t pid = fork();
    if(pid < 0) return handle_error(cfg_infos, serv_resp, HTTP_INTERNAL_ERROR);

    if(pid == 0){   // Child process    

        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        //dup2(pipe_out[1], STDERR_FILENO);

        close(pipe_in[0]); close(pipe_in[1]);
        close(pipe_out[0]); close(pipe_out[1]);
        
        execl(exec_path, exec_path, (char *)NULL);

        perror("CGI execl failed");
        fprintf(stderr, "path: %s\n", exec_path);
        fprintf(stderr, "query: %s\n", query);
        _exit(EXIT_FAILURE);
    }

    // Server 
    close(pipe_in[0]);
    close(pipe_out[1]);

    if(client_req->body_len > 0 && strcmp(client_req->method, "POST") == 0){
        write(pipe_in[1], client_req->body, client_req->body_len);
    }
    close(pipe_in[1]);

    // ----- Read response -------------------------------------------------------

    bool parsing_complete = false;

    http_status parse_res = HTTP_OK;
    parsing_response_state parse_state = RESP_PARSING_NEW_LINE;
    size_t total_bytes_parsed = 0;
    size_t pos = 0;
    bool has_body_len = false; 

    r_buffer *raw_response_buf = init_ring_buffer(2 * MAX_REQUEST_LEN + 1);
    char tmp_read_buf[4096]; 

    reset_response(serv_resp);
    serv_resp->body_len = 0;
    serv_resp->header_count = 0;

    while (!parsing_complete) {

        ssize_t bytes_read = read(pipe_out[0], tmp_read_buf, sizeof(tmp_read_buf));

        if (bytes_read < 0) {
            if (errno == EINTR) continue;
            parse_res = HTTP_BAD_GATEWAY;
            break;
        }

        if (bytes_read == 0) {
            // <=> EOF
            if (parse_state == RESP_PARSING_BODY && !has_body_len) {
                // Done if unknown body length
                parsing_complete = true;
                parse_res = HTTP_OK;
            } else if (!parsing_complete) {
                // Script interupted connection 
                parse_res = HTTP_BAD_GATEWAY;
            }
            break;
        }

        // Write received data in ring buffer 
        if (write_string_in_r_buffer(raw_response_buf, tmp_read_buf, bytes_read) != 0) {
            parse_res = HTTP_INTERNAL_ERROR;
            break;
        }

        // Parsing through all buffer 
        parse_res = parse_raw_cgi_response(cfg_infos, raw_response_buf, serv_resp, 
                        bytes_read, &total_bytes_parsed, &pos,
                        &parsing_complete, &parse_state, &has_body_len);

        if (parse_res != HTTP_OK) break;
    }

    close(pipe_out[0]);

    // Error while parsing 
    if (parse_res != HTTP_OK) {
        if(cfg_infos->verbose) print_debug("Handler - Error while parsing CGI response\n");
        return handle_error(cfg_infos, serv_resp, parse_res);
    }

    if(cfg_infos->verbose){
        print_debug("Handler - Parsed CGI response :\n");
        print_response(serv_resp);
    }

    // ----- Building response -------------------------------------------------------
    
    http_status cache_res;

    int code = atoi(serv_resp->code);
    cache_res = get_status_from_code(code);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    if(strlen(HTTP_VERSION) > MAX_VERSION_LEN) return handle_error(cfg_infos, serv_resp, HTTP_INTERNAL_ERROR);
    strcpy(serv_resp->version, HTTP_VERSION);

    cache_res = init_response_default_headers(serv_resp);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    if(strcasecmp(client_req->connection_type, "keep-alive") == 0){
        strlcpy(serv_resp->connection_type, "keep-alive", MAX_HEADER_VALUE_SIZE);
    } else {
        strlcpy(serv_resp->connection_type, "close", MAX_HEADER_VALUE_SIZE);
    }
    cache_res = add_header(serv_resp, "Connection", serv_resp->connection_type);
    if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);

    if(!has_body_len){
        cache_res = init_response_content_length(serv_resp);
        if(cache_res != HTTP_OK) return handle_error(cfg_infos, serv_resp, cache_res);
    }    

    return HTTP_OK;
}