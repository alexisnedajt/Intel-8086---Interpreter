#include "main.h"

uint8_t* text_area;
uint8_t* data_area;
uint16_t registers[8] = {0,0,0,0,0xffdc,0,0,0};
uint8_t current = 0;

char CF = '-';
char SF = '-';
char ZF = '-';
char OF = '-';

int tem = 0;

char flags[5] = {'-','C','S','Z','O'};

char* val[2][8] = {
	{"al","cl","dl","bl","ah","ch","dh","bh"},
	{"ax","cx","dx","bx","sp","bp","si","di"}
};

//---------------------------------------------------------------------------------------

char* pretty(int memory, int byte, int byte2, char* memPrint, int disp, int size, int yes, int swap){
	char* s = ""; 
	asprintf(&s,"%s%04x ",s,registers[0]);
	asprintf(&s,"%s%04x ",s,registers[3]);
	asprintf(&s,"%s%04x ",s,registers[1]);
	asprintf(&s,"%s%04x ",s,registers[2]);
	asprintf(&s,"%s%04x ",s,registers[4]);
	for (int i = 5; i < 8; i++){
		asprintf(&s,"%s%04x ",s,registers[i]);
	}
	asprintf(&s,"%s%c%c%c%c ",s,CF,SF,ZF,OF);
	asprintf(&s,"%s%04x:",s,memory);
	asprintf(&s,"%s%02x",s,byte);
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
	asprintf(&s,"%-62s",s);
	return s;
}

char* r_m(instruct* inst, int size){
	char* ans;
	char* op = "+";
	if ((inst->rm) == 0b000){
		inst->add = registers[3] + registers[6];
		ans = "bx+si";
	}
	else if ((inst->rm) == 0b001){
		inst->add = registers[3] + registers[7];
		ans = "bx+di";
	}
	else if ((inst->rm) == 0b010){
		inst->add = registers[5] + registers[6];
		ans = "bp+si";
	}
	else if ((inst->rm) == 0b011){
		inst->add = registers[5] + registers[7];
		ans = "bp+di";
	}
	else if ((inst->rm) == 0b100){
		inst->add = registers[6];
		ans = "si";
	} 
	else if ((inst->rm) == 0b101){
		inst->add = registers[7];
		ans = "di";
	} 
	else if ((inst->rm) == 0b110){
		inst->add = registers[5];
		ans = "bp";
	} 
	else{
		inst->add = registers[3];
		ans = "bx";	
	}
	if (size == 1){
		inst->disp = inst->disp & 0x00ff;
		if ((inst->disp & 0b10000000) == 0b10000000){
			op = "-";
			inst->disp = 0b11111111 - inst->disp + 1;
			inst->add -= inst->disp;
		}
		else
			inst->add += inst->disp;
		if (inst->disp < 0b10000){
			asprintf(&ans, "[%s%s%01x]",ans,op,inst->disp);
		}
		else{
			asprintf(&ans, "[%s%s%02x]",ans,op,inst->disp);
		}
		inst->type = Add2;
	}
	else if (size == 2){
		if ((inst->disp & 0b1000000000000000) == 0b1000000000000000){
			op = "-";
			inst->disp = 0b1111111111111111 - inst->disp + 1;
			inst->add -= inst->disp;
		}
		else
			inst->add += inst->disp;
		if (inst->disp < 0b10000){
			asprintf(&ans, "[%s%s%01x]",ans,op,inst->disp);
		}
		else if (inst->disp < 0b100000000){
			asprintf(&ans, "[%s%s%02x]",ans,op,inst->disp);
		}
		else if (inst->disp < 0b1000000000000){
			asprintf(&ans, "[%s%s%03x]",ans,op,inst->disp);
		}
		else
			asprintf(&ans, "[%s%s%04x]",ans,op,inst->disp);
		inst->type = Add2;
	}
	else
		asprintf(&ans, "[%s]",ans);
	inst->new_data = data_area[inst->add];
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
			if ((data & 0x8000) == 0x8000){
					data = 0xffff - data + 1;
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

//---------------------------------------------------------------------------------------

void Reg_Imm(instruct* inst){
	printf(" ;[%04x]",inst->add);
	printf("%02x%02x",data_area[inst->add+1],data_area[inst->add]);
}

void Reg_Reg(){

}

void Add_Imm(instruct* inst){
	printf(" ;[%04x]",inst->add);
	printf("%02x%02x",data_area[inst->new_data + 1],data_area[inst->new_data]);
}

void Set_Registers(instruct* inst, int val){
	if (inst->type == Add2)
		Reg_Imm(inst);
	if (inst->w){
		registers[inst->data] = val;
	}
	else if(inst->add/4)
		registers[inst->data] = (registers[inst->data] & 0x00ff) + (val<<8);
	else
		registers[inst->data] = (registers[inst->data] & 0xff00) + val;
}

//---------------------------------------------------------------------------------------

void D_w_mod_reg_rm(int byte, int memory, struct instruct* inst){
	inst->rg = val[inst->w][inst->reg];
	inst->data = inst->reg;
	uint8_t a = ((inst->mod) << 6) + ((inst->reg) << 3) + inst->rm;
	int disp = 0;
	uint8_t dis8;
	int s = 0;
	if (inst->mod == 0b01){
		current++;
		dis8 = text_area[current];
		disp = (int16_t)(dis8);
		s = 1;
		inst->disp = disp;
		inst->ad = r_m(inst,s);
	}
	else if (inst->mod == 0b10){
		current++;
		dis8 = text_area[current];
		disp = (int8_t)(dis8);
		current++;
		dis8 = text_area[current];
		disp = ((int8_t)(dis8) << 8) + disp;
		s = 2;
		inst->disp = disp;
		inst->ad = r_m(inst,s);
	}
	else if (inst->mod == 0b00 ){
		if (inst->rm == 0b110){
			current++;
			dis8 = text_area[current];
			disp = (int8_t)(dis8);
			current++;
			dis8 = text_area[current];
			disp = ((int8_t)(dis8) << 8) + disp;
			asprintf(&inst->ad,"[%04x]",disp);
			s = 2 ;
		}
		else{
			inst->disp = 0;
			inst->ad = r_m(inst,0);
		}
	}
	else if (inst->mod == 0b11){
		inst->new_data = inst->rm;
		inst->ad = val[inst->w][inst->rm];
	}
	if (inst->d == 0){
		char** temp = malloc(sizeof(char*));
		*temp = inst->rg;
		inst->rg = inst->ad;
		inst->ad = *temp;
		free(temp);
		if(inst->type == Add2)
			inst->type = Add1;
		int t = inst->data;
		inst->data = inst->new_data;
		inst->new_data = t;
	}
	printf("%s %s %s, %s", pretty(memory,byte,a,"",disp,s,1,1), inst->name, inst->rg, inst->ad);
	return;
}

void W_mod_rm(int byte, int memory, struct instruct* inst, int size){
	char* r = "";
	int disp = -1;
	int s = 1;
	uint8_t dis8;
	int d = 0;
	int sawp = 1;
	if (inst->mod == 0b01){
		current++;
		dis8 = text_area[current];
		if (inst->data == -1){
			disp = dis8;
			inst->data = dis8;
			d = bswap_16(inst->data);
		}
		else {
			disp = inst->data;
			inst->data = dis8;
			d = inst->data;
		}
		if (size == 2){
			d = disp & 0x00ff;
			disp >>= 8;
			inst->data = (d << 8) + dis8;
			d = bswap_16(inst->data);
		}
		s = 1;
		inst->disp = disp;
		r = r_m(inst,s);
	}
	else if (inst->mod == 0b10){
		current++;
		dis8 = text_area[current];
		disp = (int8_t)(dis8);
		current++;
		dis8 = text_area[current];
		disp = ((int8_t)(dis8) << 8) + disp;
		d = inst->data;
		s = 2;
		inst->disp = disp;
		r = r_m(inst,s);
	}
	else if (inst->mod == 0b00){
		if (inst->rm == 0b110){
			current++;
			dis8 = text_area[current];
			disp = (int8_t)(dis8);
			current++;
			dis8 = text_area[current];
			disp = ((int8_t)(dis8) << 8) + disp;
			d = inst->data;
			inst->data = disp;
			disp = d;
			d = inst->data;
			asprintf(&r,"[%04x]",bswap_16(disp));
			s = 1;
		}
		else {
			inst->disp = inst->data;
			d = -1;
			r = r_m(inst,0);
		}
	}
	else if (inst->mod == 0b11){
		r = val[inst->w][inst->rm];
		d = inst->data;
		if (strcmp(inst->name,"or") == 0 || strcmp(inst->name,"and") == 0 || strcmp(inst->name,"test") == 0){
			sawp = 0;
		}
	}
	if (strcmp(inst->name,"mov") == 0){
		if (d == -1){
			inst->data = bswap_16(disp);
			size = 1;
			s = 0;
		}
		else{
			inst->data = (d << 8) + dis8;
		}
		if (size > 1)
			size = 1;
		d = bswap_16(inst->data)  ;
	}
	char* mprint = memPrint(inst->data,size,1,1,0,s);
	if (mprint[0] != '\0'){
		asprintf(&r,"%s,",r);
	}
	if (d == -1)
		size = 0;
	printf("%s %s %s %s",pretty(memory,byte,(inst->mod << 6) + (inst->reg << 3) + inst->rm,memPrint(disp,1,0,0,0,1),d,size,1,sawp), inst->name, r, mprint);
	return;
}

void W_reg(int byte, int memory, struct instruct* inst, int size){
	printf("%s %s %s, %s", pretty(memory,byte,0,memPrint(inst->data,size,0,0,0,1),0,0,0,1), inst->name, val[(byte & 0b00001000) >> 3][byte & 0b00000111], memPrint(inst->data,size,1,1,0,0));
	return;
}

void W(int byte, int memory, struct instruct* inst, int size){
	printf("%s %s %s", pretty(memory,byte,0,memPrint(inst->data,size,0,0,0,1),0,0,0,1), inst->name, memPrint(inst->data,size,1,1,0,1));
	return;
}

void Mod_reg_rm(int byte, int memory, struct instruct* inst){
	char* first = val[1][inst->reg];
	char* second;
	int a = ((inst->mod) << 6) + ((inst->reg) << 3) + inst->rm;
	int disp = 0;
	int s = 0;
	uint8_t dis8;
	if (inst->mod == 0b01){
		current++;
		dis8 = text_area[current];
		disp = dis8;
		s = 1;
		inst->disp = disp;
		asprintf(&second,"%s",r_m(inst,s));
	}
	else if (inst->mod == 0b10){
		current++;
		dis8 = text_area[current];
		disp = (int8_t)(dis8);
		current++;
		dis8 = text_area[current];
		disp = ((int8_t)(dis8) << 8) + disp;
		s = 2;
		inst->disp = disp;
		asprintf(&second,"%s",r_m(inst,s));
	}
	else if (inst->mod == 0b00){
		if (inst->rm == 0b110){
			current++;
			dis8 = text_area[current];
			disp = (int8_t)(dis8);
			current++;
			dis8 = text_area[current];
			disp = ((int8_t)(dis8) << 8) + disp;
			asprintf(&second,"[%04x]",disp);
			s = 2 ;
		}
		else{
			inst->disp = 0;
			second = r_m(inst,0);
		}
	}
	if (inst->mod == 0b11){
		asprintf(&second,"%s",val[1][inst->rm]);
	}
	printf("%s %s %s, %s",pretty(memory,byte,a,"",disp,s,1,1), inst->name, first, second);
	return;
}

void Reg(int byte, int memory, struct instruct* inst){
	char* op = "";
	if (strcmp(inst->name,"xchg") == 0)
		op = ", ax";
	inst->reg = byte & 0b00000111;
	inst->rm = inst->reg;		
	printf("%s %s %s %s",pretty(memory,byte,0,"",0,0,0,1), inst->name, val[1][byte & 0b00000111], op);
	return;
}

void Mod_rm(int byte, int memory, struct instruct* inst, int size){
	char* first;
	int a = ((inst->mod) << 6) + ((inst->mod) << 3) + inst->rm;
	int disp = 0;
	uint8_t dis8;
	int s = 0;
	if (inst->mod == 0b01){
		current++;
		dis8 = text_area[current];
		disp = (int8_t)(dis8);
		s = 1;
		inst->disp = disp;
		first = r_m(inst,s);
	}
	else if (inst->mod == 0b10){
		current++;
		dis8 = text_area[current];
		disp = (int8_t)(dis8);
		current++;
		dis8 = text_area[current];
		disp = ((int8_t)(dis8) << 8) + disp;
		s = 2;
		inst->disp = disp;
		first = r_m(inst,s);
	}
	else if (inst->mod == 0b00 ){
		if (inst->rm == 0b110){
			current++;
			dis8 = text_area[current];
			disp = (int8_t)(dis8);
			current++;
			dis8 = text_area[current];
			disp = ((int8_t)(dis8) << 8) + disp;
			asprintf(&first,"[%04x]",disp);
			s = 2 ;
		}
		else{
			s = 0;
			inst->disp = disp;
			first = r_m(inst,s);
		}
	}
	if (inst->mod == 0b11)
		first = val[1][inst->rm];
	char* mprint = memPrint(inst->data,size,1,1,0,1);
	if (mprint[0] != '\0'){
		asprintf(&first,"%s,",first);
	}
	printf("%s %s %s %s",pretty(memory,byte,a, memPrint(inst->data,size,0,0,0,1),disp,s,1,1), inst->name, first, mprint);
	return;
}

void S_w_mod_rm(int byte, int memory, struct instruct* inst, int size){
	uint8_t a = ((inst->mod) << 6) + ((inst->reg) << 3) + inst->rm;
	int disp = 0;
	int s = 0;
	uint8_t dis8;
	int d = inst->data;
	int ds = 0;
	if (inst->mod == 0b01){
		current++;
		dis8 = text_area[current];
		disp = inst->data;
		if (size == 2){
			disp = (disp & 0xff00) >> 8;
		}
		s = 1;
		ds = dis8;
		d = inst->data;
		if (size == 2)
			inst->data = ((inst->data & 0x00ff) << 8) + dis8;
		else
			inst->data = dis8;
		inst->disp = disp;
		inst->rg = r_m(inst,s);
	}
	else if (inst->mod == 0b10){
		current++;
		dis8 = text_area[current];
		disp = (dis8 << 8) + inst->data;
		current++;
		dis8 = text_area[current];
		d = disp;
		inst->data = dis8;
		s = 2;
		ds = dis8;
		inst->disp = disp;
		inst->rg = r_m(inst,s);
	}
	else if (inst->mod == 0b00 ){
		if (inst->rm == 0b110){
			current++;
			dis8 = text_area[current];
			disp = (dis8 << 8) + inst->data;
			current++;	
			dis8 = text_area[current];	
			d = disp;
			ds = dis8;
			inst->data = dis8;
			asprintf(&inst->rg,"[%04x]",disp);
			s = 2 ;
			inst->type = Imm;
		}
		else{
			inst->disp = 0;
			inst->rg = r_m(inst,0);
			s = 0;
			d = inst->data;
		}
	}
	else if (inst->mod == 0b11){
		d = inst->data;
		inst->rg = val[inst->w][inst->rm];
	}
	int neg = 0;
	if (strcmp(inst->name,"cmp") == 0 || strcmp(inst->name,"cmp byte") == 0){
		if ((inst->data & 0x80) == 0x80)
			inst->data = 0xff00 + inst->data;
		else
			inst->data = 0x0000 + inst->data;
		neg = 1;
	}
	inst->ad = memPrint(inst->data,size,1,1,neg,1);
	printf("%s %s %s, %s",pretty(memory,byte,a,memPrint(d,size,0,0,0,1),ds,s,1,1), inst->name, inst->rg, inst->ad);
	inst->new_data = d;
	return;
}

void W_mod_reg_rm(int byte, int memory, struct instruct* inst){
	char* second = val[inst->w][inst->reg];
	char* first;
	int a = ((inst->mod) << 6) + ((inst->reg) << 3) + inst->rm;
	int disp = 0;
	int size = 0;
	uint8_t dis8;
	if (inst->mod == 0b01){
		current++;
		dis8 = text_area[current];
		disp = (int8_t)(dis8);
		size = 1;
		inst->disp = inst->mod;
		first = r_m(inst,size);
	}
	else if (inst->mod == 0b10){
		current++;
		dis8 = text_area[current];
		disp = (int8_t)(dis8);
		current++;
		dis8 = text_area[current];
		disp = ((int8_t)(dis8) << 8) + disp;
		size = 2;
		inst->disp = inst->mod;
		first = r_m(inst,size);
	}
	else if (inst->mod == 0b00){
		if (inst->rm == 0b110){
			current++;
			dis8 = text_area[current];
			disp = dis8;
			current++;
			dis8 = text_area[current];
			disp = ((dis8) << 8) + disp;
			asprintf(&first,"[%04x]",disp);
			size = 2 ;
		}
		else{
			inst->disp = 0;
			first = r_m(inst,0);
		}
	}
	if (inst->mod == 0b11){
		first = val[inst->w][inst->rm];
	}
	char* di = "";
	if (disp != 0){
		asprintf(&di,"%02x",disp);
	}
	printf("%s %s %s, %s",pretty(memory,byte, a, "", disp,size,1,1), inst->name, first, second);
	return;
}

void nothing(char* inst, int byte, int memory, int data, int disp, int yes, int size){
	char* str;
	asprintf(&str,"%s",pretty(memory,byte,0,memPrint(data,size,1-yes,0,0,1),0,0,0,1));
	disp = disp & 0x0000ffff;
	printf("%s %s %s", str, inst, memPrint(disp,size,yes,1,0,1));
}

void jump(instruct* inst, int memory){
		int data = text_area[current];
		int inc = data;
		if ((data & 0b10000000) == 0b10000000){
			inc = (int8_t)(data);
		}
		char* str;
		asprintf(&str,"%s",pretty(memory,(inst->mod << 6)+(inst->reg << 3)+inst->rm,0,memPrint(data,1,0,0,0,1),0,0,0,1));
		printf("%s %s %04x", str, inst->name, memory+inc+2);
		inst->data = memory+inc+2;
}

void undefined(int byte, int memory){
	char* s;
	asprintf(&s,"%s",pretty(memory,byte,0,"",0,0,0,1));
	printf("%s (undefined)", s);
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

char* instruct5(int index, uint8_t byte, struct instruct* inst) {
	switch(byte>>5){
		case PUSH3:
			current--;
			Reg(byte,index,inst);
			registers[4] -= 2;
			return "push";
	}
	return "";
}

char* instruct4(int index, uint8_t byte, struct instruct* inst) {
	int size;
	switch(byte>>4){
		case MOV3:
			inst->name = "+ mov";
			inst->w = (byte & 0b00001000) >> 3;
			inst->data = (inst->mod << 6) + (inst->reg << 3) + inst->rm;
			inst->reg = byte & 0b00000111;
			size = 1;
			if (inst->w == 1){
				current++;
				inst->data = inst->data + (text_area[current] << 8);
				size = 2;
			}
			W_reg(byte,index,inst,size);
			registers[inst->reg] = inst->data;
			return "mov";
	}
	return "";
}

char* instruct3(int index, uint8_t byte, struct instruct* inst) {
	switch(byte>>3){
		case ESC:
			inst->name = "esc";
			Mod_rm(byte,index,inst,0);
			return "esc";
		case PUSH2:
		case POP2:
		case XCHG2:
		case INC2:
		case DEC2:
			current--;
			if (byte >> 3 == PUSH2)
				inst->name = "+ push";
			else if (byte >> 3 == POP2)
				inst->name = "+ pop";
			else if (byte >> 3 == XCHG2)
				inst->name = "xchg";
			else if (byte >> 3 == INC2)
				inst->name = "inc";
			Reg(byte,index,inst);
			if (strcmp(inst->name,"+ push") == 0){
				registers[4] -= 2;
				data_area[registers[4]+1] = (registers[inst->rm] & 0xff00) >> 8;
				data_area[registers[4]] = registers[inst->rm] & 0x00ff;
			}
			else if (strcmp(inst->name,"+ pop") == 0){
				registers[inst->rm] = data_area[registers[4]];
				registers[inst->rm] += data_area[registers[4]+1] << 8;
				registers[4] += 2;
			}
			return "dec";
	}
	return "";
}	

char* instruct2(int index, uint8_t byte, struct instruct* inst) {
	switch(byte>>2){
		case MOV1:
		case ADD1:
		case ADC1:
		case SUB1:
		case SSB1:
		case CMP1:
		case AND1:
		case OR1:
		case XOR1:
			if (byte>>2 == MOV1)
				inst->name = "++ mov";
			else if (byte>>2 == ADD1)
				inst->name = "+ add";
			else if (byte>>2 == ADC1)
				inst->name = "adc";
			else if (byte>>2 == SUB1)
				inst->name = "sub";
			else if (byte>>2 == SSB1)
				inst->name = "sbb";
			else if (byte>>2 == OR1)
				inst->name = "+ or";
			else if (byte>>2 == XOR1)
				inst->name = "xor";
			else
				inst->name = "and";
			D_w_mod_reg_rm(byte, index, inst);
			if (strcmp(inst->name,"++ mov") == 0){
				if(inst->type == Norm){
					Set_Registers(inst,registers[inst->new_data]);
				}
				else if (inst->type == Add2){
					Set_Registers(inst,(data_area[inst->add+1] << 8) + data_area[inst->add]);
				}
				else if (inst->type == Add1){
					Reg_Imm(inst);
					data_area[inst->add] = registers[inst->new_data];
				}
			}
			else if (strcmp(inst->name,"+ or") == 0){
				if(inst->type == Norm){
					registers[inst->data] = registers[inst->data] | registers[inst->new_data];
					SF = flags[((data_area[inst->new_data] & 0x80) == 0x80) * 2];
					ZF = flags[(data_area[inst->new_data] == 0) * 3];
				}
			}
			else if (strcmp(inst->name,"+ add") == 0){
				if(inst->type == Norm)
					Set_Registers(inst,registers[inst->data] + registers[inst->new_data]);
				else if (inst->type == Add2)
					Set_Registers(inst,registers[inst->data] + (data_area[inst->add+1] << 8) + data_area[inst->add]);
				else if (inst->type == Add1){
					Reg_Imm(inst);
					data_area[inst->add] += registers[inst->new_data];
				}
			}
			return "xor";
		case ADD2:
			current++;
			inst->data = text_area[current];
			if (inst->d == 0 && inst->w == 1){
				current++;
				inst->data = (inst->data <<8) + text_area[current];
			}
			if (inst->reg == 0b000)
				inst->name = "add";
			else if (inst->reg == 0b010)
				inst->name = "adc";
			else if (inst->reg == 0b101)
				inst->name = "+ sub";
			else if (inst->reg == 0b011)
				inst->name = "sbb";
			else if (inst->w == 1)
				inst->name = "+ cmp";
			else
				inst->name = "cmp byte";
			S_w_mod_rm(byte,index,inst,1);
			if (strcmp(inst->name,"+ sub") == 0){
				if (inst->type == Imm){
					data_area[inst->new_data] -= inst->data;
					SF = flags[((data_area[inst->new_data] & 0x80) == 0x80) * 2];
					ZF = flags[(data_area[inst->new_data] == 0) * 3];
					Add_Imm(inst);
				}
				else{
					registers[inst->rm] -= inst->data;
					SF = flags[((registers[inst->rm] & 0x80) == 0x80) * 2];
					ZF = flags[(registers[inst->rm] == 0) * 3];
				}
			}
			else if (strcmp(inst->name,"+ cmp") == 0){
				int comp = data_area[inst->new_data] - inst->data;
				SF = flags[((comp & 0x80) == 0x80) * 2];
				ZF = flags[(comp == 0) * 3];
				Add_Imm(inst);
			}
			return "cmp";
		case SHL:
			if (inst->reg == 0b100)
				inst->name = "shl";
			else if (inst->reg == 0b101)
				inst->name = "shr";
			else if (inst->reg == 0b111)
				inst->name = "sar";
			else if (inst->reg == 0b000)
				inst->name = "rol";
			else if (inst->reg == 0b001)
				inst->name = "ror";
			else if (inst->reg == 0b010)
				inst->name = "rcl";
			else if (inst->reg == 0b011)
				inst->name = "rcr";
			char* str = "1";
			if (inst->d == 1)
				asprintf(&str,"cl");
			char* r = val[inst->w][inst->rm];
			int disp = 0;
			int s = 0;
			uint8_t dis8;
			if (inst->mod == 0b01){
				current++;
				dis8 = text_area[current];
				disp = (int8_t)(dis8);
				s = 1;
			}
			else if (inst->mod == 0b10){
				current++;
				dis8 = text_area[current];
				disp = (int8_t)(dis8);
				current++;
				disp = ((int8_t)(text_area[current]) << 8) + disp;
				s = 2;
			}
			if (inst->mod != 0b11){
				r = r_m(inst,s);
			}
			printf("%s %s %s, %s",pretty(index,byte,(inst->mod << 6) + (inst-> reg << 3) + inst->reg,memPrint((inst->mod << 6) + (inst-> reg << 3) + inst->reg,1,0,0,0,1),disp,s,0,1), inst->name, r, str);
			return "shl";
	}			
	return "";
}

char* instruct1(int index, uint8_t byte, struct instruct* inst) {
	int size = 0;
	char* str = "";
	char* st;
	char* s;
	switch(byte>>1) {
		case INC1:
			if (inst->reg == 0b001){
					inst -> name = "dec";
					W_mod_rm(byte,index,inst,0);
					return "dec";
			}
			return "";
		case REP:
			char* ins = Inst((inst->mod << 6) + (inst->reg << 3) + inst->rm);
			asprintf(&s,"%s",pretty(index,byte,0,memPrint((inst->mod << 6) + (inst->reg << 3) + inst->rm,1,0,0,0,1),0,0,0,1));
			printf("%s %s %s\n", s, "rep", ins);
			return "rep";
		case CMPS:
		case SCAS:
		case LODS:
		case STOS:
		case MOVS:
			current--;
			if (byte >> 1 == CMPS)
				inst->name = "cmps";
			else if (byte >> 1 == SCAS)
				inst->name = "scas";
			else if (byte >> 1 == LODS)
				inst->name = "lods";
			else if (byte >> 1 == STOS)
				inst->name = "stos";
			else
				inst->name = "movs";
			if (inst->w == 0){
				asprintf(&str,"%sb",str);
				nothing("movsw",byte,index,-1,-1,0,0);
			}
			else{
				asprintf(&str,"%sw",str);
				nothing("movsb",byte,index,-1,-1,0,0);
			}
			return "movs";
		case MOV2:
			if (inst->reg == 0b00000000){
				inst->name = "mov byte";
				current++;
				inst->data = text_area[current];
				size = 1;
				if (inst->w == 1) {
					inst->name = "mov";
					current++;
					inst->data = (inst->data << 8) + text_area[current];
					size = 2;
				}
				W_mod_rm(byte,index,inst,size);
				return "mov";
			}
			return "";
		case XCHG1:
			W_mod_reg_rm(byte,index,inst);
			return "xchg";
		case IN1:
		case OUT1:
			st = "al";
			if (inst->w == 1)
				st = "ax";
			if (byte >> 1 == IN1)
				str = "in";
			else
				str = "out";
			asprintf(&s,"%s",pretty(index,byte,(inst->mod << 6) + (inst->reg << 3) + inst->rm,memPrint((inst->mod << 6) + (inst->reg << 3) + inst->rm,1,1,1,0,1),0,0,0,1));
			printf("%s %s %s, %s\n", s, str, st, memPrint((inst->mod << 6) + (inst->reg << 3) + inst->rm,1,1,1,0,1));
			return "int";
		case IN2:
		case OUT2:
			current--;
			st = "al";
			if (inst->w == 1)
				st = "ax";
			if (byte >> 1 == IN2)
				str = "in";
			else
				str = "out";
			asprintf(&s,"%s",pretty(index,byte,0,"",0,0,0,1));
			printf("%s %s %s, dx\n", s, str, st);
			return "out";
		case ADD3:
		case ADC3:
		case SUB3:
		case SSB3:
		case AND3:
		case TEST3:
		case OR3:
		case XOR3:
		case CMP3:
			inst->data = (inst->mod << 6) + (inst->reg << 3) + inst->rm;
			size = 1;
			if(byte >> 1 == ADC3)
				inst->name = "adc";
			else if(byte >> 1 == SUB3)
				inst->name = "sub ax,";
			else if(byte >> 1 == SSB3)
				inst->name = "ssb";
			else if(byte >> 1 == CMP3)
				inst->name = "cmp";
			else{
				if (byte >> 1 == OR3)
					inst->name = "or";
				else if (byte >> 1 == XOR3)
					inst->name = "xor";
				else if (byte >> 1 == AND3)
					inst->name = "and";
				else
					inst->name = "test al,";
			}
			if (inst->w == 1) {	
				current++;
				inst->data = (inst->data << 8) + text_area[current];
				size = 2;
			}
			W(byte,index,inst,size);
			return "add";
		case MUL:
		case XOR2:
			if (byte >> 1 == MUL){
				if (inst->reg == 0b100)
					inst->name = "mul";
				else if (inst->reg == 0b101)
					inst->name = "imul";
				else if (inst->reg == 0b110)
					inst->name = "div";
				else if (inst->reg == 0b111)
					inst->name = "idiv";
				else if (inst->reg == 0b011)
					inst->name = "+ neg";
				else if (inst->reg == 0b010)
					inst->name = "not";
				W_mod_rm(byte,index,inst,0);
				if (strcmp(inst->name,"+ neg") == 0){
					registers[inst->rm] = ~registers[inst->rm] + 1;
					SF = flags[((registers[inst->rm] & 0x80) == 0x80) * 2];
					ZF = flags[(registers[inst->rm] == 0) * 3];
				}
				return "not";
			}
			current++;
			inst->data = text_area[current];
			size = 1;
			if (inst->reg == 0b110)
				inst->name = "xor";
			else if (byte >> 1 == MUL && inst->reg == 0b000){
				if (inst->w == 0 && inst->mod != 0b11)
					inst->name = "test byte";
				else
					inst->name = "test";
			}
			else if (inst->reg == 0b001)
				inst->name = "or";
			else if (inst->reg == 0b100)
				inst->name = "and";
			if (inst->name[0] != '\0'){
				if (inst->w == 1){
					current++;
					inst->data = (inst->data << 8) + text_area[current];
					size = 2;
				}
				W_mod_rm(byte,index,inst,size);
			}
			else{
				if (inst->d == 0 && inst->w == 1){
					current++;
					inst->data = (inst->data << 8) + text_area[current];
					size = 2;
				}
				if (inst->reg == 0b000)
					inst->name = "add";
				else if (inst->reg == 0b010)
					inst->name = "adc";
				else if (inst->reg == 0b101)
					inst->name = "sub";
				else if (inst->reg == 0b011)
					inst->name = "sbb";
				else if (inst->reg == 0b111){
					if (inst->w == 1)
						inst->name = "cmp";
					else
						inst->name = "cmp byte";
				}
				S_w_mod_rm(byte,index,inst,size);
			}
			return "cmp";
		case TEST1:
			W_mod_reg_rm(byte,index,inst);
			return "test";
	}
	return "";
}

char* instruct0(int index, uint8_t byte, struct instruct* inst) {
	int data = 0;
	switch(byte) {
		case MOV6:
		case MOV7:
			inst->name = "mov";
			if (inst->reg == 0b0000) {
				Mod_reg_rm(byte,index,inst);
				return "mov";
			}
			return "";
		case PUSH1:
			inst->name = "dec";
			if (inst->reg == 0b001){
				W_mod_rm(byte,index,inst,0);
				return "dec";
			}
			if (inst->reg == 0b000)
				inst->name = "inc";
			else if (inst->reg == 0b110)
				inst->name = "+ push";
			else if ((inst->reg == 0b100) || (inst->reg == 0b101))
				inst->name = "jmp";
			else if ((inst->reg == 0b010) || (inst->reg == 0b011))
				inst->name = "call";
			Mod_rm(byte,index,inst,0);
			if(strcmp(inst->name,"+ push") == 0){
				registers[4] -= 2;
				data_area[registers[4]+1] = data_area[inst->add + 1];
				data_area[registers[4]] = data_area[inst->add];
				Reg_Imm(inst);
			}
			return "jmp";
		case POP1:
			inst->name = "++ pop";
			if (inst->reg == 0b000) {
				Mod_rm(byte,index,inst,0);
				registers[inst->rm] = data_area[registers[4]];
				registers[inst->rm] += data_area[registers[4]+1] << 8;
				registers[4] += 2;
				return "pop";
			}
			return "";
		case XLAT:
			current--;
			nothing("xlat",byte,index,-1,-1,0,0);
			return "xlat";
		case LEA:
		case LDS:
		case LES:
			if (byte == 0b10001101)
				inst->name = "+ lea";
			else if (byte == 0b11000101)
				inst->name = "lds";
			else
				inst->name = "les";
			Mod_reg_rm(byte,index,inst);
			if (strcmp(inst->name,"+ lea") == 0){
				registers[inst->reg] = inst->add;
				Reg_Imm(inst);
			}
			return "les";
		case LAHF:
			current--;
			nothing("lahf",byte,index,-1,-1,0,0);
			return "lahf";
		case SAHF:
			current--;
			nothing("sahf",byte,index,-1,-1,0,0);
			return "sahf";
		case PUSHF:
			current--;
			nothing("pushf",byte,index,-1,-1,0,0);
			return "pushf";
		case POPF:
			current--;
			nothing("popf",byte,index,-1,-1,0,0);
			return "popf";
		case AAA:
			current--;
			nothing("aaa",byte,index,-1,-1,0,0);
			return "aaa";
		case BAA:
			current--;
			nothing("baa",byte,index,-1,-1,0,0);
			return "baa";
		case AAS:
			current--;
			nothing("aas",byte,index,-1,-1,0,0);
			return "aas";
		case DAS:
			current--;
			nothing("das",byte,index,-1,-1,0,0);
			return "das";
		case AAM1:
			if ((inst->mod << 6) + (inst->reg << 3) + inst->rm == AAM1) {
				nothing("aam",byte,index,(inst->mod << 6) + (inst->reg << 3) + inst->rm,(inst->mod << 6) + (inst->reg << 3) + inst->rm,1,1);
				return "aam";
			}
			return "";
		case AAD1:
			if ((inst->mod << 6) + (inst->reg << 3) + inst->rm == AAD2) {
				nothing("aad",byte,index,(inst->mod << 6) + (inst->reg << 3) + inst->rm,(inst->mod << 6) + (inst->reg << 3) + inst->rm,1,1);
				return "aad";
			}
			return "";
		case CBW:
			current--;
			nothing("cbw",byte,index,-1,-1,0,0);
			return "cbw";
		case CWD:
			current--;
			nothing("cwd",byte,index,-1,-1,0,0);
			return "cwd";
		case INT1:
			tem += 1;
			nothing("+ int",byte,index,(inst->mod << 6) + (inst->reg << 3) + inst->rm,(inst->mod << 6) + (inst->reg << 3) + inst->rm,1,1);
			message* m = (message*)(data_area + registers[3]);
			syscalls_l[m->m_type-1](m,data_area);
			if (m->m_type == 4){
				registers[0] = 0;
			}
			if (tem == 2){
				exit(0);
			}
			return "int";
		case JNB:
			inst->name = "jnb";
			jump(inst,index);
			return "jnb";
		case CLC:
			current--;
			nothing("clc",byte,index,-1,-1,0,0);
			return "clc";
		case STC:
			current--;
			nothing("stc",byte,index,-1,-1,0,0);
			return "stc";
		case CLI:
			current--;
			nothing("cli",byte,index,-1,-1,0,0);
			return "cli";
		case STI:
			current--;
			nothing("sti",byte,index,-1,-1,0,0);
			return "sti";
		case CLD:
			current--;
			nothing("cld",byte,index,-1,-1,0,0);
			return "cld";
		case STD:
			current--;
			nothing("std",byte,index,-1,-1,0,0);
			return "std";
		case CMC:
			current--;
			nothing("cmc",byte,index,-1,-1,0,0);
			return "cmc";
		case HLT:
			current--;
			nothing("hlt",byte,index,-1,-1,0,0);
			return "hlt";
		case WAIT1:
			current--;
			nothing("wait",byte,index,-1,-1,0,0);
			return "wait";
		case LOCK:
			current--;
			nothing("lock",byte,index,-1,-1,0,0);
			return "lock";
		case JE:
			inst->name = "+ je";
			jump(inst,index);
			if (ZF == 'Z')
				current = inst->data-1;
			return "jnb";
		case JL:
			inst->name = "jl";
			jump(inst,index);
			return "jnb";
		case JLE:
			inst->name = "jle";
			jump(inst,index);
			return "jnb";
		case JB:
			inst->name = "jb";
			jump(inst,index);
			return "jnb";
		case JBE:
			inst->name = "jbe";
			jump(inst,index);
			return "jnb";
		case JP:
			inst->name = "jp";
			jump(inst,index);
			return "jnb";
		case JO:
			inst->name = "jo";
			jump(inst,index);
			return "jnb";
		case JS:
			inst->name = "js";
			jump(inst,index);
			return "jnb";
		case JNE:
			inst->name = "jne";
			jump(inst,index);
			return "jnb";
		case JNL:
			inst->name = "jnl";
			jump(inst,index);
			return "jnb";
		case JNLE:
			inst->name = "jnle";
			jump(inst,index);
			return "jnb";
		case JNBE:
			inst->name = "jnbe";
			jump(inst,index);
			return "jnb";
		case JNP:
			inst->name = "jnp";
			jump(inst,index);
			return "jnb";
		case JNO:
			inst->name = "jno";
			jump(inst,index);
			return "jnb";
		case JNS:
			inst->name = "jns";
			jump(inst,index);
			return "jnb";
		case LOOP:
			inst->name = "loop";
			jump(inst,index);
			return "jnb";
		case LOOPZ:
			inst->name = "loopz";
			jump(inst,index);
			return "jnb";
		case LOOPNZ:
			inst->name = "loopnz";
			jump(inst,index);
			return "jnb";
		case JCXZ:
			inst->name = "jxcz";
			jump(inst,index);
			return "jnb";
		case INT2:
			current--;
			nothing("int",byte,index,-1,-1,0,0);
			return "int";
		case IRET:
			current--;
			nothing("iret",byte,index,-1,-1,0,0);
			return "iret";
		case INTO:
			current--;
			nothing("into",byte,index,-1,-1,0,0);
			return "into";
		case JMP1:
			inst->data = (inst->mod << 6) + (inst->reg << 3) + inst->rm;
			current++;
			inst->data = inst->data + (text_area[current] << 8);
			nothing("+ jmp",byte,index,inst->data,index+3+inst->data,0,2);
			current = index+2+inst->data;
			return "jmp";
		case JMP2:
			inst->name = "+ jmp short";
			jump(inst,index);
			current = inst->data - 1;
			return "jmp";
		case CALL3:
		case JMP4:
			inst->data = (inst->mod << 6) + (inst->reg << 3) + inst->rm;
			if (byte == CALL3)
				inst->name = "call";
			else
				inst->name = "jmp";
			current++;
			data = (((inst->mod << 6) + (inst->reg << 3) + inst->rm) << 8) + text_area[current];

			current++;
			int data2 = text_area[current];
			current++;
			data2 = (data2 << 8) + text_area[current];

			char* str = "";
			char* t;
			asprintf(&t,"%s%s",memPrint(data,2,0,0,0,1),memPrint(data2,2,0,0,0,1));
			asprintf(&str,"%s",pretty(index,byte,0,t,0,0,0,1));
			printf("%s %s %s:%s\n", str, inst->name, memPrint(data,2,1,1,0,1),memPrint(data2,2,1,1,0,1));
			return "jmp";
		case RET3:
		case RET1:
			current--;
			nothing("+ ret",byte,index,0,0,0,0);
			current = data_area[registers[4]];
			current += data_area[registers[4]+1] << 8;
			current--;
			registers[4] += 2;
			return "ret";
		case RET4:
		case RET2:
			inst->data = (inst->mod << 6) + (inst->reg << 3) + inst->rm;
			current++;
			data = (inst->data << 8) + text_area[current];
			nothing("ret",byte,index,data,data,1,2);
			return "ret";
		case CALL1:
			inst->data = (inst->mod << 6) + (inst->reg << 3) + inst->rm;
			current++;
			data = inst->data + (text_area[current] << 8);
			int ind = index + 3;
			if ((data & 0x8000) == 0x8000)
				ind -= (0b1111111111111111 - data) + 1;
			else
				ind += data;
			inst->disp = ind & 0x0000ffff;
			nothing("+ call",byte,index,data,ind,0,2);
			registers[4] -= 2;
			current++;
			data_area[registers[4]+1] = (current & 0xff00) >> 8;
			data_area[registers[4]] = current & 0x00ff;
			current = inst->disp - 1;
			return "call";
	}
	return "";
}

//--------------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    if (argc != 2)
        errx(1, "Incorrect execution : ./main <a.out>");

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

	text_area = malloc(length*sizeof(uint8_t));
	data_area = malloc(a_data*sizeof(uint8_t));

	for (uint32_t i = 0 ; i< length; i++)
		fread(&text_area[i], sizeof(text_area[i]), 1, file);
	for (uint32_t i = 0 ; i< a_data; i++)
		fread(&data_area[i], sizeof(data_area[i]), 1, file);

	printf(" AX   BX   CX   DX   SP   BP   SI   DI  FLAGS IP\n");

	uint8_t byte, b;
    for (; current < length; current++)
    {
		if (current != 0)
			printf("\n");

		byte = text_area[current];
		current++;
		b = text_area[current];
		struct instruct inst = {"",(byte & 0b00000010) >> 1,byte & 0b00000001,(b & 0b11000000) >> 6,(b & 0b00111000) >> 3,b & 0b00000111,b,0,0,"","",0,0};
		if (instruct0(current-1,byte,&inst)[0] != '\0')
			continue;
		if (instruct1(current-1,byte,&inst)[0] != '\0')
			continue;
		if (instruct2(current-1,byte,&inst)[0] != '\0')
			continue;
		if (instruct3(current-1,byte,&inst)[0] != '\0')
			continue;
		if (instruct4(current-1,byte,&inst)[0] != '\0')
			continue;
		if (instruct5(current-1,byte,&inst)[0] != '\0')
			continue;
    }
	printf("\n");
    return 0;
}
