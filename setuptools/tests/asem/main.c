#include "main.h"

//Disassembler issues with some instructions such as mov, and, cmp, test, or etc....
//Might want to remove them, though the Interpreter works fine

uint8_t* text_area = 0;
uint8_t* data_area = 0;
uint16_t registers[8] = {0,0,0,0,0xffdc,0,0,0};
uint16_t current = 0;

int call_size = 3;

char CF = '-';
char SF = '-';
char ZF = '-';
char OF = '-';

int tm = 0;

char flags[5] = {'-','C','S','Z','O'};

char* val[2][8] = {
	{"al","cl","dl","bl","ah","ch","dh","bh"},
	{"ax","cx","dx","bx","sp","bp","si","di"}
};

//---------------------------------------------------------------------------------------

int pretty(int memory, int byte, int byte2, char* memPrint, int disp, int size, int yes, int swap){
	int total = 52;
	printf("%04x ",registers[0]);
	printf("%04x ",registers[3]);
	printf("%04x ",registers[1]);
	printf("%04x ",registers[2]);
	printf("%04x ",registers[4]);
	for (int i = 5; i < 8; i++){
		printf("%04x ",registers[i]);
	}
	printf("%c%c%c%c %04x:%02x",OF,SF,ZF,CF,memory,byte);
	if (yes == 1){
		printf("%02x",byte2);
		total += 2;
	}
	if (memPrint[0] != '\0'){
		printf("%s",memPrint);
		total += strlen(memPrint);
	}
	if (size == 1){
		disp = disp & 0x00ff;
		printf("%02x",disp);
		total += 2;
	}
	else if (size == 2){
		if (swap == 1){
			printf("%04x",bswap_16(disp));
		}
		else
			printf("%04x",disp);
		total += 4;
	}
	return total;
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
	}
	else
		asprintf(&ans, "[%s]",ans);
	inst->type = Add2;
	inst->new_data = data_area[inst->add];
	return ans;
}

char* memPrint(int data, int size, int yes, int d, int neg, int sh){
	char* s = "";
	char* op = "";
	if (sh == 0){
		if (data != -1)
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
	if (inst->w)
		printf("%02x",data_area[inst->add+1]);
	printf("%02x",data_area[inst->add]);
}

void Reg_Reg(){

}

void Add_Imm(instruct* inst){
	printf(" ;[%04x]",inst->add);
	if (inst->w)
		printf("%02x",data_area[inst->add+1]);
	printf("%02x",data_area[inst->add]);
}

void Set_Registers(instruct* inst, int val){
	//printf("\nval = %04x",val);
	if (inst->type == Add2)
		Reg_Imm(inst);
	//printf("\nw = %i",inst->w);
	if (inst->w){
		registers[inst->data] = val;
	}
	else{ 
		int div = inst->data/4;
		int mod = inst->data%4;
		if(div >= 1)
			registers[mod] = (registers[mod] & 0x00ff) + ((val & 0x00ff)<<8);
		else
			registers[mod] = (registers[mod] & 0xff00) + (val & 0x00ff);
	}
}

void Change_flags(int val1, int val2, char op, int affected){
	int val;
	//printf("\nval1 = %04x",val1);
	//printf("\nval2 = %04x",val2);
	if(op == '+')
		val = val1 + val2;
	else if (op == '-')
		val = val1 - val2;
	else if (op == '&')
		val = val1 & val2;
	else if (op == '|')
		val = val1 | val2;
	else if (op == '^')
		val = val1 ^ val2;
	else if (op == '<')
		val = val1 << val2;
	//OF = flags[((val1 & 0x8000) == 0x8000 && (val2 & 0x8000) == 0x8000 && (val & 0x8000) == 0x0000) || ((val1 & 0x8000) == 0x0000 && (val2 & 0x8000) == 0x0000 && (val & 0x8000) == 0x8000)];
	SF = flags[((val & 0x8000) == 0x8000) * 2];
	ZF = flags[(val == 0) * 3];
	if (affected)
		CF = flags[((op == '-' && val1 < val2) ||((val1 & 0x8000) == 0x8000 && (val2 & 0x8000)==0x8000 && (val & 0x8000) == 0x0000)|| ((val1 & 0x8000) == 0x0000 && (val2 & 0x8000)==0x0000 && (val & 0x8000) == 0x8000))];
	//CF = flags[(((val1 & 0x8000) == 0x0000 && (val2 & 0x8000)==0x0000 && (val & 0x8000) == 0x8000)) || (((val1 & 0x8000) == 0x0000 && (val2 & 0x8000)==0x0000 && (val & 0x8000) == 0x8000))];
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
			inst->type = Imm1;
			inst->add = disp;
		}
		else{
			inst->disp = 0;
			inst->ad = r_m(inst,0);
		}
	}
	else if (inst->mod == 0b11){
		if(strcmp(inst->name,"+cmp") == 0)
			inst->type = RegR;	
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
		else if (inst->type == Imm1)
			inst->type = Imm2;
		int t = inst->data;
		inst->data = inst->new_data;
		inst->new_data = t;
	}
	int size = pretty(memory,byte,a,"",disp,s,1,1);
	for (int i = 0; i< 62-size; i++)
		printf(" ");
	printf(" %s %s, %s", inst->name, inst->rg, inst->ad);
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
			inst->add = bswap_16(disp);
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
	if (strcmp(inst->name,"+++ mov") == 0){
		if (d == -1){
			inst->data = bswap_16(inst->data);
		}
		else{
			inst->data = (d << 8) + dis8;
		}
		size = 1;
		d = bswap_16(inst->data)  ;
		s = 0;
		//inst->data = temp;
		if(inst->data > 0xffff)
			inst->data = inst->data >> 8;
	}
	if(strcmp(inst->name,"+ test") == 0){
		inst->data = bswap_16(inst->data);
	}
	char* mprint = memPrint(inst->data,size,1,1,0,s);
	if (mprint[0] != '\0'){
		asprintf(&r,"%s,",r);
	}
	if (d == -1)
		size = 0;
	char* mp = memPrint(disp,1,0,0,0,0);
	int space = pretty(memory,byte,(inst->mod << 6) + (inst->reg << 3) + inst->rm,mp,d,size,1,sawp);
	for (int i = 0; i< 62-space; i++)
		printf(" ");
	printf(" %s %s", inst->name, r);
	if (mprint[0] != '\0'){
		printf(" %s",mprint);
	}
	return;
}

void W_reg(int byte, int memory, struct instruct* inst, int size){
	int space = pretty(memory,byte,0,memPrint(inst->data,size,1,0,0,1),0,0,0,1);
	for(int i = 0; i<62-space; i++)
		printf(" ");
	printf(" %s %s, %s", inst->name, val[(byte & 0b00001000) >> 3][byte & 0b00000111], memPrint(inst->data,size,1,1,0,0));
	return;
}

void W(int byte, int memory, struct instruct* inst, int size){
	int space = pretty(memory,byte,0,memPrint(inst->data,size,0,0,0,1),0,0,0,1);
	for(int i = 0; i<62-space; i++)
		printf(" ");
	printf(" %s %s", inst->name, memPrint(inst->data,size,1,1,0,1));
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
	int space = pretty(memory,byte,a,"",disp,s,1,1);
	for(int i = 0; i<62-space; i++)
		printf(" ");
	printf(" %s %s, %s", inst->name, first, second);
	return;
}

void Reg(int byte, int memory, struct instruct* inst){
	char* op = "";
	if (strcmp(inst->name,"xchg") == 0)
		op = ", ax";
	inst->reg = byte & 0b00000111;
	inst->rm = inst->reg;		
	int space = pretty(memory,byte,0,"",0,0,0,1);
	for(int i = 0; i<62-space; i++)
		printf(" ");
	printf(" %s %s",inst->name, val[1][byte & 0b00000111]);
	if(op[0] != '\0')
		printf(" %s",op);
	return;
}

void Mod_rm(int byte, int memory, struct instruct* inst, int size){
	char* first;
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
			inst->type = Imm1;
			inst->new_data = disp;
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
	int space = pretty(memory,byte,inst->data, memPrint(inst->data,size,0,0,0,1),disp,s,1,1);
	call_size = (space-50)/2;
	for(int i = 0; i<62-space; i++)
		printf(" ");
	printf(" %s %s", inst->name, first);
	if (mprint[0] != '\0'){
		printf(", %s",mprint);
	}
	return;
}

void S_w_mod_rm(int byte, int memory, struct instruct* inst, int size){
	uint8_t a = ((inst->mod) << 6) + ((inst->reg) << 3) + inst->rm;
	int disp = 0;
	int s = 0;
	uint8_t dis8;
	int swap = 1;
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
			inst->add = disp;
			s = 2 ;
			inst->type = Imm1;
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
	if (strcmp(inst->name,"+ cmp") == 0 || strcmp(inst->name,"cmp byte") == 0){
		if ((inst->data & 0x80) == 0x80)
			inst->data = 0xff00 + inst->data;
		else
			inst->data = 0x0000 + inst->data;
		neg = 1;
		d = bswap_16(inst->data);
	}
	inst->ad = memPrint(inst->data,size,swap,1,neg,1);
	int space = pretty(memory,byte,a,memPrint(d,size,0,0,0,1),ds,s,1,1);
	for(int i = 0; i<62-space; i++)
		printf(" ");
	printf(" %s %s, %s", inst->name, inst->rg, inst->ad);
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
	int space = pretty(memory,byte, a, "", disp,size,1,1);
	for(int i = 0; i<62-space; i++)
		printf(" ");
	printf(" %s %s, %s", inst->name, first, second);
	return;
}

void nothing(char* inst, int byte, int memory, int data, int disp, int yes, int size){
	char* temp = memPrint(data,size,1-yes,0,0,1);
	int space = pretty(memory,byte,0,temp,0,0,0,1);
	call_size = (space-50)/2;
	for(int i = 0; i<62-space; i++)
		printf(" ");
	disp = disp & 0x0000ffff;
	char* m = memPrint(disp,size,yes,1,0,1);
	printf(" %s", inst);
	if (m[0] != '\0')
		printf(" %s",m);
}

void jump(instruct* inst, int memory){
	int data = text_area[current];
	int16_t JmpTo = memory + 2 + data;
	if ((data & 0b10000000) == 0b10000000){
		JmpTo -= data;
		data = (~(data) + 1) & 0xff;
		JmpTo -= data;
	}
	int space = pretty(memory,inst->new_data,0,memPrint(data,1,0,0,0,1),0,0,0,1);
	for(int i = 0; i<62-space; i++)
		printf(" ");
	printf(" %s %04x", inst->name, JmpTo);
	inst->data = JmpTo;
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

void move(instruct* inst){
	if(inst->type == Norm){
		Set_Registers(inst,registers[inst->new_data]);
	}
	else if (inst->type == Imm2){
		Reg_Imm(inst);
		data_area[inst->add] = registers[inst->new_data] & 0x00ff;
		data_area[inst->add + 1] =  registers[inst->new_data] >> 8;
	}
	else if (inst->type == Imm1){
		Reg_Imm(inst);
		registers[inst->data] = data_area[inst->add] + (data_area[inst->add+1] << 8);
	}
	else if (inst->type == Add2){
		Set_Registers(inst,(data_area[inst->add+1] << 8) + data_area[inst->add]);
	}
	else if (inst->type == Add1){
		Reg_Imm(inst);
		data_area[inst->add] = registers[inst->new_data] & 0x00ff;
		data_area[inst->add + 1] =  registers[inst->new_data] >> 8;
	}
}

void sub(instruct* inst){
	if (inst->type == Imm1){
		Add_Imm(inst);
		Change_flags(data_area[inst->new_data],inst->data,'-',1);
		data_area[inst->new_data] -= inst->data;
	}
	else if (inst->type == Imm2){
		printf("\nImm2");
	}
	else if (inst->type == Add1){
		printf("\nAdd1");
	}
	else if (inst->type == Add2){
		Add_Imm(inst);
		Change_flags(registers[inst->reg],data_area[inst->add] + (data_area[inst->add+1]<<8),'-',1);
		registers[inst->reg] -= (data_area[inst->add] + (data_area[inst->add+1]<<8));
	}
	else if (inst->type == Norm){
		Change_flags(registers[inst->rm],inst->data,'-',1);
		registers[inst->rm] -= inst->data;
	}
}

void and(instruct* inst){
	if(inst->type == Add2){
		Add_Imm(inst);
		inst->data = bswap_16(inst->data);
		Change_flags(data_area[inst->add],inst->data,'&',1);
		data_area[inst->add] &= inst->data;
	}	
	else if (inst->type == Norm){
		inst->data = bswap_16(inst->data);
		Change_flags(registers[inst->rm],inst->data,'&',1);
		registers[inst->rm] &= inst->data;
	}
	else if (inst->type == Imm1){
		printf("\nImm1");
	}
	else if (inst->type == Imm2){
		printf("\nImm2");
	}
	else if (inst->type == Add1){
		//Change_flags(data_area[inst->add] + (data_area[inst->add+1] << 8),registers[inst->new_data],'&',1);
		data_area[inst->add] &= (registers[inst->new_data] & 0x00ff);
		data_area[inst->add + 1] &= (registers[inst->new_data] >> 8);
	}
}

void or(instruct* inst){
	if(inst->type == Norm){
		Change_flags(registers[inst->data],registers[inst->new_data],'|',1);
		registers[inst->data] = registers[inst->data] | registers[inst->new_data];
	}
	else if (inst->type == Add2){
		Add_Imm(inst);
		inst->data = bswap_16(inst->data);
		Change_flags(data_area[inst->add],inst->data,'|',1);
		data_area[inst->add] |= (inst->data & 0x00ff);
		data_area[inst->add+1] |= (inst->data >> 8);
	}
	else if (inst->type == Imm1){
		printf("\nImm1");
	}
	else if (inst->type == Imm2){
		printf("\nImm2");
	}
	else if (inst->type == Add1){
		printf("\nAdd1");
	}
}

void xor(instruct* inst){
	OF = '-';
	CF = '-';
	int reg = inst->data;
	int reg2 = inst->new_data;
	if(inst->type == Norm){
		int val2 = registers[inst->data];
		int val = registers[inst->new_data];
		if(inst->w == 0){
			reg = inst->data % 4;
			reg2 = inst->new_data % 4;
			val2 = registers[reg2];
			val = registers[reg];
			if(inst->new_data/4 >= 1){
				val2 = val2 >> 8;
				val = val >> 8;
			}
			else{
				val2 = val2 & 0x00ff;
				val = val & 0x00ff;
			}
		}
		Change_flags(val,val2,'^',0);
		Set_Registers(inst,(val ^ val2));
	}
	else if (inst->type == Add2){
		if(inst->w == 0){
			reg = inst->data % 4;
		}
		Change_flags(registers[reg],((data_area[inst->add+1] << 8) + data_area[inst->add]),'^',0);
		Set_Registers(inst,registers[inst->data] ^ ((data_area[inst->add+1] << 8) + data_area[inst->add]));
	}
	else if (inst->type == Add1){
		Reg_Imm(inst);
		Change_flags((data_area[inst->add+1] << 8) + data_area[inst->add],registers[inst->new_data],'^',0);
		data_area[inst->add] ^= registers[inst->new_data] & 0x00ff;
		data_area[inst->add + 1] ^=  registers[inst->new_data] >> 8;
	}
	else if (inst->type == Imm1){
		printf("\nImm1");
	}
	else if (inst->type == Imm2){
		printf("\nImm2");
	}
}

void add(instruct* inst){
	if(inst->type == Norm){
		Change_flags(registers[inst->data], registers[inst->new_data],'+',1);
		Set_Registers(inst,registers[inst->data] + registers[inst->new_data]);
	}
	else if (inst->type == Imm1){
		Reg_Imm(inst);
		Change_flags(registers[inst->reg],data_area[inst->add] + (data_area[inst->add+1] << 8),'+',1);
		registers[inst->reg] += data_area[inst->add] + (data_area[inst->add+1] << 8);
	}
	else if (inst->type == Imm2){
		printf("\nImm2");
	}
	else if (inst->type == Add2){
		Change_flags(registers[inst->data], (data_area[inst->add+1] << 8) + data_area[inst->add],'+',1);
		Set_Registers(inst,registers[inst->data] + (data_area[inst->add+1] << 8) + data_area[inst->add]);
	}
	else if (inst->type == Add1){
		Reg_Imm(inst);
		Change_flags((data_area[inst->add + 1] << 8) + data_area[inst->add], registers[inst->new_data],'+',1);
		data_area[inst->add] += registers[inst->new_data] & 0x00ff;
		data_area[inst->add + 1] += (registers[inst->new_data] >> 8);
	}
	else{
		Change_flags(registers[inst->rm],inst->data,'+',1);
		inst->new_data = inst->data;
		inst->data = inst->rm;
		Set_Registers(inst,registers[inst->rm] + inst->new_data);
	}
}

void test(instruct* inst){
	if (inst->type == Norm){
		Change_flags(registers[inst->rm], (inst->data),'&',1);
	}
	else if (inst->type == Imm1){
		printf("\nImm1");
	}
	else if (inst->type == Imm2){
		printf("\nImm2");
	}
	else if (inst->type == Add1){
		printf("\nAdd1");
	}
	else if (inst->type == Add2){
		Add_Imm(inst);
		Change_flags(data_area[inst->add] + (data_area[inst->add+1]<<8), (inst->data),'&',1);
	}
	OF = '-';
	CF = '-';
}

void push(instruct* inst){
	if (inst->type == Norm){
		registers[4] -= 2;
		data_area[registers[4]+1] = (registers[inst->rm] & 0xff00) >> 8;
		data_area[registers[4]] = registers[inst->rm] & 0x00ff;
	}
	else if (inst->type == Imm1){
		inst->add = inst->new_data;
		Add_Imm(inst);
		registers[4] -= 2;
		data_area[registers[4]+1] = data_area[inst->add+1];
		data_area[registers[4]] = data_area[inst->add];
	}
	else if (inst->type == Imm2){
		printf("\nImm2");
	}
	else if (inst->type == Add1){
		printf("\nAdd1");
	}
	if(inst->type == Add2){
		Reg_Imm(inst);
		registers[4] -= 2;
		data_area[registers[4]+1] = data_area[inst->add + 1];
		data_area[registers[4]] = data_area[inst->add];
	}
}

void pop(instruct* inst){
	if (inst->type == Norm){
		printf("\nNorm");
	}
	else if (inst->type == Imm1){
		printf("\nImm1");
	}
	else if (inst->type == Imm2){
		printf("\nImm2");
	}
	else if (inst->type == Add1){
		printf("\nAdd1");
	}
	else if (inst->type == Add2){
		printf("\nAdd2");
	}
}

void cmp(instruct* inst){
	if (inst->type == Norm){
		if(inst->data == 0xff)
			inst->data = 0xffff;
		Change_flags(registers[inst->rm], (bswap_16(inst->data)),'-',1);
	}
	else if (inst->type == Imm1){
		Add_Imm(inst);
		Change_flags(data_area[inst->add] + (data_area[inst->add+1] << 8), inst->data,'-',1);
	}
	else if (inst->type == Imm2){
		Add_Imm(inst);
		Change_flags(data_area[inst->add] + (data_area[inst->add+1] << 8), registers[inst->reg],'-',1);
	}
	else if (inst->type == Add1){
		Reg_Imm(inst);
		Change_flags(data_area[inst->add] + (data_area[inst->add+1] << 8), registers[inst->reg],'-',1);
	}
	else if (inst->type == Add2){
		Reg_Imm(inst);
		Change_flags((data_area[inst->add+1] << 8) + data_area[inst->add], bswap_16(inst->data),'-',1);
	}
	else if (inst->type == RegR){
		Add_Imm(inst);
		Change_flags(registers[inst->rm],registers[inst->new_data],'-',1);
	}
}

void inc(instruct* inst){
	if (inst->type == Norm){
		Change_flags(registers[inst->reg],1,'+',0);
		registers[inst->reg] += 1;
	}
	else if (inst->type == Imm1){
		printf("\nImm1");
	}
	else if (inst->type == Imm2){
		printf("\nImm2");
	}
	else if (inst->type == Add1){
		printf("\nAdd1");
	}
	else if (inst->type == Add2){
		printf("\nAdd2");
	}

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
				inst->name = "+inc";
			else
				inst->name = "+ dec";
			Reg(byte,index,inst);
			if (strcmp(inst->name,"+ push") == 0){
				push(inst);
			}
			else if (strcmp(inst->name,"+ pop") == 0){
				registers[inst->rm] = data_area[registers[4]];
				registers[inst->rm] += data_area[registers[4]+1] << 8;
				registers[4] += 2;
			}
			else if (strcmp(inst->name,"+ dec") == 0){
				registers[inst->reg] -= 1;
				Change_flags(registers[inst->reg] + 1,1,'-',0);
			}
			else if (strcmp(inst->name,"+inc") == 0){
				inc(inst);
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
				inst->name = "+sub";
			else if (byte>>2 == SSB1)
				inst->name = "sbb";
			else if (byte>>2 == OR1)
				inst->name = "+ or";
			else if (byte>>2 == XOR1)
				inst->name = "+ xor";
			else if (byte>>2 == AND1)
				inst->name = "+and";
			else if (byte>>2 == CMP1)
				inst->name = "+cmp";
			D_w_mod_reg_rm(byte, index, inst);
			if (strcmp(inst->name,"++ mov") == 0){
				move(inst);
			}
			else if (strcmp(inst->name,"+ or") == 0){
				or(inst);
			}
			else if (strcmp(inst->name,"+ add") == 0){
				add(inst);
			}
			else if (strcmp(inst->name,"+ xor") == 0){
				xor(inst);
			}
			else if (strcmp(inst->name,"+sub")== 0){
				sub(inst);
			}
			else if(strcmp(inst->name,"+and") == 0){
				and(inst);
			}
			else if (strcmp(inst->name,"+cmp") == 0){
				cmp(inst);
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
				inst->name = "++ add";
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
				sub(inst);
			}
			else if (strcmp(inst->name,"+ cmp") == 0){
				inst->data = inst->new_data;
				cmp(inst);
			}
			else if (strcmp(inst->name,"++ add") == 0){
				inst->data = inst->rm;
				if(inst->type == Norm){
					Change_flags(registers[inst->data],inst->new_data,'+',1);
					Set_Registers(inst,registers[inst->data] + inst->new_data);
				}
				else if (inst->type == Add2){
					Change_flags(registers[inst->data],(data_area[inst->add+1] << 8) + data_area[inst->add],'+',1);
					Set_Registers(inst,registers[inst->data] + (data_area[inst->add+1] << 8) + data_area[inst->add]);
				}
				else if (inst->type == Add1){
					Reg_Imm(inst);
					Change_flags(data_area[inst->add] + (data_area[inst->add+1] << 8),registers[inst->new_data],'+',1);
					data_area[inst->add] += registers[inst->new_data] & 0x00ff;
					data_area[inst->add+1] += registers[inst->new_data] >> 8;
				}
			}
			return "cmp";
		case SHL:
			if (inst->reg == 0b100)
				inst->name = "++shl";
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
			int space = pretty(index,byte,(inst->mod << 6) + (inst-> reg << 3) + inst->reg,memPrint((inst->mod << 6) + (inst-> reg << 3) + inst->reg,1,0,0,0,1),disp,s,0,1);
			for(int i = 0; i<62-space; i++)
				printf(" ");
			printf("%s %s, %s", inst->name, r, str);
			if(strcmp(inst->name,"++shl") == 0){
				int bit = (registers[inst->rm] & 0x8000) >> 15;
				Change_flags(registers[inst->rm],1,'<',0);
				CF = flags[bit];
				registers[inst->rm] = (registers[inst->rm] << 1) & 0xffff;
			}
			return "shl";
	}			
	return "";
}

char* instruct1(int index, uint8_t byte, struct instruct* inst) {
	int size = 0;
	char* str = "";
	char* st;
	int space;
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
			space = pretty(index,byte,0,memPrint((inst->mod << 6) + (inst->reg << 3) + inst->rm,1,0,0,0,1),0,0,0,1);
			for (int i = 0; 62-space; i++)
				printf(" ");
			printf(" %s %s\n", "rep", ins);
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
				inst->name = "+++ mov byte";
				current++;
				inst->data = text_area[current];
				size = 1;
				if (inst->w == 1) {
					inst->name = "+++ mov";
					current++;
					inst->data = (inst->data << 8) + text_area[current];
					size = 2;
				}
				W_mod_rm(byte,index,inst,size);
				Add_Imm(inst);
				data_area[inst->add] = inst->data & 0x00ff;
				if (inst->w)
					data_area[inst->add+1] = inst->data >> 8;
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
			space = pretty(index,byte,(inst->mod << 6) + (inst->reg << 3) + inst->rm,memPrint((inst->mod << 6) + (inst->reg << 3) + inst->rm,1,1,1,0,1),0,0,0,1);
			for (int i = 0; 62-space; i++)
				printf(" ");
			printf(" %s %s, %s\n", str, st, memPrint((inst->mod << 6) + (inst->reg << 3) + inst->rm,1,1,1,0,1));
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
			space = pretty(index,byte,0,"",0,0,0,1);
			for (int i = 0; 62-space; i++)
				printf(" ");
			printf(" %s %s, dx\n", str, st);
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
				inst->name = "+cmp ax,";
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
			if(strcmp(inst->name,"+cmp ax,") == 0){
				inst->rm = 0;
				cmp(inst);
			}
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
				if (strcmp(inst->name,"") != 0){
					W_mod_rm(byte,index,inst,0);
					if (strcmp(inst->name,"+ neg") == 0){
						CF = flags[registers[inst->rm]!=0];
						Change_flags(0,registers[inst->rm],'-',0);
						registers[inst->rm] = ~registers[inst->rm] + 1;
					}
					return "not";
				}
			}
			current++;
			inst->data = text_area[current];
			size = 1;
			if (inst->reg == 0b110)
				inst->name = "xor";
			else if (byte >> 1 == MUL && inst->reg == 0b000){
				if (inst->w == 0 && inst->mod != 0b11)
					inst->name = "+ test byte";
				else
					inst->name = "+ test";
			}
			else if (inst->reg == 0b001)
				inst->name = "+or";
			else if (inst->reg == 0b100)
				inst->name = "+and";
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
					inst->name = "+++add";
				else if (inst->reg == 0b010)
					inst->name = "adc";
				else if (inst->reg == 0b101)
					inst->name = "++ sub";
				else if (inst->reg == 0b011)
					inst->name = "sbb";
				else if (inst->reg == 0b111){
					if (inst->w == 1)
						inst->name = "++ cmp";
					else
						inst->name = "cmp byte";
				}
				S_w_mod_rm(byte,index,inst,size);
				if(strcmp(inst->name,"++ cmp") == 0){
					cmp(inst);
				}
				else if (strcmp(inst->name,"++ sub") == 0){
					inst->data = bswap_16(inst->data);
					sub(inst);
				}
			}
			if(strcmp(inst->name,"+ test") == 0){
				test(inst);
			}
			else if (strcmp(inst->name,"+ test byte") == 0){
				test(inst);
			}
			else if (strcmp(inst->name, "+and") == 0){
				and(inst);
			}
			else if (strcmp(inst->name, "+or") == 0){
				or(inst);
			}
			else if (strcmp(inst->name, "+++add") == 0){
				inst->data = bswap_16(inst->data);
				inst->type = 16;
				add(inst);
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
	int space;
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
				inst->name = "+inc";
			else if (inst->reg == 0b110)
				inst->name = "+push";
			else if ((inst->reg == 0b100) || (inst->reg == 0b101))
				inst->name = "jmp";
			else if ((inst->reg == 0b010) || (inst->reg == 0b011))
				inst->name = "++call";
			Mod_rm(byte,index,inst,0);
			if(strcmp(inst->name,"+push") == 0){
				push(inst);
			}
			else if (strcmp(inst->name,"+inc") == 0){
				Add_Imm(inst);
				Change_flags(data_area[inst->add],1,'+',0);
				data_area[inst->add] += 1;
			}
			else if (strcmp(inst->name,"++call") == 0){
				registers[4] -= 2;
				current = current+call_size-1;
				data_area[registers[4]+1] = (current & 0xff00) >> 8;
				data_area[registers[4]] = current & 0x00ff;
				current = registers[inst->rm] - 1;
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
			nothing("+ cbw",byte,index,-1,-1,0,0);
			if ((registers[0] & 0x0080) == 0x0080)
				registers[0] = 0xff00 + (registers[0] & 0x00ff);
			else
				registers[0] = registers[0] & 0x00ff;
			return "cbw";
		case CWD:
			current--;
			nothing("cwd",byte,index,-1,-1,0,0);
			return "cwd";
		case INT1:
			nothing("+ int",byte,index,(inst->mod << 6) + (inst->reg << 3) + inst->rm,(inst->mod << 6) + (inst->reg << 3) + inst->rm,1,1);
			/*for(long unsigned int i = 0; i<sizeof(message); i+=2){
				printf("\ndata[%04lx] = %02x%02x",registers[3]+i,data_area[registers[3]+i+1],data_area[registers[3]+i]);
			}
			if(tm == 3)
				exit(1);
			else
				tm++;*/
			message* m = (message*)(data_area + registers[3]);
			syscalls_l[m->m_type-1](m,data_area);
			if (m->m_type == 4)
				registers[0] = 0;
			return "int";
		case JNB:
			inst->name = "+ jnb";
			inst->new_data = byte;
			jump(inst,index);
			if(CF == '-')
				current = inst->data - 1;
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
			inst->name = "+je";
			inst->new_data = byte;
			jump(inst,index);
			if (ZF == 'Z')
				current = inst->data-1;
			return "jnb";
		case JL:
			inst->name = "+jl";
			inst->new_data = byte;
			jump(inst,index);
			if ((SF == 'S' && OF == '-') || (SF == '-' && OF == 'O'))
				current = inst->data-1;
			return "jnb";
		case JLE:
			inst->name = "+jle";
			inst->new_data = byte;
			jump(inst,index);
			if((SF == 'S' && OF == '-') || (SF == '-' && OF == 'O') || ZF == 'Z')
				current = inst->data-1;
			return "jnb";
		case JB:
			inst->name = "+jb";
			inst->new_data = byte;
			jump(inst,index);
			if(CF != '-')
				current = inst->data - 1;
			return "jnb";
		case JBE:
			inst->name = "+jbe";
			inst->new_data = byte;
			jump(inst,index);
			if(CF != '-' || ZF == 'Z')
				current = inst->data - 1;
			return "jnb";
		case JP:
			inst->name = "jp";
			inst->new_data = byte;
			jump(inst,index);
			return "jnb";
		case JO:
			inst->name = "+jo";
			inst->new_data = byte;
			jump(inst,index);
			if(OF != '-')
				current = inst->data - 1;
			return "jnb";
		case JS:
			inst->name = "+js";
			inst->new_data = byte;
			jump(inst,index);
			if(SF != '-')
				current = inst->data - 1;
			return "jnb";
		case JNE:
			inst->name = "+jne";
			inst->new_data = byte;
			jump(inst,index);
			if (ZF == '-')
				current = inst->data - 1;
			return "jnb";
		case JNL:
			inst->name = "+jnl";
			inst->new_data = byte;
			jump(inst,index);
			if((SF == '-' && OF == '-') || (SF == 'S' && OF == 'O'))
				current = inst->data - 1;
			return "jnb";
		case JNLE:
			inst->name = "+jnle";
			inst->new_data = byte;
			jump(inst,index);
			if((SF == '-' && OF == '-') || (SF == 'S' && OF == 'O') || ZF == '-')
				current = inst->data - 1;
			return "jnb";
		case JNBE:
			inst->name = "+jnbe";
			inst->new_data = byte;
			jump(inst,index);
			if(CF == '-' && ZF == '-')
				current = inst->data - 1;
			return "jnb";
		case JNP:
			inst->name = "jnp";
			inst->new_data = byte;
			jump(inst,index);
			return "jnb";
		case JNO:
			inst->name = "+jno";
			inst->new_data = byte;
			jump(inst,index);
			if(OF == '-')
				current = inst->data - 1;
			return "jnb";
		case JNS:
			inst->name = "+jns";
			inst->new_data = byte;
			jump(inst,index);
			if(SF == '-')
				current = inst->data - 1;
			return "jnb";
		case LOOP:
			inst->name = "loop";
			inst->new_data = byte;
			jump(inst,index);
			return "jnb";
		case LOOPZ:
			inst->name = "loopz";
			inst->new_data = byte;
			jump(inst,index);
			return "jnb";
		case LOOPNZ:
			inst->name = "loopnz";
			inst->new_data = byte;
			jump(inst,index);
			return "jnb";
		case JCXZ:
			inst->name = "jxcz";
			inst->new_data = byte;
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
			inst->new_data = byte;
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

			char* t;
			asprintf(&t,"%s%s",memPrint(data,2,0,0,0,1),memPrint(data2,2,0,0,0,1));
			space = pretty(index,byte,0,t,0,0,0,1);
			for (int i = 0; 62-space; i++)
				printf(" ");
			printf(" %s %s:%s\n", inst->name, memPrint(data,2,1,1,0,1),memPrint(data2,2,1,1,0,1));
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
			else{
				ind += data;
			}
			inst->disp = ind & 0x0000ffff;
			nothing("+ call",byte,index,data,ind,0,2);
			//JUST KEEP IN MIND THE SIZE OF CALL, RN IT'S ALWAYS 3 BUT AT SOME POINT ITS 2 WHICH BUGS THE CODE
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
	data_area = malloc(0xffff*sizeof(uint8_t));

	for (uint32_t i = 0 ; i< length; i++)
		fread(&text_area[i], sizeof(text_area[i]), 1, file);
	for (uint32_t i = 0 ; i< a_data; i++)
		fread(&(data_area[i]), sizeof(data_area[i]), 1, file);
	for (uint32_t i = a_data; i<0xffff; i++)
		data_area[i] = 0;

	fclose(file);

	printf(" AX   BX   CX   DX   SP   BP   SI   DI  FLAGS IP\n");

	data_area[0xffdd] = 0x00;
	data_area[0xffdc] = 0x01;
	data_area[0xffdf] = 0xff;
	data_area[0xffde] = 0xe6;
	/*data_area[0x0218] = 0x00;
	data_area[0x0219] = 0x00;*/

	uint8_t byte, b;
    for (; current < length; current++)
    {
		byte = text_area[current];
		current++;
		b = text_area[current];

		struct instruct inst = {"",(byte & 0b00000010) >> 1,byte & 0b00000001,(b & 0b11000000) >> 6,(b & 0b00111000) >> 3,b & 0b00000111,b,0,0,"","",0,0};
		
		if (instruct0(current-1,byte,&inst)[0] != '\0')
			printf("\n");
		else if (instruct1(current-1,byte,&inst)[0] != '\0')
			printf("\n");
		else if (instruct2(current-1,byte,&inst)[0] != '\0')
			printf("\n");
		else if (instruct3(current-1,byte,&inst)[0] != '\0')
			printf("\n");
		else if (instruct4(current-1,byte,&inst)[0] != '\0')
			printf("\n");
		else if (instruct5(current-1,byte,&inst)[0] != '\0')
			printf("\n");
		//printf("data1 = %02x%02x\n",data_area[0x0274],data_area[0x0275]);
    }
	printf("\n");
    return 0;
}
