#ifndef RING_BUFFER
#define RING_BUFFER

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct ring_buffer_s {

    char *buf;
    size_t read_pos;
    size_t write_pos;
    size_t buf_size;

} ring_buffer_t ; 

/** 
 * @brief Initializes an empty ring buffer.
 * @param buf_size Desired ring buffer size.
 * @return A pointer to a new ring buffer of size `buf_size`, or NULL on error. 
 * */
ring_buffer_t *init_ring_buffer(size_t buf_size);

/**
 * @brief Frees a ring buffer and its components.
 */
void free_ring_buffer(ring_buffer_t *r_buf);

// returns true if argument is NULL
bool r_buffer_is_empty(ring_buffer_t *r_buf);

// returns true if argument is NULL
bool r_buffer_is_full(ring_buffer_t *r_buf);

/**
 * @return Returns the buffer free space, or 0 if buffer is NULL.
 */
size_t get_r_buffer_free_space(ring_buffer_t *r_buf);

/**
 * @brief Attemps to read a character from a ring buffer. Once read, the character is deleted. 
 * @param r_buf   Pointer to the ring buffer.
 * @param target  Pointer to a char that will receive read character value.
 * @return 0 on success, -1 otherwise.
 */
int read_from_r_buffer(ring_buffer_t *r_buf, char *target);


/**
 * @brief Copies a string of characters in a ring buffer.
 * @param r_buf  Pointer to the ring buffer.
 * @param s      The array to copy.
 * @param s_len  The number of characters to copy.
 */
int write_string_in_r_buffer(ring_buffer_t *r_buf, char *s, size_t s_len);


#endif