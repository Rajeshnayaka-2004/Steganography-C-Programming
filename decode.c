#include <stdio.h>
#include <string.h>
#include "types.h"
#include "decode.h"

Status read_and_validate_decode_args(char *argv[], DecodeInfo *decInfo)
{
    if(strstr(argv[2], ".bmp") != NULL)
    {
        printf(".bmp found\n");
        decInfo->dstego_image_fname = argv[2];
    }
    else
    {
        printf(".bmp not present\n");
        return e_failure;
    }
    
    if(argv[3] != NULL)
    {
       char *ptr = strchr(argv[3] , '.');
       *ptr = '\0';
       decInfo->output_fname = argv[3];
       //printf("%s\n",decInfo->output_fname);
    }
    else
    {
        decInfo->output_fname = "output";
        //printf("%s\n",decInfo->output_fname);
    }
    
    return e_success;

}

Status do_decoding(DecodeInfo *decInfo )
{
    printf("<------ Started decoding ------>\n");

    if(dopen_files(decInfo) == e_success)
    {
        printf("All files opened successfully\n");
    }
    else
    {
        printf("Files not opened successfully\n");
        return e_failure;
    }

    fseek(decInfo->fptr_dstego_image, 54 , SEEK_SET);

    printf("Enter encoded magic string: ");
    scanf("%[^\n]", decInfo->magic);

    decInfo->size = strlen(decInfo->magic);

    //printf("%s\n",temp);
    if(decode_magic_string(decInfo) == e_success)
    {
        printf("Magic string decoded successfully\n");
    }
    else
    {
        printf("Failed to decode magic string\n");
        return e_failure;
    }

    if(decode_secret_file_extn_size(decInfo) == e_success)
    {
        printf("Secret file extension size decoded successfully\n");
    }
    else
    {
        printf("Failed to decode secret file extension size\n");
        return e_failure;
    }

    if(decode_secret_file_extn(decInfo) == e_success)
    {
        printf("Secret file extension decoded successfully\n");
        //printf("%s\n",decInfo->temp);
        strcat(decInfo->output_fname , decInfo->temp);
        decInfo->fptr_output = fopen(decInfo->output_fname , "w");
        if(decInfo->fptr_output != NULL)
         printf("Output file is opened successfully\n");
    }
    else
    {
        printf("Failed to decode secret file extension\n");
        return e_failure;
    }

    if(decode_secret_file_size(decInfo) == e_success)
    {
        printf("Secret file size decoded successfully\n");
        //printf("%d\n",decInfo->size);
    }
    else
    {
        printf("Failed to decode secret file size\n");
        return e_failure;
    }

    if(decode_secret_file_data(decInfo) == e_success)
    {
        printf("Secret file data decoded successfully\n");
    }
    else
    {
        printf("Failed to decode secret file data\n");
        return e_failure;
    }

    return e_success;
}

Status dopen_files(DecodeInfo *decInfo)
{
    decInfo->fptr_dstego_image = fopen(decInfo->dstego_image_fname, "r");;
    
    if(decInfo->fptr_dstego_image == NULL)
    {
        printf("encoded file is not present\n");
        return e_failure;
    }

    return e_success;
}

Status decode_magic_string(DecodeInfo *decInfo)
{
    decode_data_from_image(decInfo);
    //printf("magic = %s\n",decInfo->magic);
    //printf("temp = %s\n",decInfo->temp);
    if(strcmp(decInfo->magic ,decInfo->temp) == 0)
    {
        
        printf("Magic string is matched continue decoding\n");
        return e_success;
    }
    else
    {
        printf("Magic string is not matched stop decoding\n");
        return e_failure;
    }

}

Status decode_data_from_image(DecodeInfo *decInfo)
{
    char buffer[8];

    for(int i = 0; i < decInfo->size ; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_dstego_image);
        decode_byte_from_lsb(&decInfo->temp[i], buffer);////////////////
        // sprintf(data[i], "%s",buffer);
    }
    decInfo->temp[decInfo->size] = '\0';

    return e_success;
}

Status decode_byte_from_lsb(char *data, char *buffer)
{
    *data = *data & 0;
    for(int i = 7; i >= 0; i--)
    {
        *data =  *data | ((buffer[7 - i] & 0x01) << i);
    }

    return e_success;
}

Status decode_secret_file_extn_size(DecodeInfo *decInfo)
{
    return decode_int_from_image(decInfo);
}

Status decode_int_from_image(DecodeInfo *decInfo)
{
    char buffer[32];

    fread(buffer, 32, 1, decInfo->fptr_dstego_image);

    decInfo->size = decInfo->size & 0;
    for(int i = 31; i >= 0; i--)
    {
        decInfo->size = decInfo->size |  ((buffer[31 - i] & 0x01) << i);
    }
    //printf("%d\n",*decInfo->size);

    return e_success;
}


Status decode_secret_file_extn(DecodeInfo *decInfo)
{
    return decode_data_from_image(decInfo);
}

Status decode_secret_file_size(DecodeInfo *decInfo)
{
    return decode_int_from_image(decInfo);
}


Status decode_secret_file_data(DecodeInfo *decInfo)
{
    char buffer[8];
    char ch;
    for(int i = 0; i < decInfo->size ; i++)
    {
        fread(buffer, 8, 1, decInfo->fptr_dstego_image);
        decode_byte_from_lsb(&ch, buffer);
        //printf("%c",ch);
        fwrite(&ch , 1 , 1 , decInfo->fptr_output);
    }


    return e_success;
}