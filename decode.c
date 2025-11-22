#include <stdio.h>
#include "decode.h"
#include "types.h"
#include <string.h>
#include "common.h"


char str[50];

/* Function Definitions */

/* Read and validate decode args from argv */
Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    // Check for stego image file
    if(argv[2][0] != '.')   // Ensure filename doesn't start with dot
    {
        if(strstr(argv[2], ".bmp") != NULL) // Verify it's a BMP file
        {
            decInfo -> dest_image_fname = argv [2];   // Store stego image filename
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

    //check for output file 
    if(argv[3] == NULL)     // If no output filename provided
    {
        decInfo -> output_fname = "output";     // Use default output name
    }
    else
    {
        decInfo -> output_fname = argv[3];      // Store provided output filename
    }

    return e_success;
}

/* Get File pointers for i/p and o/p files */
Status open_files_for_decoding(DecodeInfo *decInfo)
{
    decInfo -> fptr_dest_image = fopen(decInfo -> dest_image_fname, "r");   // Open stego image for reading

    // Do Error handling
    if (decInfo -> fptr_dest_image == NULL)
    {
    	perror("fopen");
    	fprintf(stderr, "ERROR: Unable to open file %s\n", decInfo -> dest_image_fname);

    	return e_failure;
    }

    return e_success;
}

/* Skip bmp image header */
Status skip_bmp_header(FILE *fptr_dest_image)
{
    if(fseek(fptr_dest_image, 54, SEEK_SET) != 0)      // Skip first 54 bytes (BMP header)
    {
        printf("Error: Failed to skip BMP header\n");
        return e_failure;
    }
    return e_success;
}

/* Decode byte from LSB*/
Status decode_byte_from_lsb(char *data, char *image_buffer)  
{
    char decoded_byte = 0;
    char bit_data;

    for(int i = 7; i >= 0; i--)
    {
        bit_data = image_buffer[7 - i] & 1;  // Get the LSB from image byte
        decoded_byte = decoded_byte | (bit_data << i);
    }

    *data = decoded_byte;   // Store decoded byte in output parameter  
    return e_success; 
}

/* Decode int from LSB*/
Status decode_int_from_lsb(int *size, char *image_buffer)  
{
    int decoded_size = 0;
    char bit_data;

    for(int i = 31; i >= 0; i--)
    {
        bit_data = image_buffer[31 - i] & 1;    // Get the LSB
        decoded_size = decoded_size | (bit_data << i);
    }

    *size = decoded_size;    
    return e_success; 
}

/* Store Magic String */
Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char arr[8];
    char decoded_char;

    //Run the loop strlen(magic_string) times
    for(int i = 0; i < strlen(magic_string); i++)   // Process each character in magic string
    {
        //Read the 8byte of data from src file
        fread(arr, 1, 8, decInfo -> fptr_dest_image);

        /* Decode a byte from LSB of image data */
        if((decode_byte_from_lsb(&decoded_char, arr)) == e_success)  
        {
            //Verify the decoded character matches magic string
            if(decoded_char == magic_string[i])
            {
                continue;
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

/* Decode secret file extension size */
Status decode_secret_file_extn_size(int *size, DecodeInfo *decInfo)  
{
    char arr[32];       // Buffer for 32 image bytes

    fread(arr, 1, 32, decInfo->fptr_dest_image);   // Read 32 bytes for extension size

    if((decode_int_from_lsb(size, arr)) == e_success)  
    {
        return e_success;
    } 
    else
    {
        return e_failure;
    }
}

/* Decode secret file extension */
Status decode_secret_file_extn(int file_extn, DecodeInfo *decInfo)
{
    char arr[8];
    char decoded_char;

    // Process each character in extension
    for(int i = 0; i < file_extn; i++)
    {
        fread(arr, 1, 8, decInfo->fptr_dest_image);  // Read 8 bytes for one character

        if((decode_byte_from_lsb(&decoded_char, arr)) == e_success) // Decode one character
        {
            decInfo -> extn_output_file[i] = decoded_char;  
            
            if(decoded_char == '\0')      // Check for null terminator
                break;
        }
        else
        {
            return e_failure;
        }
    }

    decInfo -> extn_output_file[file_extn] = '\0';
    
    int i=0;    // Index for filename processing
    while(decInfo -> output_fname[i])   // Process each character in output filename
    {
        if(decInfo -> output_fname[i] != '.')   // Check if character is not a dot
        {
            str[i] = decInfo -> output_fname[i];
        }
        else
        {
            break;  // Stop at first dot
        }
        i++;       // Move to next character
    }

    str[i] = '\0';

    strcat(str, decInfo -> extn_output_file);      // Append decoded file extension to base filename
    decInfo -> output_fname = str;                // Update output filename with full name
    printf("--%s\n",decInfo -> output_fname);   // Print final output filename
    return e_success;
}

/* Decode secret file size */
Status decode_secret_file_size(int *file_size, DecodeInfo *decInfo)
{
    char arr[32];

    fread(arr, 1, 32, decInfo->fptr_dest_image);    // Read 32 bytes for file size

    if((decode_int_from_lsb(file_size, arr)) == e_success)  // Decode file size as integer
    {
        decInfo-> size_output_file = (*file_size);
        return e_success;
    }
    return e_failure;
}

/* Decode secret file data*/
Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char arr[8];
    char decoded_char;

    // Open output file for writing
    decInfo->fptr_output = fopen(decInfo->output_fname, "w");   // Open output file
    if(decInfo->fptr_output == NULL)
    {
        return e_failure;
    }

    // Process each byte of secret data
    for(int i = 0; i < decInfo->size_output_file; i++)
    {
        fread(arr, 1, 8, decInfo->fptr_dest_image); // Read 8 bytes for one secret byte

        if((decode_byte_from_lsb(&decoded_char, arr)) == e_success) // Decode one character  
        {
            fwrite(&decoded_char, 1, 1, decInfo->fptr_output);  // Write character to output file
        }
        else
        {
            fclose(decInfo->fptr_output);
            return e_failure;
        }
    }
    
    fclose(decInfo->fptr_output);
    return e_success;
}

/* Perform the decoding */
Status do_decoding(DecodeInfo *decInfo)
{
    int extn_size;  
    int file_size;

    /* Get File pointers for i/p files */
    if((open_files_for_decoding(decInfo)) == e_success)
    {
        printf("Stego image file opened successfully\n");

        /* Skip bmp image header */
        if((skip_bmp_header(decInfo -> fptr_dest_image)) == e_success)
        {
            printf("BMP header skipped\n");

            /* Decode Magic String */
            if((decode_magic_string(MAGIC_STRING, decInfo)) == e_success)
            {
                printf("Magic string verified\n");

                /* Decode secret file extension size */
                if((decode_secret_file_extn_size(&extn_size, decInfo)) == e_success)  
                {
                    printf("Secret file extension size decoded: %d\n", extn_size);

                    /* Decode secret file extension */
                    if((decode_secret_file_extn(extn_size, decInfo)) == e_success)
                    {
                        printf("Secret file extension decoded: %s\n", decInfo->extn_output_file);

                        /* Decode secret file size */
                        if((decode_secret_file_size(&file_size, decInfo)) == e_success)
                        {
                            printf("Secret file size decoded: %ld\n", decInfo->size_output_file);
                                
                            /* Decode secret file data */
                            if((decode_secret_file_data(decInfo)) == e_success)
                            {
                                printf("Secret file data decoded successfully\n");

                                return e_success;
                            }
                        }
                    }
                }
            }
        }
    }

    return e_failure;
}