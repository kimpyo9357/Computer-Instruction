#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#pragma comment(lib,"ws2_32")

int pc;
int R[32];
int Memory[0x400000];
int cycle;
int instruction;

struct instruction_ {
	int instruction;
	int opcode;
	int rs;
	int rt;
	int rd;
	int simm;
	int btarget;
	int jtarget;
	int shamt;
	int funct;
};
struct instruction_ in;

void fetch();
struct instruction_ decode(int instruction);
void execute(struct instruction_ inst);
void memory(struct instruction_);
void writeback(struct instruction_);
void update_pc();
void print();

int main() {
	FILE* fp = NULL;
	int inst_;
	int value;
	int ret;
	int i = 0;

	fp = fopen("simple2.bin", "rb");
	while (1) {
		ret = fread(&value, sizeof(value), 1, fp);
		if (ret != 1) { break; }
		inst_ = ntohl(value);
		printf("Mem[%d] 0x%08X\n", i, inst_);
		Memory[i] = inst_;
		i++;
	}
	pc = 0;
	R[31] = -1;
	R[29] = 0x1000000;

	while (1) {
		if (pc == 48) { break; }

		fetch();
		in = decode(instruction);
		execute(in);
		// memory(in);
		// writeback(in);
		update_pc();

		//print();
		cycle++;
	}
	fclose(fp);
	return 0;
}

int instruction;
void fetch() {
	instruction = Memory[pc / 4];
}

void execute(struct instruction_ in) {
	switch (in.opcode){
		case 0: {
			printf("R type\n");
			break;
		};
		case 2 || 3: {
			printf("J type\n");
			break;
		};
		default: {
			printf("I type\n");
		};
	}
	printf("\n");
}

struct instruction_ decode(int inst_binary) {
	struct instruction_ ret;
	char buffer[33] = {0};
	char imm[33] = {0};
	char target[3][33] = {0};
	int i = 0;
	memset(&ret, 0, sizeof(ret));
	ret.instruction = 0;
	ret.opcode = 0;
	ret.rs = 0;
	ret.rt = 0;
	ret.rd = 0;
	ret.simm = 0;
	ret.btarget = 0;
	ret.jtarget = 0;
	ret.shamt = 0;
	ret.funct = 0;

	ret.opcode = (inst_binary >> 26) & 0x3F;
	ret.rs = (inst_binary >> 21) & 0x1F;
	ret.rt = (inst_binary >> 16) & 0x1F;
	ret.rd = (inst_binary >> 11) & 0x1F;
	ret.shamt = (inst_binary >> 6) & 0x1F;
	ret.funct = inst_binary & 0x3F;

	for (i = 0; i < 16; i++) {
		target[0][i] = ((inst_binary >> 15) & 0x1) + 48;
	}
	sprintf(imm, "%016s", itoa((inst_binary & 0xFFFF), buffer, 2));
	strcat(target[0], imm);
	ret.simm = strtoul(target[0], NULL, 2);

	memset(&buffer, 0, sizeof(buffer));
	memset(&imm, 0, sizeof(imm));

	for (i = 0; i < 14; i++) {
		target[1][i] = ((inst_binary >> 15) & 0x1) + 48;
	}
	sprintf(imm, "%016s", itoa((inst_binary & 0xFFFF), buffer, 2));
	strcat(target[1], imm);
	strcat(target[1], "00");
	ret.btarget = strtoul(target[1], NULL, 2);

	memset(&buffer, 0, sizeof(buffer));
	memset(&imm, 0, sizeof(imm));

	sprintf(imm, "%04s", itoa((pc + 4) >> 28 & 0xF, buffer, 2));
	strcat(target[2], imm);
	sprintf(imm, "%026s", itoa((inst_binary & 0x3FFFFFF), buffer, 2));
	strcat(target[2], imm);
	strcat(target[2], "00");
	ret.jtarget = strtoul(target[2], NULL, 2);
	
	printf("dec: opcode: %d shamt : %d funct: %x \n", ret.opcode, ret.shamt, ret.funct);
	printf("dec: rs: %d rt %d rd : %d \n", ret.rs, ret.rt, ret.rd);
	printf("dec: simm: %x btarget : %x jtarget: %x\n", ret.simm, ret.btarget, ret.jtarget);

	return ret; 
};

void update_pc() {
	pc = pc + 4;
}