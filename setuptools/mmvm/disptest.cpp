
#include <stdio.h>

int main(void) {
	short s;
	short num = 10000;
	unsigned char buf[] = {0xee, 0xef};

	s = (char)buf[0];

	printf("%d\n", s);
	printf("%x\n", s);

	if(s < 0) {
		printf("minus\n");
		printf("-%x\n", ~s+1);
		printf("-%x\n", s * -1);
	} else {
		printf("plus\n");
	}

	printf("%d\n", (num + s));
	return 0;
}
