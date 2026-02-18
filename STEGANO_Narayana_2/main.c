/* Jakka Narayana
Date of submission : 16/02/2026
Summary :
         This project implements Image Steganography using the Least Significant Bit (LSB) technique in C programming. 
 It hides a secret file inside a BMP image without visibly changing the image quality. 
 The encoding process embeds the secret data into the image pixels, while the decoding process extracts the hidden data from the stego image.
 This project demonstrates concepts of file handling, bit manipulation, and data security in C.
*/
/* Sample execution :
 Input : (for encoding)
  ./a.out -e beautiful.bmp secret.txt
Output :(for encoding)
  Read and validate sucessfully
    -----Start Encoding----
    width = 1024
    height = 768
    Available capacity in image : 294912 bytes
    Secret file size            : 21 bytes
    Check capacity is success
    Copied bmp header successfully
    OFFSET at 54 before encode magic string
    Encode magic string successfully
    OFFSET at 70 before encode extension size
    Encoded secret file extn size successfully
    OFFSET at 102 before encode file extension
    Encoded secret file extn is successfully
    OFFSET at 134 before encode file size
    Encoded secret file size successfully
    OFFSET at 166 before encode file data
    Encoded the secret file data successfully
    OFFSET at 334 before encode copy remaining data
    Copied remaining data successfully
    Encoding successful

  Input : (for decoding)
    ./a.out -d deafult.bmp
  Output :
    -----Started decoding-----
    Magic string decoded successfully
    Extension size decoded = 4
    Extension decoded = .txt
    Secret file size decoded = 21
    Secret file data decoded successfully
    Decoding successful
    INFO: Decoding completed successfully. Output file: output..txt
*/

#include <stdio.h>
#include "encode.h"
#include "decode.h"
#include "types.h"
#include<string.h>

OperationType check_operation_type(char *arg)
{
    if (strcmp(arg,"-e")==0)
    return e_encode;
    else if(strcmp(arg,"-d")==0)
    return e_decode;
    else 
    return e_unsupported;
}
int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage:\n");
        printf("  Encode: %s -e <src.bmp> <secret.txt> <stego.bmp>\n", argv[0]);
        printf("  Decode: %s -d <stego.bmp> <output.txt>\n", argv[0]);
        return 1;
    }

    OperationType op = check_operation_type(argv[1]);

    if (op == e_encode)
    {
        EncodeInfo encInfo;
        if (read_and_validate_encode_args(argv, &encInfo) == e_failure)
        {
            fprintf(stderr, "ERROR: Invalid encoding arguments\n");
            return 1;
        }

        if (do_encoding(&encInfo) == e_success)
            printf("INFO: Encoding completed successfully. Stego file: %s\n", encInfo.stego_image_fname);
        else
            fprintf(stderr, "ERROR: Encoding failed\n");
    }
    else if (op == e_decode)
{
    DecodeInfo decInfo;

    decInfo.stego_image_fname = argv[2];

    if (argc >= 4)
        decInfo.output_fname = argv[3];
    else
        decInfo.output_fname = NULL;

    if (do_decoding(&decInfo) == e_success)
        printf("INFO: Decoding completed successfully. Output file: %s\n", decInfo.output_fname);
    else
        fprintf(stderr, "ERROR: Decoding failed\n");
}

}
