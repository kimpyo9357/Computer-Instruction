#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#pragma comment(lib,"ws2_32")

int pc;
int R[32];
int Memory[0x4000000];
int instruction;
int cycle;
int count[5] = { 0 }; // R type / I type / J type / Memory Access / branche

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
	int zimm;
};
struct instruction_ in;

void fetch();
struct instruction_ decode(int instruction);
int execute(struct instruction_ inst);
int memory(struct instruction_, int value);
void writeback(struct instruction_, int value);
void update_pc();
void print();

int main() {
	FILE* fp = NULL;
	int inst_;
	int ret;
	int value;
	int i = 0;


	fp = fopen("input4.bin", "rb");
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
	printf("\n\n");
	while (1) {
		if (pc == -1) { break; }
		fetch();
		printf("Mem[%d] 0x%08X\n", pc/4, Memory[pc / 4]);
		update_pc();
		in = decode(instruction);
		value = execute(in);
		value = memory(in, value);
		writeback(in, value);
		printf("Register state : \n");
		for (i = 0; i < 32; i++) { printf("%x ", R[i]); }
		printf("\n");
		printf("pc : %d", pc);
		if (in.opcode == 0x23) printf(", Memory address : %x, value :%d", R[in.rs] + in.simm, R[in.rt]);
		printf("\n\n");
		cycle++;
	}

	printf("result : %d\n", R[2]);
	print();
	fclose(fp);
	return 0;
}

int instruction;
void fetch() {
	instruction = Memory[pc / 4];
}

struct instruction_ decode(int inst_binary) {
	struct instruction_ ret;
	char buffer[33] = { 0 };
	char imm[33] = { 0 };
	char target[4][33] = { 0 };
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
	ret.zimm = inst_binary & 0xFFFF;

	for (i = 0; i < 16; i++) {
		target[0][i] = ((inst_binary >> 15) & 0x1) + 48;
	}
	sprintf(imm, "%016s", itoa((inst_binary & 0xFFFF), buffer, 2));
	strcat(target[0], imm);


	ret.simm = strtoul(target[0], NULL, 2);

	memset(&buffer, 0, sizeof(buffer));
	memset(&imm, 0, sizeof(imm));

	for (i = 0; i < 14; i++) {
		target[2][i] = ((inst_binary >> 15) & 0x1) + 48;
	}
	sprintf(imm, "%016s", itoa((inst_binary & 0xFFFF), buffer, 2));
	strcat(target[2], imm);
	strcat(target[2], "00");
	ret.btarget = strtoul(target[2], NULL, 2);

	memset(&buffer, 0, sizeof(buffer));
	memset(&imm, 0, sizeof(imm));

	sprintf(imm, "%04s", itoa((pc + 4) >> 28 & 0xF, buffer, 2));
	strcat(target[3], imm);
	sprintf(imm, "%026s", itoa((inst_binary & 0x3FFFFFF), buffer, 2));
	strcat(target[3], imm);
	strcat(target[3], "00");
	ret.jtarget = strtoul(target[3], NULL, 2);
	
	if (ret.opcode == 0x0 && ret.funct == 0x08) { pc = R[ret.rs]; }
	if (ret.opcode == 0x2) { pc = ret.jtarget; }

	//printf("dec: opcode: %x shamt : %d funct: %x \n", ret.opcode, ret.shamt, ret.funct);
	//printf("dec: rs: %d rt %d rd : %d \n", ret.rs, ret.rt, ret.rd);
	//printf("dec: simm: %x btarget : %x jtarget: %x\n", ret.simm, ret.btarget, ret.jtarget);

	return ret;
};

int execute(struct instruction_ in) {
	int value = 0;
	switch (in.opcode) {
	case 0x0: {
		switch (in.funct) {
		case 0x20:
		case 0x21: {
			value = R[in.rs] + R[in.rt];
			break;
		}
		case 0x24: {
			value = R[in.rs] & R[in.rt];
			break;
		}
		case 0x8: {
			//pc = R[in.rs];
			count[3]++;
			break;
		}
		case 0x27: {
			value = !(R[in.rs] | R[in.rt]);
			break;
		}
		case 0x25: {
			value = R[in.rs] | R[in.rt];
			break;
		}
		case 0x2a:
		case 0x2b: {
			value = (R[in.rs] < R[in.rt]) ? 1 : 0;
			break;
		}
		case 0x00: {
			value = R[in.rt] << in.shamt;
			break;
		}
		case 0x02: {
			value = R[in.rt] >> in.shamt;
			break;
		}
		case 0x22:
		case 0x23: {
			value = R[in.rs] - R[in.rt];
			break;
		}
		default: printf("Not In Instance");
		}
		count[0]++;
		//printf("R type\n");
		break;
	}
	case 0x2:
	case 0x3: {
		switch (in.opcode) {
			/*case 0x2: {
				pc = in.jtarget;
				break;
			}*/
			case 0x3: {
				value = pc + 4;
				//pc = in.jtarget;
				break;
			}
			default: printf("Not In Instance");
			}
		//printf("J type\n");
		count[2]++;
		count[3]++;
		break;
	}
	default: {
		switch (in.opcode) {
			case 0x8:
			case 0x9: {
				value = R[in.rs] + in.simm;
				break;
			}
			case 0xc: {
				value = R[in.rs] & in.zimm;
				break;
			}
			case 0x4: {
				if (R[in.rs] == R[in.rt]) { pc = pc + in.btarget; }
				count[4]++;
				break;
			}
			case 0x5: {
				if (R[in.rs] != R[in.rt]) { pc = pc + in.btarget; }
				count[4]++;
				break;
			}
			case 0xf: {
				value = in.zimm << 16;
				break;
			}
			case 0x23: {
				value = R[in.rs] + in.simm;
				break;
			}
			case 0xd: {
				value = R[in.rs] | in.zimm;
				break;
			}
			case 0xa:
			case 0xb: {
				value = (R[in.rs] < in.simm) ? 1 : 0;
				break;
			}
			case 0x2b: {
				value = R[in.rt];
				break;
			}
			default:
				printf("Not In Instance");
			}
			count[1]++;
			//printf("I type\n");
			break;
		}
	}
	return value;
	printf("\n");
}

int memory(struct instruction_ in, int value) {
	if (in.opcode == 0x23) { 
		printf("%x", value);
		return Memory[value]; }
	if (in.opcode == 0x2b) {
		Memory[R[in.rs] + in.simm] = value;
	}
	return value;
}

void writeback(struct instruction_ in, int input) {
	//int value = 0;
	switch (in.opcode) {
		case 0x0: {
			switch (in.funct) {
				case 0x20:
				case 0x21:
				case 0x24:
				case 0x27:
				case 0x25:
				case 0x2a:
				case 0x2b:
				case 0x00:
				case 0x02:
				case 0x22:
				case 0x23: {
					R[in.rd] = input;
					break;
				}
			}
			break;
		}
		case 0x3: {
			R[31] = input;
			pc = in.jtarget;
			break;
		}
		default: {
			switch (in.opcode) {
				case 0x8:
				case 0x9:
				case 0xc:
				case 0xf:
				case 0x23:
				case 0xd:
				case 0xa:
				case 0xb: {
					R[in.rt] = input;
					break;
				}
			break;
			}
		}
	}
}

void update_pc() {
	pc = pc + 4;
}

void print() {
	int i = 0;
	printf("실행된 명령어 수 : %d\n", cycle);
	printf("R type 명령어 수 : %d\n", count[0]);
	printf("I type 명령어 수 : %d\n", count[1]);
	printf("J type 명령어 수 : %d\n", count[2]);
	printf("Memory Access 명령어 수 : %d\n", count[3]);
	printf("Branches 명령어 수 : %d\n\n", count[4]);

}
