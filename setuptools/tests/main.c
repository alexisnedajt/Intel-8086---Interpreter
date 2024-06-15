#include "main.h"

//Probleme d'affichage de memoire sinon fini !!!!

int Cmp(char* s1, char* s2){
	if (strcmp(s1,s2) == 0)
		return 1;
	return 0;
}

char* pretty(int byte, int byte2, char* memPrint, int disp, int size, int yes, int swap){
	char* s = ""; 
	asprintf(&s,"%02x",byte);
	if (yes == 1){
		asprintf(&s,"%s%02x",s,byte2);
	}
	if (memPrint[0] != '\0'){
		asprintf(&s,"%s%s",s,memPrint);
	}
	if (size == 1){
		disp = disp & 0x00ff;
		asprintf(&s,"%s%02x",s,disp);
	}
	else if (size == 2){
		if (swap == 1)
			asprintf(&s,"%s%04x",s,bswap_16(disp));
		else
			asprintf(&s,"%s%04x",s,disp);
	}
	asprintf(&s,"%-13s",s);
	return s;
}

char* val(int w, int v){
	if (w == 1){
		if (v == 0b000)
			return "ax";
		if (v == 0b001)
			return "cx";
		if (v == 0b010)
			return "dx";
		if (v == 0b011)
			return "bx";
		if (v == 0b100)
			return "sp";
		if (v == 0b101)
			return "bp";
		if (v == 0b110)
			return "si";
		return "di";
	}
	else {
		if (v == 0b000)
			return "al";
		if (v == 0b001)
			return "cl";
		if (v == 0b010)
			return "dl";
		if (v == 0b011)
			return "bl";
		if (v == 0b100)
			return "ah";
		if (v == 0b101)
			return "ch";
		if (v == 0b110)
			return "dh";
		return "bh";	
	}
}

char* r_m(int* rm, int disp, int size){
	char* ans;
	char* op = "+";
	if ((*rm) == 0b000)
		ans = "bx+si";
	else if ((*rm) == 0b001)
		ans = "bx+di";
	else if ((*rm) == 0b010)
		ans = "bp+si";
	else if ((*rm) == 0b011)
		ans = "bp+di";
	else if ((*rm) == 0b100) 
		ans = "si";
	else if ((*rm) == 0b101) 
		ans = "di";
	else if ((*rm) == 0b110) 
		ans = "bp";
	else
		ans = "bx";
	if (size == 1){
		disp = disp & 0x00ff;
		if ((disp & 0b10000000) == 0b10000000){
			op = "-";
			disp = 0b11111111 - disp + 1;
		}
		if (disp < 0b10000){
			asprintf(&ans, "[%s%s%01x]",ans,op,disp);
		}
		else{
			asprintf(&ans, "[%s%s%02x]",ans,op,disp);
		}
	}
	else if (size == 2){
		if ((disp & 0b1000000000000000) == 0b1000000000000000){
			op = "-";
			disp = 0b1111111111111111 - disp + 1;
		}
		if (disp < 0b10000){
			asprintf(&ans, "[%s%s%01x]",ans,op,disp);
		}
		else if (disp < 0b100000000){
			asprintf(&ans, "[%s%s%02x]",ans,op,disp);
		}
		else if (disp < 0b1000000000000){
			asprintf(&ans, "[%s%s%03x]",ans,op,disp);
		}
		else
			asprintf(&ans, "[%s%s%04x]",ans,op,disp);
	}
	else
		asprintf(&ans, "[%s]",ans);
	return ans;
}

char* memPrint(int data, int size, int yes, int d, int neg, int sh){
	char* s = "";
	char* op = "";
	if (sh == 0){
		asprintf(&s,"%04x",data);
	}
	else{
		if (neg == 1) {
			//printf("first data = %d\n",data);
			if ((data & 0x8000) == 0x8000){
					data = 0xffff - data + 1;
					//printf("second data = %02x\n",data);
					op = "-";
			}
		}
		if (data >= 0 || (data < 0 && neg == 1)){
			switch (size){
				case 1:
					if (sh == 1) {
						if (d == 1 && data < 16)
							asprintf(&s,"%s%1x",op,data);
						else
							asprintf(&s,"%s%02x",op,data);
					}
					else
						asprintf(&s,"%s%02x",op,data);
					return s;
				case 2:
					if (yes == 1){
						data = bswap_16(data);
					}
					asprintf(&s,"%s%04x",op,data);
					return s;
				case 4:
					if (yes == 1){
						data = bswap_32(data);
					}
					if (sh == 1){
						if (data < 0x10)
							asprintf(&s,"%s%01x",op,data);
						else if (data < 0x100)
							asprintf(&s,"%s%02x",op,data);
						else if (data < 0x1000)
							asprintf(&s,"%s%03x",op,data);
						else
							asprintf(&s,"%s%04x",op,data);
					}
					else{
						asprintf(&s,"%s%04x",op,data);
					}
					return s;
			}
		}
	}
	return s;
}

void D_w_mod_reg_rm(FILE* file, char* inst, int byte, int memory, int* d, int* w, int* mod, int* reg, int*rm, uint32_t* add){
	char* first = val(*w,*reg);
	char* second;
	uint8_t a = ((*mod) << 6) + ((*reg) << 3) + *rm;
	int disp = 0;
	uint8_t dis8;
	int s = 0;
	if (*mod == 0b01){
		fread(&dis8,sizeof(dis8),1,file);
		*add += 1;
		disp = (int16_t)(dis8);
		s = 1;
		second = r_m(rm,disp,s);
	}
	else if (*mod == 0b10){
		fread(&dis8,sizeof(dis8),1,file);
		disp = (int8_t)(dis8);
		fread(&dis8,sizeof(dis8),1,file);
		disp = ((int8_t)(dis8) << 8) + disp;
		*add += 2;
		s = 2;
		second = r_m(rm,disp,s);
	}
	else if (*mod == 0b00 ){
		if (*rm == 0b110){
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = dis8;
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = ((dis8) << 8) + disp;
			*add += 2;
			asprintf(&second,"[%04x]",disp);
			s = 2 ;
		}
		else{
			second = r_m(rm,0,0);
		}
	}
	else if (*mod == 0b11){
		second = val(*w,*rm);
	}
	if (*d == 0){
		char** temp = malloc(sizeof(char*));
		*temp = first;
		first = second;
		second = *temp;
		free(temp);
	}
	printf("%04x: %s %s %s, %s\n", memory, pretty(byte,a,"",disp,s,1,1), inst, first, second);
	return;
}

void W_mod_rm(FILE* file, char* inst, int byte, int memory, int* w, int* mod, int* byte2, int* rm, int data, int size, uint32_t* add){
	char* r = "";
	int disp = -1;
	int s = 1;
	uint8_t dis8;
	int d = 0;
	int sawp = 1;
	if (*mod == 0b01){
		fread(&dis8,sizeof(dis8),1,file);
		*add += 1;
		if (data == -1){
			disp = dis8;
			data = dis8;
			d = bswap_16(data);
		}
		else {
			disp = data;
			data = dis8;
			d = data;
		}
		if (size == 2){
			d = disp & 0x00ff;
			disp >>= 8;
			data = (d << 8) + dis8;
			d = bswap_16(data);
		}
		s = 1;
		r = r_m(rm,disp,s);
	}
	else if (*mod == 0b10){
		fread(&dis8,sizeof(dis8),1,file);
		disp = (int8_t)(dis8);
		fread(&dis8,sizeof(dis8),1,file);
		disp = ((int8_t)(dis8) << 8) + disp;
		d = data;
		*add += 2;
		s = 2;
		r = r_m(rm,disp,s);
	}
	else if (*mod == 0b00){
		if (*rm == 0b110){
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = dis8;
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = ((dis8) << 8) + disp;
			d = data;
			*add += 2;
			data = disp;
			disp = d;
			d = data;
			asprintf(&r,"[%04x]",bswap_16(disp));
			s = 1;
		}
		else {
			disp = data;
			d = -1;
			r = r_m(rm,disp,0);
		}
	}
	else if (*mod == 0b11){
		r = val(*w,*rm);
		d = data;
		if (Cmp(inst,"or") || Cmp(inst,"and")/* || Cmp(inst,"xor") || Cmp(inst,"add") || Cmp(inst,"adc") || Cmp(inst,"sub") || Cmp(inst,"sbb")*/ || Cmp(inst,"test")){
			sawp = 0;
		}
	}
	if (Cmp(inst,"mov")){
		if (d == -1){
			data = bswap_16(disp);
			size = 1;
			s = 0;
		}
		else{
			data = (d << 8) + dis8;
		}
		if (size > 1)
			size = 1;
		d = bswap_16(data)  ;
	}
	char* mprint = memPrint(data,size,1,1,0,s);
	if (mprint[0] != '\0'){
		asprintf(&r,"%s,",r);
	}
	if (d == -1)
		size = 0;
	printf("%04x: %s %s %s %s\n", memory, pretty(byte,*byte2,memPrint(disp,1,0,0,0,1),d,size,1,sawp), inst, r, mprint);
	return;
}

void W_reg(char* inst, int byte, int memory, int* w, int* reg, int data, int size){
	char* s;
	asprintf(&s,"%04x: %s", memory, pretty(byte,0,memPrint(data,size,0,0,0,1),0,0,0,1));
	printf("%s %s %s, %s\n", s, inst, val(*w,*reg), memPrint(data,size,1,1,0,1));
	return;
}

void W(char* inst, int byte, int memory, int data, int size){
	char* s;
	asprintf(&s,"%04x: %s", memory, pretty(byte,0,memPrint(data,size,0,0,0,1),0,0,0,1));
	printf("%s %s %s\n", s, inst, memPrint(data,size,1,1,0,1));
	return;
}

void Mod_reg_rm(FILE* file, char* inst, int byte, int memory, int* mod, int* reg, int* rm, uint32_t* add){
	char* first = val(1,*reg);
	char* second;
	int a = ((*mod) << 6) + ((*reg) << 3) + *rm;
	int disp = 0;
	int s = 0;
	uint8_t dis8;
	if (*mod == 0b01){
		fread(&dis8,sizeof(dis8),1,file);
		*add += 1;
		disp = dis8;
		s = 1;
		asprintf(&second,"%s",r_m(rm,disp,s));
	}
	else if (*mod == 0b10){
		fread(&dis8,sizeof(dis8),1,file);
		disp = dis8;
		fread(&dis8,sizeof(dis8),1,file);
		disp = (dis8 << 8) + disp;
		*add += 2;
		s = 2;
		asprintf(&second,"%s",r_m(rm,disp,s));
	}
	else if (*mod == 0b00){
		if (*rm == 0b110){
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = dis8;
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = ((dis8) << 8) + disp;
			*add += 2;
			asprintf(&second,"[%04x]",disp);
			s = 2 ;
		}
		else
			second = r_m(rm,0,0);
	}
	if (*mod == 0b11){
		asprintf(&second,"%s",val(1,*rm));
	}
	printf("%04x: %s %s %s, %s\n", memory, pretty(byte,a,"",disp,s,1,1), inst, first, second);
	return;
}

void Reg(char* inst, int byte, int memory, int* reg){
	char* op = "";
	if (Cmp(inst,"xchg"))
		op = ", ax";
	printf("%04x: %s %s %s %s\n", memory, pretty(byte,0,"",0,0,0,1), inst, val(1,*reg), op);
	return;
}

void Mod_rm(FILE* file, char* inst, int byte, int memory, int* mod, int* mid, int* rm, int data, int size, uint32_t* add){
	char* first;
	int a = ((*mod) << 6) + ((*mid) << 3) + *rm;
	int disp = 0;
	uint8_t dis8;
	int s = 0;
	if (*mod == 0b01){
		fread(&dis8,sizeof(dis8),1,file);
		*add += 1;
		disp = (int8_t)(dis8);
		s = 1;
		first = r_m(rm,disp,s);
	}
	else if (*mod == 0b10){
		fread(&dis8,sizeof(dis8),1,file);
		disp = (int8_t)(dis8);
		fread(&dis8,sizeof(dis8),1,file);
		disp = ((int8_t)(dis8) << 8) + disp;
		*add += 2;
		s = 2;
		first = r_m(rm,disp,s);
	}
	else if (*mod == 0b00 ){
		if (*rm == 0b110){
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = dis8;
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = ((dis8) << 8) + disp;
			*add += 2;
			asprintf(&first,"[%04x]",disp);
			s = 2 ;
		}
		else{
			first = r_m(rm,0,0);
			s = 0;
			first = r_m(rm,disp,s);
		}
	}
	if (*mod == 0b11)
		first = val(1,*rm);
	char* mprint = memPrint(data,size,1,1,0,1);
	if (mprint[0] != '\0'){
		asprintf(&first,"%s,",first);
	}
	printf("%04x: %s %s %s %s\n", memory, pretty(byte,a, memPrint(data,size,0,0,0,1),disp,s,1,1), inst, first, mprint);
	return;
}

void S_w_mod_rm(FILE* file, char* inst, int byte, int memory, int* w, int* mod, int* reg, int*rm, int data, int size, uint32_t* add){
	char* first;
	uint8_t a = ((*mod) << 6) + ((*reg) << 3) + *rm;
	int disp = 0;
	int s = 0;
	uint8_t dis8;
	int d = data;
	int ds = 0;
	if (*mod == 0b01){
		fread(&dis8,sizeof(dis8),1,file);
		*add += 1;
		disp = data;
		if (size == 2){
			disp = (disp & 0xff00) >> 8;
		}
		s = 1;
		ds = dis8;
		d = data;
		if (size == 2)
			data = ((data & 0x00ff) << 8) + dis8;
		else
			data = dis8;
		first = r_m(rm,disp,s);
	}
	else if (*mod == 0b10){
		fread(&dis8,sizeof(dis8),1,file);
		disp = (dis8 << 8) + data;
		fread(&dis8,sizeof(dis8),1,file);
		d = disp;
		data = dis8;
		*add += 2;
		s = 2;
		ds = dis8;
		first = r_m(rm,disp,s);
	}
	else if (*mod == 0b00 ){
		if (*rm == 0b110){
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = (dis8 << 8) + data;
			fread(&dis8,sizeof(uint8_t),1,file);
			d = disp;
			ds = dis8;
			data = dis8;
			*add += 2;
			asprintf(&first,"[%04x]",disp);
			s = 2 ;
		}
		else{
			first = r_m(rm,0,0);
			s = 0;
			d = data;
		}
	}
	else if (*mod == 0b11){
		d = data;
		first = val(*w,*rm);
	}
	int neg = 0;
	if (Cmp(inst,"cmp") || Cmp(inst,"cmp byte")){
		if ((data & 0x80) == 0x80)
			data = 0xff00 + data;
		else
			data = 0x0000 + data;
		neg = 1;
	}
	printf("%04x: %s %s %s, %s\n", memory, pretty(byte,a,memPrint(d,size,0,0,0,1),ds,s,1,1), inst, first, memPrint(data,size,1,1,neg,1));
	return;
}

void W_mod_reg_rm(FILE* file, char* inst, int byte, int memory, int* w, int* mod, int* reg, int* rm, uint32_t* add){
	char* second = val(*w,*reg);
	char* first;
	int a = ((*mod) << 6) + ((*reg) << 3) + *rm;
	int disp = 0;
	int size = 0;
	uint8_t dis8;
	if (*mod == 0b01){
		fread(&dis8,sizeof(dis8),1,file);
		*add += 1;
		disp = (int8_t)(dis8);
		size = 1;
		first = r_m(rm,*mod,size);
	}
	else if (*mod == 0b10){
		fread(&dis8,sizeof(dis8),1,file);
		disp = (int8_t)(dis8);
		fread(&dis8,sizeof(dis8),1,file);
		disp = ((int8_t)(dis8) << 8) + disp;
		*add += 2;
		size = 2;
		first = r_m(rm,*mod,size);
	}
	else if (*mod == 0b00){
		if (*rm == 0b110){
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = dis8;
			fread(&dis8,sizeof(uint8_t),1,file);
			disp = ((dis8) << 8) + disp;
			*add += 2;
			asprintf(&first,"[%04x]",disp);
			size = 2 ;
		}
		else{
			first = r_m(rm,0,0);
		}
	}
	if (*mod == 0b11){
		first = val(*w,*rm);
	}
	char* di = "";
	if (disp != 0){
		asprintf(&di,"%02x",disp);
	}
	printf("%04x: %s %s %s, %s\n", memory, pretty(byte, a, "", disp,size,1,1), inst, first, second);
	return;
}

void nothing(char* inst, int byte, int memory, int data, int disp, int yes, int size){
	char* str;
	asprintf(&str,"%04x: %s", memory, pretty(byte,0,memPrint(data,size,1-yes,0,0,1),0,0,0,1));
	disp = disp & 0x0000ffff;
	printf("%s %s %s\n", str, inst, memPrint(disp,size,yes,1,0,1));
}

void jump(FILE* file, char* inst, int byte, int memory, uint32_t* add){
		int data = 0;
		fread(&data,sizeof(uint8_t),1,file);
		*add = 1;
		int inc = data;
		if ((data & 0b10000000) == 0b10000000){
			inc = (int8_t)(data);
		}
		char* str;
		asprintf(&str,"%04x: %s", memory, pretty(byte,0,memPrint(data,1,0,0,0,1),0,0,0,1));
		printf("%s %s %04x\n", str, inst, memory+inc+2);
}

void undefined(int byte, int memory){
	char* s;
	asprintf(&s,"%04x: %s", memory, pretty(byte,0,"",0,0,0,1));
	printf("%s (undefined)\n", s);
}

char* Inst(uint8_t byte){
	switch(byte){
		case AAM1:
			return "aam";
		case AAD1:
		case AAD2:
			return "aad";
		case AAS:
			return "aas";
		case DAS:
			return "das";
		case XLAT:
			return "xlat";
		case LEA:
			return "lea";
		case LDS:
			return "lds";
		case LES:
			return "les";
		case LAHF:
			return "lahf";
		case SAHF:
			return "sahf";
		case PUSHF:
			return "pushf";
		case POPF:
			return "popf";
		case POP1:
			return "pop";
		case PUSH1:
			return "push";
		case XOR:
			return "xor";
		case MOV7:
		case MOV6:
			return "mov";
	}
	switch (byte >> 1){
		case MOVS:
			if ((byte & 0b00000001) == 0b00000001)
				return "movsw";
			return "movsb";
		case MUL:
			return "mul";
		case CMP3:
			return "cmp";
		case SUB3:
			return "sub";
		case AAA:
			return "aaa";
		case BAA:
			return "baa";
		case INC1:
			return "inc";
		case ADC3:
			return "adc";
		case ADD3:
			return "add";
		case OUT1:
		case OUT2:
			return "out";
		case IN1:
		case IN2:
			return "in";
		case XCHG1:
			return "xchg";
		case MOV3:
		case MOV4:
		case MOV2:
			return "mov";
	}
	switch (byte >> 2){
		case CMP1:
			return "cmp";
		case SSB1:
		case SSB3:
			return "sbb";
		case SUB1:
			return "sub";
		case ADC1:
			return "adc";
		case ADD1:
		case ADD2:
			return "add";
		case MOV1:
			return "mov";

	}
	switch (byte >> 3){
		case DEC2:
			return "dec";
		case INC2:
			return "inc";
		case XCHG2:
			return "xchg";
		case POP2:
			return "pop";
		case PUSH2:
			return "push";
	}
	switch (byte >> 4){
		case MOV3:
			return "mov";
	}
	switch (byte >> 5){
		case POP3:
			return "pop";
		case PUSH3:
			return "push";
	}
	return "";
}


//---------------------------------------------------------------------------------------


char* instruct5(int index, uint8_t byte, uint32_t* add) {
	//uint8_t b;
	*add = 0;
	switch(byte>>5){
		case PUSH3:
			int reg = ((byte & 0b00011000) >> 3);
			Reg("push",byte,index,&reg);
			return "push";
	}
	return "";
}

char* instruct4(FILE* file, int index, uint8_t byte, int* w, int* reg, int* data1, uint32_t* add) {
	uint8_t b;
	*add = 0;
	int size;
	switch(byte>>4){
		case MOV3:
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*w = (byte & 0b00001000) >> 3;
			*reg = byte & 0b00000111;
			*data1 = b;
			size = 1;
			if (*w == 1){
				int temp = *data1 << 8;
				fread(data1,sizeof(b),1,file);
				*data1 = temp + *data1;
				*add = 2;
				*data1 = *data1;
				size = 2;
			}
			W_reg("mov",byte,index,w,reg,*data1,size);
			return "mov";
	}	
	return "";
}

char* instruct3(FILE* file, int index, uint8_t byte, int*w, int* mod, int* reg, int* rm, uint32_t* add) {
	uint8_t b;
	*add = 0;
	switch(byte>>3){
		case ESC:
			*w = byte & 0b00000001;
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*mod = (b & 0b11000000)>>6;
			*rm = (b & 0b00000111);
			int mid = (b & 0b00111000) >> 3;
			Mod_rm(file,"esc",byte,index,mod,&mid,rm,0,0,add);
			return "esc";
		case PUSH2:
		case POP2:
		case XCHG2:
		case INC2:
		case DEC2:
			*reg = byte & 0b00000111;
			if (byte >> 3 == PUSH2){
				Reg("push",byte,index,reg);
				return "push";
			}	
			if (byte >> 3 == POP2){
				Reg("pop",byte,index,reg);
				return "pop";
			}
			if (byte >> 3 == XCHG2){
				Reg("xchg",byte,index,reg);
				return "xchg";
			}
			if (byte >> 3 == INC2){
				Reg("inc",byte,index,reg);
				return "inc";
			}
			Reg("dec",byte,index,reg);
			return "dec";
	}
	return "";
}	

char* instruct2(FILE* file, int index, uint8_t byte, int* d, int* w, int* mod, int* reg, int* rm,/* int* data1,*/ uint32_t* add) {
	uint8_t b;
	*add = 0;
	switch(byte>>2){
		case MOV1:
		case ADD1:
		case ADC1:
		case SUB1:
		case SSB1:
		case CMP1:
			*d = (byte & 0b00000010) > 1;
			*w = byte & 0b00000001;
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*mod = (b & 0b11000000) >> 6;
			*reg = (b & 0b00111000) >>3;
			*rm = b & 0b00000111;
			if (byte>>2 == MOV1){
				D_w_mod_reg_rm(file, "mov", byte, index, d, w, mod, reg, rm, add);
				return "mov";
			}
			if (byte>>2 == ADD1){
				D_w_mod_reg_rm(file, "add", byte, index, d, w, mod, reg, rm, add);
				return "add";
			}
			if (byte>>2 == ADC1){
				D_w_mod_reg_rm(file,"adc", byte, index, d, w, mod, reg, rm, add);

				return "adc";	
			}
			if (byte>>2 == SUB1){
				D_w_mod_reg_rm(file,"sub", byte, index, d, w, mod, reg, rm, add);
				return "sub";
			}
			if (byte>>2 == SSB1){
				D_w_mod_reg_rm(file,"sbb", byte, index, d, w, mod, reg, rm, add);
				return "sbb";
			}
			D_w_mod_reg_rm(file,"cmp", byte, index, d, w, mod, reg, rm, add);
			return "cmp";
		case AND1:
		case OR1:
		case XOR1:
		    char* str = "";
			if (byte>>2 == OR1)
				asprintf(&str,"or");
			else if (byte>>2 == XOR1)
				asprintf(&str,"xor");
			else
				asprintf(&str,"and");
			*d = (byte & 0b00000010) >> 1;
			*w = byte & 0b00000001;
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*mod = (b & 0b11000000) >> 6;
			*reg = (b & 0b00111000) >> 3;
			*rm = b & 0b00000111;
			D_w_mod_reg_rm(file,str, byte, index, d, w, mod, reg, rm, add);
			return "xor";
		case ADD2:
		    *d = (byte & 0b00000010) >> 1;
			*w = byte & 0b00000001;
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*mod = (b & 0b11000000) >> 6;
			*reg = (b & 0b00111000) >> 3;
			*rm = b & 0b00000111;
			uint8_t data1;
			fread(&data1,sizeof(data1),1,file);
			*add = 2;
			if (*d == 0 && *w == 1){
				int temp = data1;
				fread(&data1,sizeof(data1),1,file);
				*add = 3;
				data1 = (data1 << 8) + temp;
			}
			if ((b & 0b00111000) == 0b00000000){
				S_w_mod_rm(file,"add",byte,index,w,mod,reg,rm,data1,1,add);
				return "add";
			}
			else if ((b & 0b00111000) == 0b00010000){
				S_w_mod_rm(file,"adc",byte,index,w,mod,reg,rm,data1,1,add);
				return "adc";
			}
			else if ((b & 0b00111000) == 0b00101000){
				S_w_mod_rm(file,"sub",byte,index,w,mod,reg,rm,data1,1,add);
				return "sub";
			}
			else if ((b & 0b00111000) == 0b00011000){
				S_w_mod_rm(file,"sbb",byte,index,w,mod,reg,rm,data1,1,add);
				return "ssb";
			}
			if (*w == 1)
				S_w_mod_rm(file,"cmp",byte,index,w,mod,reg,rm,data1,1/*2*/,add);
			else
				S_w_mod_rm(file,"cmp byte",byte,index,w,mod,reg,rm,data1,1/*2*/,add);
			return "cmp";
		case SHL:
			char* c ;
			*d = (byte & 0b00000010) >> 1;
			*w = byte & 0b00000001;
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*mod = (b & 0b11000000) >> 6;
			*reg = (b & 0b00111000) >> 3;
			*rm = b & 0b00000111;
			int byte2 = (int)b;
			if (*reg == 0b100)
				c = "shl";
			else if (*reg == 0b101)
				c = "shr";
			else if (*reg == 0b111)
				c = "sar";
			else if (*reg == 0b000)
				c = "rol";
			else if (*reg == 0b001)
				c = "ror";
			else if (*reg == 0b010)
				c = "rcl";
			else if (*reg == 0b011)
				c = "rcr";
			str = "1";
			if (*d == 1)
				asprintf(&str,"cl");
			char* r = val(*w,*rm);
			int disp = 0;
			int s = 0;
			uint8_t dis8;
			if (*mod == 0b01){
				fread(&dis8,sizeof(dis8),1,file);
				*add += 1;
				disp = (int8_t)(dis8);
				s = 1;
			}
			else if (*mod == 0b10){
				fread(&dis8,sizeof(dis8),1,file);
				disp = (int8_t)(dis8);
				fread(&dis8,sizeof(dis8),1,file);
				disp = ((int8_t)(dis8) << 8) + disp;
				*add += 2;
				s = 2;
			}
			if (*mod != 0b11){
				r = r_m(rm,disp,s);
			}
			printf("%04x: %s %s %s, %s\n", index, pretty(byte,byte2,memPrint(byte2,1,0,0,0,1),disp,s,0,1), c, r, str);
			return "shl";
	}			
	return "";
}

char* instruct1(FILE* file, int index, uint8_t byte,int* d, int* w, int* mod, int* reg, int* rm, int* data1, uint32_t* add) {
	uint8_t b;
	*add = 0;
	int size = 0;
	int byte2 = 0;
	char* str = "";
	char* st;
	char* s;
	switch(byte>>1) {
		case INC1:
		    printf("an here\n");
		    *w = byte & 0b00000001;
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*mod = (b & 0b11000000) >> 6;
			*rm = b & 0b00000111;
			byte2 = (int)b;
			if ((b & 0b00111000) == 0b00001000){
					W_mod_rm(file,"dec",byte,index,w,mod,&byte2,rm,-1,0,add);
					return "dec";
			}
			return "";
		case REP:
				fread(&b,sizeof(b),1,file);
				*add = 1;
				char* inst = Inst(b);
				asprintf(&s,"%04x: %s", index, pretty(byte,0,memPrint(b,1,0,0,0,1),0,0,0,1));
				printf("%s %s %s\n", s, "rep", inst);
				return "rep";
		case CMPS:
		case SCAS:
		case LODS:
		case STOS:
		case MOVS:
			if (byte >> 1 == CMPS)
				asprintf(&str,"cmps");
			else if (byte >> 1 == SCAS)
				asprintf(&str,"scas");
			else if (byte >> 1 == LODS)
				asprintf(&str,"lods");
			else if (byte >> 1 == STOS)
				asprintf(&str,"stos");
			else
				asprintf(&str,"movs");
			*w = byte & 0b00000001;
			if (w == 0){
				asprintf(&str,"%sb",str);
				nothing("movsw",byte,index,-1,-1,0,0);
			}
			else{
				asprintf(&str,"%sw",str);
				nothing("movsb",byte,index,-1,-1,0,0);
			}
			return "movs";
		case MOV2:
			fread(&b,sizeof(b),1,file);
			*add = 1;
			if ((b & 0b00111000) == 0b00000000){
				str = "mov byte";
				*w = byte & 0b00000001;
				*mod = (b & 0b11000000)>>6;
				*rm = (b & 0b00000111);
				fread(data1,sizeof(b),1,file);
				size = 1;
				byte2 = (*mod << 6)+ ((b & 0b00111000) << 3) + *rm;
				*add = 2;
				if (*w == 1) {
					str = "mov";
					int temp = *data1 << 8;
					fread(data1,sizeof(b),1,file);
					*data1 = temp + *data1;
					*add = 3;
					size = 2;
				}
				W_mod_rm(file,str,byte,index,w,mod,&byte2,rm,*data1,size, add);
				return "mov";
			}
			return "";
		case XCHG1:
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*w = byte & 0b00000001;
			*mod = (b & 0b11000000)>>6;
			*reg = (b & 0b00111000)>>3;
			*rm = b & 0b00000111;
			W_mod_reg_rm(file,"xchg",byte,index,w,mod,reg,rm,add);
			return "xchg";
		case IN1:
		case OUT1:
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*w = byte & 0b00000001;
			st = "al";
			if (*w == 1)
				st = "ax";
			if (byte >> 1 == IN1)
				str = "in";
			else
				str = "out";
			asprintf(&s,"%04x: %s", index, pretty(byte,b,memPrint(b,1,1,1,0,1),0,0,0,1));
			printf("%s %s %s, %s\n", s, str, st, memPrint(b,1,1,1,0,1));
			return "int";
		case IN2:
		case OUT2:
			*w = byte & 0b00000001;
			st = "al";
			if (*w == 1)
				st = "ax";
			if (byte >> 1 == IN2)
				str = "in";
			else
				str = "out";
			asprintf(&s,"%04x: %s", index, pretty(byte,0,"",0,0,0,1));
			printf("%s %s %s, dx\n", s, str, st);
			return "out";
		case ADD3:
			*w = byte & 0b00000001;
			fread(data1,sizeof(b),1,file);
			*add = 1;
			size = 1;
			if (*w == 1) {	
				int temp = *data1 << 8;
				fread(data1,sizeof(b),1,file);
				*data1 = temp + *data1;
				*add = 2;
				*data1 = *data1;
				size = 2;
			}
			W("add",byte,index,*data1,size);
			return "add";
		case ADC3:
			*w = byte & 0b00000001;
			fread(data1,sizeof(byte),1,file);
			*add = 1;
			size = 1;
			if (*w == 1){
				int temp = *data1 << 8;
				fread(data1,sizeof(b),1,file);
				*data1 = temp + *data1;
				*add = 2;
				size = 2;
			}
			W("adc",byte,index,*data1,size);
			return "adc";
		case SUB3:
			*w = byte & 0b00000001;
			fread(data1,sizeof(byte),1,file);
			*add = 1;
			size = 1;
			if (*w == 1) {
				int temp = *data1 << 8;
				fread(data1,sizeof(b),1,file);
				*data1 = temp + *data1;
				*add = 2;
				size = 2;
			}
			W("sub ax,",byte,index,*data1,size);
			return "sub";
		case SSB3:
			*w = byte & 0b00000001;
			fread(data1,sizeof(byte),1,file);
			*add = 1;
			size = 1;
			if (*w == 1) {
				*add = 2;
				int temp = *data1 << 8;
				fread(data1,sizeof(b),1,file);
				*data1 = temp + *data1;	
				size = 2;
			}
			W("sbb",byte,index,*data1, size);
			return "ssb";
		case CMP3:
			*w = byte & 0b00000001;
			fread(data1,sizeof(byte),1,file);
			*add = 1;
			size = 1;
			str = "cmp";
			if (*w == 1) {
				str = "cmp ax,";
				int temp = *data1 << 8;
				fread(data1,sizeof(b),1,file);
				*data1 = temp + *data1;
				*add = 2;
				*data1 = *data1/*bswap_16(*data1)*/;
				size = 2;
			}
			W(str,byte,index,*data1,size);
			return "cmp";
		case MUL:
		case XOR2:
			*w = byte & 0b00000001;
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*mod = (b & 0b11000000) >> 6;
			*reg = (b & 0b00111000) >> 3;
			*rm = b & 0b00000111;
			int byte2 = (int)b;
			if (byte >> 1 == MUL){
				if ((b & 0b00111000) == 0b00100000){
					W_mod_rm(file,"mul",byte,index,w,mod,&byte2,rm,-1,0,add);
					return "mul";
				}
				else if ((b & 0b00111000) == 0b00101000){
					W_mod_rm(file,"imul",byte,index,w,mod,&byte2,rm,-1,0,add);
					return "mul";
				}
				else if ((b & 0b00111000) == 0b00110000){
					W_mod_rm(file,"div",byte,index,w,mod,&byte2,rm,-1,0,add);
					return "div";
				}
				else if ((b & 0b00111000) == 0b00111000){
					W_mod_rm(file,"idiv",byte,index,w,mod,&byte2,rm,-1,0,add);
					return "idiv";
				}
				else if ((b & 0b00111000) == 0b00011000){
					W_mod_rm(file,"neg",byte,index,w,mod,&byte2,rm,-1,0,add);
					return "neg";
				}
				else if ((b & 0b00111000) == 0b00010000){
					W_mod_rm(file,"not",byte,index,w,mod,&byte2,rm,-1,0,add);
					return "not";
				}
			}
			fread(data1,sizeof(uint8_t),1,file);
			*add = 2;
			size = 1;
			if ((b & 0b00111000) == 0b00110000)
				str = "xor";
			else if (byte >> 1 == MUL && (b & 0b00111000) == 0b00000000){
				if (*w == 0 && *mod != 0b11)
					str = "test byte";
				else
					str = "test";
			}
			else if ((b & 0b00111000) == 0b00001000)
				str = "or";
			else if ((b & 0b00111000) == 0b00100000)
				str = "and";
			if (str[0] != '\0'){
				if (*w == 1){
					*add = 3;
					int temp = *data1 << 8;
					fread(data1,sizeof(b),1,file);
					*data1 = temp + *data1;
					size = 2;
				}
				//printf("data1 = %02x\n",*data1);
				W_mod_rm(file,str,byte,index,w,mod,&byte2,rm,*data1,size,add);
			}
			else{
				if (*d == 0 && *w == 1){
					int temp = *data1 << 8;
					fread(data1,sizeof(b),1,file);
					*data1 = temp + *data1;
					*add = 3;
					size = 2;
				}
				if ((b & 0b00111000) == 0b00000000)
					S_w_mod_rm(file,"add",byte,index,w,mod,reg,rm,*data1,size,add);
				else if ((b & 0b00111000) == 0b00010000)
					S_w_mod_rm(file,"adc",byte,index,w,mod,reg,rm,*data1,size,add);
				else if ((b & 0b00111000) == 0b00101000)
					S_w_mod_rm(file,"sub",byte,index,w,mod,reg,rm,*data1,size,add);
				else if ((b & 0b00111000) == 0b00011000)
					S_w_mod_rm(file,"sbb",byte,index,w,mod,reg,rm,*data1,size,add);
				else if ((b & 0b00111000) == 0b00111000){
					if (*w == 1)
						S_w_mod_rm(file,"cmp",byte,index,w,mod,reg,rm,*data1,size,add);
					else
						S_w_mod_rm(file,"cmp byte",byte,index,w,mod,reg,rm,*data1,size,add);
				}
			}
			return "cmp";
		case AND3:
		case TEST3:
		case OR3:
		case XOR3:
			if (byte >> 1 == OR3)
				asprintf(&str,"or");
			else if (byte >> 1 == XOR3)
				asprintf(&str,"xor");
			else if (byte >> 1 == AND3)
				asprintf(&str,"and");
			else
				asprintf(&str,"test al,");
			*w = byte & 0b00000001;
			fread(data1,sizeof(b),1,file);
			*add = 1;
			int size = 1;
			if (*w == 1){
				*add = 2;
				int temp = *data1 << 8;
				fread(data1,sizeof(b),1,file);
				*data1 = temp + *data1;
				size = 2 ;
			}
			W(str,byte,index,*data1,size);
			return "xor";
		case TEST1:
			*w = byte & 0b00000001;
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*mod = (b & 0b11000000) >> 6;
			*reg = (b & 0b00111000) >> 3;
			*rm = b & 0b00000111;
			W_mod_reg_rm(file,"test",byte,index,w,mod,reg,rm,add);
			return "test";
	}
	return "";
}

char* instruct0(FILE* file, int index, uint8_t byte, int* mod, int* reg, int* rm, uint32_t* add) {
	uint8_t b;
	int data = 0;
	*add = 0;
	switch(byte) {
		case MOV6:
		case MOV7:
			fread(&b,sizeof(b),1,file);
			*add = 1;
			if ((b & 0b001000000) == 0b000000000) {
				*mod = (b & 0b110000000) >> 7;
				*reg = (b & 0b000111000) >> 3;
				*rm = b & 0b000000111;
							printf("bruh\n");
				char* ins = "mov";
				Mod_reg_rm(file,ins,byte,index,mod,reg,rm,add);
				return ins;
			}
			return "";
		case PUSH1:
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*mod = (b & 0b11000000) >> 6;
			*rm = (b & 0b00000111);
			int mid = (b & 0b00111000) >> 3;
			int byte2 = (int)b;
			int w = (byte & 0b00000001);
			if ((b & 0b00111000) == 0b00001000){
					W_mod_rm(file,"dec",byte,index,&w,mod,&byte2,rm,-1,0,add);
					return "dec";
				}
			if ((b & 0b00111000) == 0b00000000){
				int w = (byte & 0b00000001);
				W_mod_rm(file,"inc",byte,index,&w,mod,&byte2,rm,-1,0,add);
			}
			else if ((b & 0b00111000) == 0b00110000) {
				Mod_rm(file,"push",byte,index,mod,&mid,rm,-1,0,add);
			}
			else if (((b & 0b00111000) == 0b00100000) || ((b & 0b00111000) == 0b00101000)){
				Mod_rm(file,"jmp",byte,index,mod,&mid,rm,-1,0,add);
			}
			else if (((b & 0b00111000) == 0b00010000) || ((b & 0b00111000) == 0b00011000)){
				Mod_rm(file,"call",byte,index,mod,&mid,rm,-1,0,add);
			}
			return "jmp";
		case POP1:
			fread(&b,sizeof(b),1,file);
			*add = 1;
			if ((b & 0b00111000) == 0b00000000) {
				*mod = (b & 0b11000000) >> 6;
				*rm = (b & 0b00000111);
				int mid = b & 0b00111000;
				Mod_rm(file,"pop",byte,index,mod,&mid,rm,-1,0,add);
				return "pop";
			}
			return "";
		case XLAT:
			nothing("xlat",byte,index,-1,-1,0,0);
			return "xlat";
		case LEA:
		case LDS:
		case LES:
			fread(&b,sizeof(b),1,file);
			*add = 1;
			*mod = (b & 0b11000000) >> 6;
			*reg = (b & 0b00111000) >> 3;
			*rm = b & 0b00000111;
			if (byte == 0b10001101){
				Mod_reg_rm(file,"lea",byte,index,mod,reg,rm,add);
				return "lea";
			}
			if (byte == 0b11000101){
				Mod_reg_rm(file,"lds",byte,index,mod,reg,rm,add);
				return "lds";
			}
			Mod_reg_rm(file,"les",byte,index,mod,reg,rm,add);
			return "les";
		case LAHF:
			nothing("lahf",byte,index,-1,-1,0,0);
			return "lahf";
		case SAHF:
			nothing("sahf",byte,index,-1,-1,0,0);
			return "sahf";
		case PUSHF:
			nothing("pushf",byte,index,-1,-1,0,0);
			return "pushf";
		case POPF:
			nothing("popf",byte,index,-1,-1,0,0);
			return "popf";
		case AAA:
			nothing("aaa",byte,index,-1,-1,0,0);
			return "aaa";
		case BAA:
			nothing("baa",byte,index,-1,-1,0,0);
			return "baa";
		case AAS:
			nothing("aas",byte,index,-1,-1,0,0);
			return "aas";
		case DAS:
			nothing("das",byte,index,-1,-1,0,0);
			return "das";
		case AAM1:
			fread(&b,sizeof(b),1,file);
			*add = 1;
			if (b == AAM1) {
				nothing("aam",byte,index,b,b,1,1);
				return "aam";
			}
			return "";
		case AAD1:
			fread(&b,sizeof(b),1,file);
			*add = 1; 
			if (b == AAD2) {
				nothing("aad",byte,index,b,b,1,1);
				return "aad";
			}
			/*TODO*/
			/*SHL/SAL/SHR/SAR/ROL/ROR/RCL/RCR*/
			return "";
		case CBW:
			nothing("cbw",byte,index,-1,-1,0,0);
			return "cbw";
		case CWD:
			nothing("cwd",byte,index,-1,-1,0,0);
			return "cwd";
		case INT1:
			fread(&b,sizeof(b),1,file);
			*add = 1;
			nothing("int",byte,index,b,b,1,1);
			return "int";
		case JNB:
			jump(file,"jnb",byte,index,add);
			return "jnb";
		case CLC:
			nothing("clc",byte,index,-1,-1,0,0);
			return "clc";
		case STC:
			nothing("stc",byte,index,-1,-1,0,0);
			return "stc";
		case CLI:
			nothing("cli",byte,index,-1,-1,0,0);
			return "cli";
		case STI:
			nothing("sti",byte,index,-1,-1,0,0);
			return "sti";
		case CLD:
			nothing("cld",byte,index,-1,-1,0,0);
			return "cld";
		case STD:
			nothing("std",byte,index,-1,-1,0,0);
			return "std";
		case CMC:
			nothing("cmc",byte,index,-1,-1,0,0);
			return "cmc";
		case HLT:
			nothing("hlt",byte,index,-1,-1,0,0);
			return "hlt";
		case WAIT:
			nothing("wait",byte,index,-1,-1,0,0);
			return "wait";
		case LOCK:
			nothing("lock",byte,index,-1,-1,0,0);
			return "lock";
		case JE:
			jump(file,"je",byte,index,add);
			return "jnb";
		case JL:
			jump(file,"jl",byte,index,add);
			return "jnb";
		case JLE:
			jump(file,"jle",byte,index,add);
			return "jnb";
		case JB:
			jump(file,"jb",byte,index,add);
			return "jnb";
		case JBE:
			jump(file,"jbe",byte,index,add);
			return "jnb";
		case JP:
			jump(file,"jp",byte,index,add);
			return "jnb";
		case JO:
			jump(file,"jo",byte,index,add);
			return "jnb";
		case JS:
			jump(file,"js",byte,index,add);
			return "jnb";
		case JNE:
			jump(file,"jne",byte,index,add);
			return "jnb";
		case JNL:
			jump(file,"jnl",byte,index,add);
			return "jnb";
		case JNLE:
			jump(file,"jnle",byte,index,add);
			return "jnb";
		case JNBE:
			jump(file,"jnbe",byte,index,add);
			return "jnb";
		case JNP:
			jump(file,"jnp",byte,index,add);
			return "jnb";
		case JNO:
			jump(file,"jno",byte,index,add);
			return "jnb";
		case JNS:
			jump(file,"jns",byte,index,add);
			return "jnb";
		case LOOP:
			jump(file,"loop",byte,index,add);
			return "jnb";
		case LOOPZ:
			jump(file,"loopz",byte,index,add);
			return "jnb";
		case LOOPNZ:
			jump(file,"loopnz",byte,index,add);
			return "jnb";
		case JCXZ:
			jump(file,"jxcz",byte,index,add);
			return "jnb";
		case INT2:
			nothing("int",byte,index,-1,-1,0,0);
			return "int";
		case IRET:
			nothing("iret",byte,index,-1,-1,0,0);
			return "iret";
		case INTO:
			nothing("into",byte,index,-1,-1,0,0);
			return "into";
		case JMP1:
			fread(&data,sizeof(uint8_t),1,file);
			int temp = data;
			fread(&data,sizeof(uint8_t),1,file);
			data = (data << 8) + temp;
			*add = 2;
			nothing("jmp",byte,index,data,index+3+data,0,2);
			return "jmp";
		case JMP2:
			//fread(&data,sizeof(uint8_t),1,file);
			//*add = 1;
			//nothing("jmp short",byte,index,data,index+2+data,1,1);
			jump(file,"jmp short",byte,index,add);
			return "jmp";
		case CALL3:
		case JMP4:
			char* inst;
			if (byte == CALL3)
				inst = "call";
			else
				inst = "jmp";
			fread(&data,sizeof(uint8_t),1,file);
			temp = data << 8;
			fread(&data,sizeof(uint8_t),1,file);
			data = (data << 8) + temp;
			int data2;
			fread(&data2,sizeof(uint8_t),1,file);
			temp = data2 << 8;
			fread(&data2,sizeof(uint8_t),1,file);
			data2 = (data2 << 8) + temp;
			*add = 4;
			char* str = "";
			char* t;
			asprintf(&t,"%s%s",memPrint(data,2,0,0,0,1),memPrint(data2,2,0,0,0,1));
			asprintf(&str,"%04x: %s", index, pretty(byte,0,t,0,0,0,1));
			printf("%s %s %s:%s\n", str, inst, memPrint(data,2,1,1,0,1),memPrint(data2,2,1,1,0,1));
			return "jmp";
		case RET3:
		case RET1:
			nothing("ret",byte,index,0,0,0,0);
			return "ret";
		case RET4:
		case RET2:
			fread(&data,sizeof(uint8_t),1,file);
			temp = data;
			fread(&data,sizeof(uint8_t),1,file);
			*add = 2;
			data = (temp << 8) + data;
			nothing("ret",byte,index,data,data,1,2);
			return "ret";
		case CALL1:
			fread(&data,sizeof(uint8_t),1,file);
			temp = data;
			fread(&data,sizeof(uint8_t),1,file);
			data = (data << 8) + temp;
			*add = 2;
			int ind = index + 3;
			if ((data & 0b1000000000000000) == 0b1000000000000000)
				ind -= (0b1111111111111111 - data) + 1;
			else
				ind += data;
			nothing("call",byte,index,data,ind,0,2);
			return "call";
	}
	return "";
}

//--------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc < 2)
        errx(1, "Argument missing: ./main <a.out>");

    char * filename = argv[1];
    FILE* file = fopen(filename, "r");

    uint32_t header;
    fread(&header, sizeof(header), 1, file);
    fread(&header, sizeof(header), 1, file);

    uint32_t length;
    fread(&length, sizeof(length), 1, file);
	uint32_t a_data;
    fread(&a_data, sizeof(a_data), 1, file);

    fread(&header, sizeof(header), 1, file);
    fread(&header, sizeof(header), 1, file);

    fread(&header, sizeof(header), 1, file);
    fread(&header, sizeof(header), 1, file);

    uint8_t byte1;
    for (uint32_t i = 0; i < length;i++)
    {
        fread(&byte1, sizeof(byte1), 1, file);
		if (length - i == 1){
			undefined(byte1,i);
		}
		else{
			int d = 0, w = 0, mod = 0, reg = 0, rm = 0;
			int data1 = 0;
			uint32_t add = 0;
			char* instruct;
			instruct = instruct0(file,i,byte1,&mod,&reg,&rm,&add);
			if (instruct[0] != '\0'){
				i += add;
				continue;
			}
			instruct = instruct1(file,i,byte1,&d,&w,&mod,&reg,&rm,&data1,&add);
			if (instruct[0] != '\0'){
				i += add;
				continue;
			}
			instruct = instruct2(file,i,byte1,&d,&w,&mod,&reg,&rm,&add);
			if (instruct[0] != '\0'){
				i += add;
				continue;
			}
			instruct = instruct3(file, i,byte1,&w,&mod,&reg,&rm,&add);
			if (instruct[0] != '\0'){
				i += add;
				continue;
			}
			instruct = instruct4(file,i,byte1,&w,&reg,&data1,&add);
			if (instruct[0] != '\0'){
				i += add;
				continue;
			}
			instruct = instruct5(i,byte1,&add);
			if (instruct[0] != '\0'){
				i += add;
				continue;
			}
		}
    }
	
    return 0;
}
