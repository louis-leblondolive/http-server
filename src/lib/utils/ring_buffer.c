#include "ring_buffer.h"


r_buffer *init_ring_buffer(size_t buf_size){

    r_buffer *r_buf = (r_buffer*)malloc(sizeof(r_buffer));
    if(!r_buf) return NULL;

    r_buf->buf = (char*)malloc(buf_size);
    if(!r_buf->buf){
        free(r_buf);
        return NULL;
    }

    r_buf->buf_size = buf_size;
    r_buf->read_pos = 0;
    r_buf->write_pos = 0;

    return r_buf;
}


void free_ring_buffer(r_buffer *r_buf){
    if(!r_buf) return;
    free(r_buf->buf);
    free(r_buf);
    return ;
}


bool r_buffer_is_empty(r_buffer *r_buf){
    if (!r_buf) return true;
    return r_buf->read_pos == r_buf->write_pos;
}

bool r_buffer_is_full(r_buffer *r_buf){
    if (!r_buf) return true;
    return (r_buf->write_pos + 1) % r_buf->buf_size == r_buf->read_pos;
}


size_t get_r_buffer_free_space(r_buffer *r_buf){

    if (!r_buf) return 0;

    if(r_buf->write_pos >= r_buf->read_pos){
        return (r_buf->buf_size - 1) - (r_buf->write_pos - r_buf->read_pos);
    } else {
        return (r_buf->read_pos - r_buf->write_pos) - 1;
    }

}


int read_from_r_buffer(r_buffer *r_buf, char *target){

    if (!r_buf) return -1;
    if (!target) return -1;

    if(r_buffer_is_empty(r_buf)){    // buffer is empty 
        return -1; 
    }

    *target = r_buf->buf[r_buf->read_pos];
    r_buf->read_pos = (r_buf->read_pos + 1) % r_buf->buf_size;

    return 0;
}


static int write_char_in_r_buffer(r_buffer *r_buf, char c){

    if (!r_buf) return -1;

    if(r_buffer_is_full(r_buf)){    // buffer is full
        return -1;
    }

    r_buf->buf[r_buf->write_pos] = c;
    r_buf->write_pos = (r_buf->write_pos + 1) % r_buf->buf_size;

    return 0; 
}


int write_string_in_r_buffer(r_buffer *r_buf, char *s, size_t s_len){

    if (!r_buf) return -1;
    if (!s) return -1;

    if(s_len > get_r_buffer_free_space(r_buf)){
        return -1;
    }

    for (size_t i = 0; i < s_len; i++){
        if(write_char_in_r_buffer(r_buf, s[i]) == -1) return -1;
    }
    
    return 0;
}