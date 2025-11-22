#ifndef ENCODE_H
#define ENCODE_H
#include <stdio.h>
#include "types.h" // Contains user defined types

/* 
 * Structure to store information required for
 * encoding secret file to source Image
 * Info about output and intermediate data is
 * also stored
 */

#define MAX_SECRET_BUF_SIZE 1   //Process 1 byte of secret data at a time
#define MAX_IMAGE_BUF_SIZE (MAX_SECRET_BUF_SIZE * 8)    //Need 8 image bytes to store 1 secret byte (1 bit per image byte)
#define MAX_FILE_SUFFIX 4   //Maximum file extension length (".txt", ".c", etc.)

typedef struct _EncodeInfo
{
    /* Source Image info */
    char *src_image_fname;  // Pointer to filename string: "beautiful.bmp"
    FILE *fptr_src_image;   // File pointer to read source image
    uint image_capacity;    // Total bytes available: width × height × 3
    uint bits_per_pixel;    // Color depth (usually 24 for BMP)
    char image_data[MAX_IMAGE_BUF_SIZE];    // Buffer: stores 8 image bytes

    /* Secret File Info */
    char *secret_fname;     // Pointer to secret filename: "secret.txt"
    FILE *fptr_secret;      // File pointer to read secret file
    char extn_secret_file[MAX_FILE_SUFFIX];     // File extension: ".txt" 
    char secret_data[MAX_SECRET_BUF_SIZE];      // Buffer: stores 1 secret byte
    long size_secret_file;                      // Size of secret file in bytes

    /* Stego Image Info */
    char *stego_image_fname;        // Pointer to output filename: "stego.bmp"
    FILE *fptr_stego_image;         // File pointer to write stego image

} EncodeInfo;


/* Encoding function prototype */

/* Check operation type */
OperationType check_operation_type(char *argv[]);

/* Read and validate Encode args from argv */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo);

/* Perform the encoding */
Status do_encoding(EncodeInfo *encInfo);

/* Get File pointers for i/p and o/p files */
Status open_files(EncodeInfo *encInfo);

/* check capacity */
Status check_capacity(EncodeInfo *encInfo);

/* Get image size */
uint get_image_size_for_bmp(FILE *fptr_image);

/* Get file size */
uint get_file_size(FILE *fptr);

/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image);

/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo);

/* Encode extenstion size */
Status encode_secret_extn_file_size(int size, EncodeInfo *encInfo);

/* Encode secret file extenstion */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo);

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo);

/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo);

/* Encode int into LSB*/
Status encode_int_to_lsb(int size, char *image_buffer); 

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(char data, char *image_buffer); 

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest);

#endif
