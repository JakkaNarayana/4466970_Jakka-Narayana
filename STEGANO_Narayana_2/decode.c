 #include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "decode.h"
#include "common.h"
#include "types.h"

#define MAX_EXTN_SIZE 20
#define MAX_SECRET_FILE_SIZE 10000000   // 10MB safety

char decode_byte_from_lsb(char *image_buffer)
{
    unsigned char ch = 0;
    for (int i = 0; i < 8; i++)
        ch = (ch << 1) | (image_buffer[i] & 1);
    return ch;
}

long decode_size_from_lsb(char *image_buffer)
{
    long size = 0;
    for (int i = 0; i < 32; i++)
        size = (size << 1) | (image_buffer[i] & 1);
    return size;
}

Status decode_magic_string(const char *magic_string, DecodeInfo *decInfo)
{
    char buffer[8];

    for (int i = 0; i < strlen(magic_string); i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            printf("ERROR: Unable to read magic string\n");
            return e_failure;
        }

        char ch = decode_byte_from_lsb(buffer);
        if (ch != magic_string[i])
        {
            printf("ERROR: Magic string mismatch\n");
            return e_failure;
        }
    }

    printf("Magic string decoded successfully\n");
    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    char buffer[32];

    if (fread(buffer, 32, 1, decInfo->fptr_stego_image) != 1)
    {
        printf("ERROR: Unable to read extension size\n");
        return e_failure;
    }

    decInfo->extn_size = (int)decode_size_from_lsb(buffer);

    printf("Extension size decoded = %d\n", decInfo->extn_size);

    if (decInfo->extn_size <= 0 || decInfo->extn_size >= MAX_EXTN_SIZE)
    {
        printf("ERROR: Invalid extension size\n");
        return e_failure;
    }

    return e_success;
}

Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    char buffer[8];

    for (int i = 0; i < decInfo->extn_size; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            printf("ERROR: Unable to read extension\n");
            return e_failure;
        }

        decInfo->extn_secret_file[i] = decode_byte_from_lsb(buffer);
    }

    decInfo->extn_secret_file[decInfo->extn_size] = '\0';

    printf("Extension decoded = %s\n", decInfo->extn_secret_file);

    return e_success;
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    char buffer[32];

    if (fread(buffer, 32, 1, decInfo->fptr_stego_image) != 1)
    {
        printf("ERROR: Unable to read secret file size\n");
        return e_failure;
    }

    decInfo->size_secret_file = decode_size_from_lsb(buffer);

    printf("Secret file size decoded = %ld\n", decInfo->size_secret_file);

    if (decInfo->size_secret_file <= 0 || 
        decInfo->size_secret_file > MAX_SECRET_FILE_SIZE)
    {
        printf("ERROR: Invalid secret file size\n");
        return e_failure;
    }

    return e_success;
}

Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];

    for (long i = 0; i < decInfo->size_secret_file; i++)
    {
        if (fread(buffer, 8, 1, decInfo->fptr_stego_image) != 1)
        {
            printf("ERROR: Unable to read secret data\n");
            return e_failure;
        }

        char ch = decode_byte_from_lsb(buffer);

        if (fwrite(&ch, 1, 1, decInfo->fptr_output) != 1)
        {
            printf("ERROR: Unable to write output\n");
            return e_failure;
        }
    }

    printf("Secret file data decoded successfully\n");
    return e_success;
}

Status do_decoding(DecodeInfo *decInfo)
{
    printf("-----Started decoding-----\n");

    decInfo->fptr_stego_image = fopen(decInfo->stego_image_fname, "rb");
    if (!decInfo->fptr_stego_image)
    {
        printf("ERROR: Unable to open stego image\n");
        return e_failure;
    }

    fseek(decInfo->fptr_stego_image, 54, SEEK_SET);

    if (decode_magic_string(MAGIC_STRING, decInfo) == e_failure)
        return e_failure;

    if (decode_secret_file_extn_size(decInfo) == e_failure)
        return e_failure;

    if (decode_secret_file_extn(decInfo) == e_failure)
        return e_failure;

    if (decode_secret_file_size(decInfo) == e_failure)
        return e_failure;

    static char outname[50];

    if (!decInfo->output_fname)
    {
        sprintf(outname, "output.%s", decInfo->extn_secret_file);
        decInfo->output_fname = outname;
    }

    decInfo->fptr_output = fopen(decInfo->output_fname, "wb");
    if (!decInfo->fptr_output)
    {
        printf("ERROR: Unable to create output file\n");
        return e_failure;
    }

    if (decode_secret_file_data(decInfo) == e_failure)
        return e_failure;

    fclose(decInfo->fptr_output);
    fclose(decInfo->fptr_stego_image);

    printf("Decoding successful\n");
    return e_success;
}
