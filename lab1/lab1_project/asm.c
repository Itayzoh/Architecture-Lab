/*
 * SP ASM: Simple Processor assembler
 *
 * usage: asm
 */
#include <stdio.h>
#include <stdlib.h>

#define ADD 0
#define SUB 1
#define LSF 2
#define RSF 3
#define AND 4
#define OR  5
#define XOR 6
#define LHI 7
#define LD 8
#define ST 9
#define JLT 16
#define JLE 17
#define JEQ 18
#define JNE 19
#define JIN 20
#define HLT 24

#define MEM_SIZE_BITS	(16)
#define MEM_SIZE	(1 << MEM_SIZE_BITS)
#define MEM_MASK	(MEM_SIZE - 1)
unsigned int mem[MEM_SIZE];

int pc = 0;

static void asm_cmd(int opcode, int dst, int src0, int src1, int immediate)
{
	int inst;

	inst = ((opcode & 0x1f) << 25) | ((dst & 7) << 22) | ((src0 & 7) << 19) | ((src1 & 7) << 16) | (immediate & 0xffff);
	mem[pc++] = inst;
}

static void assemble_program(char *program_name)
{
	FILE *fp;
	int addr, i, last_addr;

	for (addr = 0; addr < MEM_SIZE; addr++)
		mem[addr] = 0;

	pc = 0;
	/* pc=0: Load N */
	asm_cmd(LD, 2, 0, 1, 1000);   //  0: R2 = MEM[1000]  (N = input)

	/* pc=1,2: Init bounds */
	asm_cmd(ADD, 3, 0, 0, 0);      //  1: R3 = lo = 0
	asm_cmd(ADD, 4, 1, 0, 0xFFFF); //  2: R4 = hi = 65535
	//     Valid upper bound for any 32-bit N

/* ── BINARY SEARCH LOOP (pc=3) ── */
	asm_cmd(JLE, 0, 4, 3, 97);     //  3: if hi <= lo → FINISH (pc=97)

	asm_cmd(ADD, 5, 3, 4, 0);      //  4: R5 = lo + hi
	asm_cmd(RSF, 5, 5, 1, 1);      //  5: R5 >>= 1  →  mid = (lo+hi)/2

	/* Save lo, hi -- frees R3,R4 for multiply */
	asm_cmd(ST, 0, 3, 1, 1002);   //  6: MEM[1002] = lo
	asm_cmd(ST, 0, 4, 1, 1003);   //  7: MEM[1003] = hi

	/* ── MULTIPLY: R6 = mid*mid (shift-and-add, 16 bits unrolled) ── */
	asm_cmd(ADD, 6, 0, 0, 0);      //  8: R6 = 0        (accumulator)
	asm_cmd(ADD, 3, 1, 0, 1);      //  9: R3 = 1        (bitmask)
	asm_cmd(ADD, 4, 5, 0, 0);      // 10: R4 = mid      (mid << 0)

	/* bit 0 */
	asm_cmd(AND, 2, 5, 3, 0);      // 11: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 14);     // 12: if bit==0 skip → pc=14
	asm_cmd(ADD, 6, 6, 4, 0);      // 13: R6 += mid<<0
	asm_cmd(LSF, 3, 3, 1, 1);      // 14: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 15: mid_shifted <<= 1

	/* bit 1 */
	asm_cmd(AND, 2, 5, 3, 0);      // 16: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 19);     // 17: if bit==0 skip → pc=19
	asm_cmd(ADD, 6, 6, 4, 0);      // 18: R6 += mid<<1
	asm_cmd(LSF, 3, 3, 1, 1);      // 19: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 20: mid_shifted <<= 1

	/* bit 2 */
	asm_cmd(AND, 2, 5, 3, 0);      // 21: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 24);     // 22: if bit==0 skip → pc=24
	asm_cmd(ADD, 6, 6, 4, 0);      // 23: R6 += mid<<2
	asm_cmd(LSF, 3, 3, 1, 1);      // 24: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 25: mid_shifted <<= 1

	/* bit 3 */
	asm_cmd(AND, 2, 5, 3, 0);      // 26: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 29);     // 27: if bit==0 skip → pc=29
	asm_cmd(ADD, 6, 6, 4, 0);      // 28: R6 += mid<<3
	asm_cmd(LSF, 3, 3, 1, 1);      // 29: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 30: mid_shifted <<= 1

	/* bit 4 */
	asm_cmd(AND, 2, 5, 3, 0);      // 31: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 34);     // 32: if bit==0 skip → pc=34
	asm_cmd(ADD, 6, 6, 4, 0);      // 33: R6 += mid<<4
	asm_cmd(LSF, 3, 3, 1, 1);      // 34: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 35: mid_shifted <<= 1

	/* bit 5 */
	asm_cmd(AND, 2, 5, 3, 0);      // 36: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 39);     // 37: if bit==0 skip → pc=39
	asm_cmd(ADD, 6, 6, 4, 0);      // 38: R6 += mid<<5
	asm_cmd(LSF, 3, 3, 1, 1);      // 39: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 40: mid_shifted <<= 1

	/* bit 6 */
	asm_cmd(AND, 2, 5, 3, 0);      // 41: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 44);     // 42: if bit==0 skip → pc=44
	asm_cmd(ADD, 6, 6, 4, 0);      // 43: R6 += mid<<6
	asm_cmd(LSF, 3, 3, 1, 1);      // 44: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 45: mid_shifted <<= 1

	/* bit 7 */
	asm_cmd(AND, 2, 5, 3, 0);      // 46: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 49);     // 47: if bit==0 skip → pc=49
	asm_cmd(ADD, 6, 6, 4, 0);      // 48: R6 += mid<<7
	asm_cmd(LSF, 3, 3, 1, 1);      // 49: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 50: mid_shifted <<= 1

	/* bit 8 */
	asm_cmd(AND, 2, 5, 3, 0);      // 51: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 54);     // 52: if bit==0 skip → pc=54
	asm_cmd(ADD, 6, 6, 4, 0);      // 53: R6 += mid<<8
	asm_cmd(LSF, 3, 3, 1, 1);      // 54: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 55: mid_shifted <<= 1

	/* bit 9 */
	asm_cmd(AND, 2, 5, 3, 0);      // 56: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 59);     // 57: if bit==0 skip → pc=59
	asm_cmd(ADD, 6, 6, 4, 0);      // 58: R6 += mid<<9
	asm_cmd(LSF, 3, 3, 1, 1);      // 59: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 60: mid_shifted <<= 1

	/* bit 10 */
	asm_cmd(AND, 2, 5, 3, 0);      // 61: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 64);     // 62: if bit==0 skip → pc=64
	asm_cmd(ADD, 6, 6, 4, 0);      // 63: R6 += mid<<10
	asm_cmd(LSF, 3, 3, 1, 1);      // 64: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 65: mid_shifted <<= 1

	/* bit 11 */
	asm_cmd(AND, 2, 5, 3, 0);      // 66: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 69);     // 67: if bit==0 skip → pc=69
	asm_cmd(ADD, 6, 6, 4, 0);      // 68: R6 += mid<<11
	asm_cmd(LSF, 3, 3, 1, 1);      // 69: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 70: mid_shifted <<= 1

	/* bit 12 */
	asm_cmd(AND, 2, 5, 3, 0);      // 71: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 74);     // 72: if bit==0 skip → pc=74
	asm_cmd(ADD, 6, 6, 4, 0);      // 73: R6 += mid<<12
	asm_cmd(LSF, 3, 3, 1, 1);      // 74: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 75: mid_shifted <<= 1

	/* bit 13 */
	asm_cmd(AND, 2, 5, 3, 0);      // 76: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 79);     // 77: if bit==0 skip → pc=79
	asm_cmd(ADD, 6, 6, 4, 0);      // 78: R6 += mid<<13
	asm_cmd(LSF, 3, 3, 1, 1);      // 79: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 80: mid_shifted <<= 1

	/* bit 14 */
	asm_cmd(AND, 2, 5, 3, 0);      // 81: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 84);     // 82: if bit==0 skip → pc=84
	asm_cmd(ADD, 6, 6, 4, 0);      // 83: R6 += mid<<14
	asm_cmd(LSF, 3, 3, 1, 1);      // 84: mask <<= 1
	asm_cmd(LSF, 4, 4, 1, 1);      // 85: mid_shifted <<= 1

	/* bit 15 -- no shifts after last bit */
	asm_cmd(AND, 2, 5, 3, 0);      // 86: R2 = mid & mask
	asm_cmd(JEQ, 0, 2, 0, 89);     // 87: if bit==0 skip → pc=89
	asm_cmd(ADD, 6, 6, 4, 0);      // 88: R6 += mid<<15

	/* ── Restore registers, update bounds ── */
	asm_cmd(LD, 2, 0, 1, 1000);   // 89: R2 = N           (reload)
	asm_cmd(LD, 3, 0, 1, 1002);   // 90: R3 = lo          (restore)
	asm_cmd(LD, 4, 0, 1, 1003);   // 91: R4 = hi          (restore)

	asm_cmd(JLE, 0, 6, 2, 95);     // 92: if mid²(R6) <= N(R2) → pc=95 (lo update)
	asm_cmd(ADD, 4, 5, 0, 0);      // 93: hi = mid
	asm_cmd(JEQ, 0, 0, 0, 3);      // 94: jump to LOOP (0==0, always taken)

	asm_cmd(ADD, 3, 5, 1, 1);      // 95: lo = mid + 1
	asm_cmd(JEQ, 0, 0, 0, 3);      // 96: jump to LOOP (0==0, always taken)

	/* ── FINISH (pc=97) ── */
	asm_cmd(SUB, 3, 3, 1, 1);      // 97: R3 = lo - 1  =  floor(sqrt(N))
	asm_cmd(ST, 0, 3, 1, 1001);   // 98: MEM[1001] = result
	asm_cmd(HLT, 0, 0, 0, 0);      // 99: HALT

	/* ── Input data ── */
	mem[1000] = 4937775;            // N = 4937775 (Smith number)
	// Expected: floor(sqrt(4937775)) = 2222
	// Check: 2222² = 4937284 <= 4937775 ✓
	//        2223² = 4941729 >  4937775 ✓

	last_addr = 1002;               // write code + data up to MEM[1001]


	fp = fopen(program_name, "w");
	if (fp == NULL) {
		printf("couldn't open file %s\n", program_name);
		exit(1);
	}
	addr = 0;
	while (addr < last_addr) {
		fprintf(fp, "%08x\n", mem[addr]);
		addr++;
	}
}


int main(int argc, char *argv[])
{
	
	if (argc != 2){
		printf("usage: asm program_name\n");
		return -1;
	}else{
		assemble_program(argv[1]);
		printf("SP assembler generated machine code and saved it as %s\n", argv[1]);
		return 0;
	}
	
}
