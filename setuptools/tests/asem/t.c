#include <err.h>
#include<stdint.h>
#include<byteswap.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include "ansi.h"
#include </usr/local/core/minix2/usr/include/a.out.h>

int read_whole_file(int fd, const char **output_str)
{
    int max_len = 64;
    int len = 0;
    char *str = malloc((max_len + 1) * sizeof(char));

    int r;
    while ((r = read(fd, str + len, max_len - len)) != 0)
    {
         if (r == -1)
             err(EXIT_FAILURE, "read file");
         len += r;
         if (len == max_len)
         {
             max_len *= 2;
             str = realloc(str, (max_len + 1) * sizeof(char));
         }
     }
     str[len] = 0;
     *output_str = (const char *)str;
     return len;
}

char *instruct(char *text_segment, long *index)
{
	unsigned char op =((unsigned char *)text_segment)[0];
	text_segment++;
	switch (op)
	{
		case 0xbb:
			uint16_t mem = ((uint16_t *)text_segment)[0];
			printf("%04lx: %02hhx%04hx		mov bx, %04hhx\n", *index, op, bswap_16(mem), mem);
			(*index) += 3;
			return text_segment + 2;
		case 0xcd:
			char val = text_segment[0];
			printf("%04lx: %02hhx%02hx		int %02hhx\n", *index, op, val, val);
			(*index) += 2;
			return text_segment + 1;
		case 0x0000:
			printf("%04lx: %04hhx		add [bx+si], al\n", *index, op);
			(*index) += 2;
			return text_segment + 1;
		default:
			break;
	}
	(*index) += 2;
	return text_segment + 1;
}

int main(int argc, char **argv)
{
	if (argc != 2)
		return 1;
	
	char *path = argv[1];
	int fd = open(path, O_RDONLY);
	char *content;
	read_whole_file(fd, &content);

	struct exec *header = (struct exec *)content;

	//printf("%li\n", header->a_text);
	//printf("%hhu %hhu\n", header->a_magic[0], header->a_magic[1]);
	//printf("%hhu %hhu %hhu %hhu %hhu %hhu\n",
	//	header->a_magic[0], header->a_magic[1], header->a_flags, header->a_cpu, header->a_hdrlen, header->a_unused);
	//printf("%i\n", header->a_text);
	//printf("%i %i %i %i %i %i\n", header->a_text,	
    //                              header->a_data,	
    //                              header->a_bss,	
    //                              header->a_entry,
    //                              header->a_total,
	char *text_area = content + header->a_hdrlen;
	char *current_text = text_area;
	long index = 0;
	while (current_text < text_area + header->a_text)
	{
		//printf("%02hhX ", text_area[i]);
		current_text = instruct(current_text, &index);
	}
	char *data_area = text_area + header->a_text;
	printf("\n");
	for (int32_t i = 0; i < header->a_data; ++i)
		printf("%c", data_area[i]);


	return 0;	
}
