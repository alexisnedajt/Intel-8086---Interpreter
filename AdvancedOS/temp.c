#include "main.h"
#include <err.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int loop(char* memory, const char* s, int size, int index){ 
	while(s[index] != ']') {
		size = operations(memory,s,size,&index);
		index++;
	}
	return size;
}

int operations(char* memory, const char* s, int size, int* index) {
	switch(s[*index]) {
		case '+':
			memory[size] += 1;
			//printf("memory[%d] = %hhi\n",size,memory[size]);
			break;
		case '-':
  			memory[size] -= 1;
			//printf("memory[%d] = %hhi\n",size,memory[size]);
			break;
		case '>':
			size++;
			//printf("newSize: %d\n",size);
			break;
		case '<':
			size--;
			//printf("newSize: %d\n",size);
			break;
		case '.':
			printf("%c",memory[size]);
			break;
		case ',':
			char val;
			int r = scanf("%c",&val);
			if (r == -1) 
				errx(1,"couldn't read an int\n");
			memory[size] = val;
			break;
		case '[':
			while(memory[size] > 0) {
				
				size = loop(memory,s,size,*index+1);
			}
			(*index)++;
			int l = 1;
			while(l > 0) {
				if (s[*index] == '[')
					l += 1;
				else{
					if(s[*index] == ']')
						l-=1;
				}
				(*index)++;
			}
			(*index)--;
			break;
	}
	return size;
}

void interpreter(char* memory,const char* s) {
	int size = 0;
	int index = 0;
	while(s[index] != '\0') {
		size = operations(memory,s,size,&index);
		index++;
	}
}

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

int main(int argc, char** argv) {
	if (argc != 2) {
		errx(1,"Too many parameters");
	}
	int file = open(argv[1], O_RDONLY);
	if (file == -1) {
		errx(1,"Could not open the file");
	}
	const char* str;
	int r = read_whole_file(file,&str);
	printf("file of %d characters\n",r);
	
	char* memory = calloc(10000*sizeof(char),0);
	interpreter(memory,str);
	close(file);
	return 1;
}	
