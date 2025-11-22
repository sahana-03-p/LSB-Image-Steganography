#include <stdio.h>
#include "encode.h"
#include "types.h"
#include<string.h>
#include "common.h"

/* Function Definitions */

/* Get image size
 * Input: Image file ptr
 * Output: width * height * bytes per pixel (3 in our case)
 * Description: In BMP Image, width is stored in offset 18,
 * and height after that. size is 4 bytes
 */
uint get_image_size_for_bmp(FILE *fptr_image)
{
    uint width, height;
    // Seek to 18th byte
    fseek(fptr_image, 18, SEEK_SET);

    // Read the width (an int)
    fread(&width, sizeof(int), 1, fptr_image);
    printf("width = %u\n", width);

    // Read the height (an int)
    fread(&height, sizeof(int), 1, fptr_image);
    printf("height = %u\n", height);

    // Return image capacity
    return width * height * 3;
}

/* 
 * Get File pointers for i/p and o/p files
 * Inputs: Src Image file, Secret file and
 * Stego Image file
 * Output: FILE pointer for above files
 * Return Value: e_success or e_failure, on file errors
 */
Status open_files(EncodeInfo *encInfo)
{
    // Src Image file
    encInfo->fptr_src_image = fopen(encInfo->src_image_fname, "r");
    
    // Do Error handling
    if (encInfo->fptr_src_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->src_image_fname);

    	return e_failure;
    }

    // Secret file
    encInfo->fptr_secret = fopen(encInfo->secret_fname, "r");
    
    // Do Error handling
    if (encInfo->fptr_secret == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->secret_fname);

    	return e_failure;
    }

    // Stego Image file
    encInfo->fptr_stego_image = fopen(encInfo->stego_image_fname, "w");
   
    // Do Error handling
    if (encInfo->fptr_stego_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", encInfo->stego_image_fname);

    	return e_failure;
    }

    // No failure return e_success
    return e_success;
}

/* Read and validate Encode args from argv */
Status read_and_validate_encode_args(char *argv[], EncodeInfo *encInfo)
{
    // Validate source image filename format
    if(argv[2][0] != '.')   // Ensure filename doesn't start with dot
    {
        if(strstr(argv[2], ".bmp"))  // Check if file has .bmp extension
        {
            encInfo -> src_image_fname = argv[2]; // Store source image filename
        }
        else
        {
            return e_failure;
        }
    }
    else
    {
        return e_failure;
    }

    // Validate secret file filename and format
    if(argv[3][0] != '.')       // Ensure filename doesn't start with dot
    {
        // Check for supported secret file extensions
        if(strstr(argv[3], ".txt") || strstr(argv[3], ".c") || strstr(argv[3], ".sh") || strstr(argv[3], ".h"))  
        {
            encInfo -> secret_fname = argv[3]; // Store secret filename
        }
        else
        {
            return e_failure;
        }
    }
    else
    {
        return e_failure;
    }

    // Handle output filename (optional argument)
    if(argv[4] == NULL)
    {
        encInfo -> stego_image_fname = "default.bmp"; // Use default output name
    }
    else
    {
        if(argv[4][0] != '.')   // Validate output filename format
        {
            if(strstr(argv[4], ".bmp"))  // Check for .bmp extension
            {   
                encInfo -> stego_image_fname = argv[4]; // Store output filename
            }
            else
            {
                return e_failure;
            }
        }
        else
        {
            return e_failure;
        }
    }

    return e_success;
}

/* Copy bmp image header */
Status copy_bmp_header(FILE *fptr_src_image, FILE *fptr_dest_image)
{
    unsigned char header[54];       // Buffer for 54-byte BMP header

    // Reset source file pointer to beginning
    rewind(fptr_src_image);

    // Read complete BMP header from source image
    fread(header, 1, 54, fptr_src_image);
    // Write header to destination image
    fwrite(header, 1, 54, fptr_dest_image);
    
    // Verify both files are at same position after copy
    if(ftell(fptr_src_image) == ftell(fptr_dest_image)) 
    {
        return e_success;
    }
    else
    {
        return e_failure;
    }
}

/* Get size of any file in bytes */
uint get_file_size(FILE *fptr)
{
    fseek(fptr, 0, SEEK_END);    // Move to end of file
    uint size = ftell(fptr);    
    return size;                 // Return file size in bytes
}

/* check capacity */
Status check_capacity(EncodeInfo *encInfo)
{
    uint size = get_image_size_for_bmp(encInfo->fptr_src_image);
    encInfo -> size_secret_file = get_file_size(encInfo -> fptr_secret);

    // Calculate if image can hold magic string + extension + file size + secret data
    if(size > ((strlen(MAGIC_STRING) + MAX_FILE_SUFFIX + sizeof(encInfo -> extn_secret_file) + sizeof(encInfo -> size_secret_file) + encInfo -> size_secret_file ) * 8) + 54)
    {
        return e_success;
    }

    return e_failure;
}

/* Encode a byte into LSB of image data array */
Status encode_byte_to_lsb(char data, char *image_buffer)
{
    char bit_data;

    // Process each bit from MSB to LSB
    for(int i = 7; i >= 0; i--)
    {
        bit_data = ((data >> i) & 1);                           //Get the bit
        image_buffer[7 - i] = image_buffer[7 - i] & (~1);       //clear the bit
        image_buffer[7 - i] = image_buffer[7 - i] | bit_data;   //set the bit
    }

    return e_success; 
}

/* Store Magic String */
Status encode_magic_string(const char *magic_string, EncodeInfo *encInfo)
{
    //Declare array of size 8
    char arr[8];

    // Process each character in magic string
    for(int i = 0; i < strlen(magic_string); i++)
    {
        // Read 8 image bytes for encoding one character
        fread(arr, 1, 8, encInfo -> fptr_src_image);

        /* Encode a byte into LSB of image data array */
        if((encode_byte_to_lsb(magic_string[i], arr)) == e_success)
        {
            // Write modified image bytes to output file
            fwrite(arr, 1, 8, encInfo -> fptr_stego_image);
        }
        else
        {
            return e_failure;
        }
    }
    return e_success; 
}

/* Encode function, which does the real encoding */
Status encode_int_to_lsb(int size, char *image_buffer) //collecting 32 bytes of data
{
    char bit_data;

    // Process each of 32 bits from MSB to LSB
    for(int i = 31; i >= 0; i--)
    {
        bit_data = ((size >> i) & 1);                             //Get the bit
        image_buffer[31 - i] = image_buffer[31 - i] & (~1);      //clear the bit
        image_buffer[31 - i] = image_buffer[31 - i] | bit_data; //set the bit
    }

    return e_success; 
}

/* Encode extenstion size */
Status encode_secret_extn_file_size(int size, EncodeInfo *encInfo)
{
    //Declare the array with size 32
    char arr[32];

    strcpy(encInfo->extn_secret_file, strstr(encInfo->secret_fname,".")); 

    //Read 32 byte of data from src file
    fread(arr, 1, 32, encInfo -> fptr_src_image);

    if((encode_int_to_lsb(strlen(encInfo -> extn_secret_file), arr)) == e_success)
    {
        //write  the 32 byte data into dest file
        fwrite(arr, 1, 32, encInfo->fptr_stego_image);
        return e_success;
    } 

    return e_failure;
}

/* Encode secret file extenstion */
Status encode_secret_file_extn(const char *file_extn, EncodeInfo *encInfo)
{
    //Declare array of size 8
    char arr[8];

    // Process each character in file extension
    for(int i = 0; i < strlen(file_extn); i++)
    {
        //Read the 8byte of data from src file
        fread(arr, 1, 8, encInfo -> fptr_src_image);

        /* Encode a byte into LSB of image data array */
        if((encode_byte_to_lsb(file_extn[i], arr)) == e_success)
        {
            //Write the 8byted data to destination
            fwrite(arr, 1, 8, encInfo -> fptr_stego_image);
        } 
        else
        {
            return e_failure;
        }
    }
    return e_success;
}

/* Encode secret file size */
Status encode_secret_file_size(long file_size, EncodeInfo *encInfo)
{
    //Declare the array with size 32
    char arr[32];

    //Read 32 byte of data from src file
    fread(arr, 1, 32, encInfo->fptr_src_image);

    // Encode file size as 32-bit integer
    if((encode_int_to_lsb(file_size, arr)) == e_success)
    {
        //write  the 32 byte data into dest file
        fwrite(arr, 1, 32, encInfo->fptr_stego_image);
        return e_success;
    }
    return e_failure;
}

/* Encode secret file data*/
Status encode_secret_file_data(EncodeInfo *encInfo)
{
    char arr[8];    // Buffer for 8 image bytes
    char ch;        // Buffer for one secret data byte

    // Reset secret file pointer to beginning
    rewind(encInfo -> fptr_secret);

    // Process each byte of secret file data
    for(int i = 0; i < encInfo -> size_secret_file; i++)
    {
        // Read one byte from secret file
        fread(&ch, 1, 1, encInfo -> fptr_secret);
        
        // Read 8 image bytes for encoding one secret byte
        fread(arr, 1, 8, encInfo -> fptr_src_image);

        /* Encode a byte into LSB of image data array */
        if((encode_byte_to_lsb(ch, arr)) == e_success)
        {
            // Write modified image bytes to output file
            fwrite(arr, 1, 8, encInfo -> fptr_stego_image);
        }
        else
        {
            return e_failure;
        }
    }
    return e_success;  
}

/* Copy remaining image bytes from src to stego image after encoding */
Status copy_remaining_img_data(FILE *fptr_src, FILE *fptr_dest)
{ 
   char ch;   // Buffer for one byte

   // Copy all remaining bytes from source to destination
   while(fread(&ch, 1, 1, fptr_src))
   {
        fwrite(&ch, 1, 1, fptr_dest);   // Write byte to destination
   }

   return e_success;
}

/* Perform the encoding */
Status do_encoding(EncodeInfo *encInfo)
{
    /* Get File pointers for i/p and o/p files */
    if((open_files(encInfo)) == e_success)
    {
        printf("Opening Files Done...\n");
        
        // Initialize file information
        encInfo->size_secret_file = get_file_size(encInfo->fptr_secret); 
        
        printf("Secret file size: %ld bytes\n", encInfo->size_secret_file);
        
        if((check_capacity(encInfo)) == e_success)
        {
            printf("Checking the capacity done...\n");
            /* Copy bmp image header */
            if((copy_bmp_header(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)) == e_success)
            {
                printf("Header Copied Successfully...\n");
                /* Store Magic String */
                if((encode_magic_string(MAGIC_STRING, encInfo)) == e_success)
                {
                    printf("Encoded Magic string Successfully...\n");
                    /* Encode extenstion size */
                    if((encode_secret_extn_file_size(MAX_FILE_SUFFIX, encInfo)) == e_success)
                    {
                        printf("Encoded secret File extention Size Successfully...\n");
                        /* Encode secret file extenstion */
                        if((encode_secret_file_extn(encInfo -> extn_secret_file, encInfo)) == e_success)
                        {
                            printf("Encoded secret File extention Successfully...\n");
                            /* Encode secret file size */
                            if((encode_secret_file_size(encInfo -> size_secret_file, encInfo)) == e_success)
                            {
                                printf("Encoded secret File Size Successfully...\n");
                                /* Encode secret file data*/
                                if((encode_secret_file_data(encInfo)) == e_success)
                                {
                                    printf("Encoded secret File data Successfully...\n");
                                    if((copy_remaining_img_data(encInfo -> fptr_src_image, encInfo -> fptr_stego_image)) == e_success)
                                    {                                    
                                        return e_success; 
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            printf("Capacity check failed\n");
        }
    }
    return e_failure;
}