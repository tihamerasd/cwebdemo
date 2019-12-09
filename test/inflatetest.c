#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <string.h>


void compress_content(char* input, int input_len, char* output, int* output_len){
z_stream defstream;
defstream.zalloc = Z_NULL;
defstream.zfree = Z_NULL;
defstream.opaque = Z_NULL;
defstream.avail_in = (uInt)input_len; // size of input, string + terminator
defstream.next_in = (Bytef *)input; // input char array
defstream.avail_out = (uInt)input_len; // size of output It's not sure thats enough.
defstream.next_out = (Bytef *)output; // output char array
deflateInit(&defstream, 9);
deflate(&defstream, Z_FINISH);
deflateEnd(&defstream);
printf("Deflated size is: %lu\n", (char*)defstream.next_out - output);
int donelen =(int)((char*)defstream.next_out - output);
}

int main(void){
char *a= "AAAAAAAAAAAAAAAAAAAAAAAAAAAAA\0";
char b[50];
int len=0;
compress_content(a, strlen(a)+1, b, &len);
printf("%d\n", len);

return 0;
}
