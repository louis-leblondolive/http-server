#ifndef RING_BUFFER
#define RING_BUFFER

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct Ring_buffer {

    char *buf;
    size_t read_pos;
    size_t write_pos;
    size_t buf_size;

} r_buffer ; 


r_buffer *init_ring_buffer(size_t buf_size);
void free_ring_buffer(r_buffer *r_buf);
bool r_buffer_is_empty(r_buffer *r_buf);
bool r_buffer_is_full(r_buffer *r_buf);
size_t get_r_buffer_free_space(r_buffer *r_buf);
int read_from_r_buffer(r_buffer *r_buf, char *target);
int write_in_r_buffer(r_buffer *r_buf);
int write_string_in_r_buffer(r_buffer *r_buf, char *s, size_t s_len);


#endif