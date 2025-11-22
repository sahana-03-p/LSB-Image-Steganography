#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include <string.h>

/* Check operation type */
OperationType check_operation_type(char *argv[])
{
    if(strcmp(argv[1], "-e") == 0)      // Check if first argument is "-e" for encode
    {
        return e_encode;                // Return encode operation type
    }
    else if(strcmp(argv[1], "-d") == 0) // Check if first argument is "-d" for decode
    {
        return e_decode;                // Return decode operation type
    }
    else
    {
        return e_unsupported;           // Return unsupported for invalid operation
    }
}


int main(int argc, char *argv[])
{
    if(argc < 2)
    {
        printf("Error: Insufficient arguments\n");
        return 1;
    }

    EncodeInfo encInfo;  //structure variable for encoding operations
    
    int ret = check_operation_type(argv); 

    if(ret == 0)    // If operation is encoding (e_encode = 0)
    {
        if(argc >= 4)   // Check if minimum 4 arguments provided for encoding
        {
           /* Read and validate Encode args from argv */
           Status ret1 = read_and_validate_encode_args(argv, &encInfo);

           if(ret1 == e_failure)     // If argument validation failed
           {
                printf("Error: Invalid argument for encoding\n");      // Print error message
                return 0;
           }
           else     // If argument validation successful
           {
                 /* Perform the encoding */
                if(do_encoding(&encInfo) == e_success)      // Execute encoding process
                {
                    printf("ENCODING COMPLETED SUCCESSFULLY!\n");   
                }
                else                                       // If encoding failed
                {
                    printf("Encoding failed!\n");
                }
           }
        }
        else         // If insufficient arguments for encoding
        {
            printf("Error: Invalid argument for encoding\n");
        }
    }
    else if(ret == 1)        // If operation is decoding (e_decode = 1)
    {
        DecodeInfo decInfo;  // Structure variable for decoding operations

        if(argc >= 3)        // Check if minimum 3 arguments provided for decoding
        {
            /* Read and validate Decode args from argv */
            Status ret2 = read_and_validate_decode_args(argv, &decInfo);

            if(ret2 == e_failure)
            {
                printf("Error: Invalid argument for decoding\n");
                return 0;
            }
            else        // If argument validation successful
            {
                if(do_decoding(&decInfo) == e_success)
                {
                    printf("DECODING COMPLETED SUCCESSFULLY!\n");
                    return 0;
                }
                else
                {
                    printf("Decoding failed!\n");
                    return 1;
                }
            }
        }
        else       // If insufficient arguments for decoding
        {
            printf("Error: Invalid argument for decoding\n");
            return 1;
        }
    }
    else           // If operation is unsupported
    {
        //Error messages
        printf("Error: Unsupported operation\n");
        printf("Use -e for encoding or -d for decoding\n");
        return 0;
    }

    return 0;
}