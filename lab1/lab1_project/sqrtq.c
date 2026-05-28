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
#define LD  8
#define ST  9
#define JLT 16
#define JLE 17
#define JEQ 18
#define JNE 19
#define JIN 20
#define HLT 24

#define MEM_SIZE (1 << 16)
unsigned int mem[MEM_SIZE];
int pc = 0;

static void asm_cmd(int opcode, int dst, int src0, int src1, int immediate)
{
    unsigned int inst;
    inst = ((opcode & 0x1f) << 25) | ((dst & 7) << 22) | ((src0 & 7) << 19) | ((src1 & 7) << 16) | (immediate & 0xffff);
    mem[pc++] = inst;
}

static void assemble_program(char* program_name)
{
    FILE* fp;
    for (int i = 0; i < MEM_SIZE; i++) mem[i] = 0;
    pc = 0;

    // pc=0: Load N
    asm_cmd(LD, 2, 0, 1, 1000);

    // pc=1-3: Init bounds
    asm_cmd(ADD, 3, 0, 0, 0);     // R3 = lo = 0
    asm_cmd(LHI, 4, 0, 1, 1);     // R4 = 0x00010000
    asm_cmd(SUB, 4, 4, 1, 1);     // R4 = hi = 65535

    // pc=4: Loop guard
    asm_cmd(JLE, 0, 4, 3, 99);

    // pc=5-7: mid = lo + ((hi - lo) >> 1)
    asm_cmd(SUB, 5, 4, 3, 0);     // R5 = hi - lo
    asm_cmd(RSF, 5, 5, 1, 1);     // R5 = (hi - lo) >> 1
    asm_cmd(ADD, 5, 5, 3, 0);     // R5 = mid

    // pc=8-9: Save lo/hi before multiply clobbers R3/R4
    asm_cmd(ST, 0, 3, 1, 1002);
    asm_cmd(ST, 0, 4, 1, 1003);

    // pc=10-12: Multiply init (compute mid*mid in R6)
    asm_cmd(ADD, 6, 0, 0, 0);     // R6 = acc = 0
    asm_cmd(ADD, 3, 0, 1, 1);     // R3 = mask = 1
    asm_cmd(ADD, 4, 5, 0, 0);     // R4 = shifted_mid = mid

    /* bit 0  (pc 13-17) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 0 of mid
    asm_cmd(JEQ, 0, 2, 0, 16);     // if bit 0 == 0, skip accumulate (jump to pc=16)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 0)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 1)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 1)

    /* bit 1  (pc 18-22) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 1 of mid
    asm_cmd(JEQ, 0, 2, 0, 21);     // if bit 1 == 0, skip accumulate (jump to pc=21)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 1)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 2)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 2)

    /* bit 2  (pc 23-27) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 2 of mid
    asm_cmd(JEQ, 0, 2, 0, 26);     // if bit 2 == 0, skip accumulate (jump to pc=26)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 2)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 3)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 3)

    /* bit 3  (pc 28-32) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 3 of mid
    asm_cmd(JEQ, 0, 2, 0, 31);     // if bit 3 == 0, skip accumulate (jump to pc=31)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 3)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 4)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 4)

    /* bit 4  (pc 33-37) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 4 of mid
    asm_cmd(JEQ, 0, 2, 0, 36);     // if bit 4 == 0, skip accumulate (jump to pc=36)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 4)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 5)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 5)

    /* bit 5  (pc 38-42) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 5 of mid
    asm_cmd(JEQ, 0, 2, 0, 41);     // if bit 5 == 0, skip accumulate (jump to pc=41)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 5)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 6)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 6)

    /* bit 6  (pc 43-47) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 6 of mid
    asm_cmd(JEQ, 0, 2, 0, 46);     // if bit 6 == 0, skip accumulate (jump to pc=46)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 6)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 7)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 7)

    /* bit 7  (pc 48-52) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 7 of mid
    asm_cmd(JEQ, 0, 2, 0, 51);     // if bit 7 == 0, skip accumulate (jump to pc=51)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 7)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 8)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 8)

    /* bit 8  (pc 53-57) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 8 of mid
    asm_cmd(JEQ, 0, 2, 0, 56);     // if bit 8 == 0, skip accumulate (jump to pc=56)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 8)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 9)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 9)

    /* bit 9  (pc 58-62) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 9 of mid
    asm_cmd(JEQ, 0, 2, 0, 61);     // if bit 9 == 0, skip accumulate (jump to pc=61)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 9)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 10)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 10)

    /* bit 10 (pc 63-67) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 10 of mid
    asm_cmd(JEQ, 0, 2, 0, 66);     // if bit 10 == 0, skip accumulate (jump to pc=66)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 10)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 11)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 11)

    /* bit 11 (pc 68-72) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 11 of mid
    asm_cmd(JEQ, 0, 2, 0, 71);     // if bit 11 == 0, skip accumulate (jump to pc=71)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 11)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 12)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 12)

    /* bit 12 (pc 73-77) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 12 of mid
    asm_cmd(JEQ, 0, 2, 0, 76);     // if bit 12 == 0, skip accumulate (jump to pc=76)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 12)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 13)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 13)

    /* bit 13 (pc 78-82) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 13 of mid
    asm_cmd(JEQ, 0, 2, 0, 81);     // if bit 13 == 0, skip accumulate (jump to pc=81)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 13)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 14)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 14)

    /* bit 14 (pc 83-87) */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 14 of mid
    asm_cmd(JEQ, 0, 2, 0, 86);     // if bit 14 == 0, skip accumulate (jump to pc=86)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 14)
    asm_cmd(LSF, 3, 3, 1, 1);      // mask <<= 1  (now selects bit 15)
    asm_cmd(LSF, 4, 4, 1, 1);      // shifted_mid <<= 1  (now holds mid << 15)

    /* bit 15 (pc 88-90): last bit — no LSFs needed after */
    asm_cmd(AND, 2, 5, 3, 0);      // R2 = mid & mask  — isolate bit 15 of mid
    asm_cmd(JEQ, 0, 2, 0, 91);     // if bit 15 == 0, skip accumulate (jump to restore block)
    asm_cmd(ADD, 6, 6, 4, 0);      // acc += shifted_mid  (mid << 15) — R6 = mid*mid complete

    // pc=91-93: Restore N, lo, hi (clobbered during multiply)
    asm_cmd(LD, 2, 0, 1, 1000);    // R2 = N  (reload from MEM[1000])
    asm_cmd(LD, 3, 0, 1, 1002);    // R3 = lo (reload from MEM[1002])
    asm_cmd(LD, 4, 0, 1, 1003);    // R4 = hi (reload from MEM[1003])

    // pc=94: if N < mid*mid, jump to hi=mid at pc=97
    asm_cmd(JLT, 0, 2, 6, 97);   // N < mid^2  => hi = mid
    // pc=95: lo = mid + 1  (mid^2 <= N)
    asm_cmd(ADD, 3, 5, 1, 1);
    // pc=96: loop back
    asm_cmd(JEQ, 0, 0, 0, 4);
    // pc=97: hi = mid  (mid^2 > N)
    asm_cmd(ADD, 4, 5, 0, 0);
    // pc=98: loop back
    asm_cmd(JEQ, 0, 0, 0, 4);

    // pc=99-101: Finish
    asm_cmd(SUB, 3, 3, 1, 1);     // R3 = lo - 1 = floor(sqrt(N))
    asm_cmd(ST, 0, 3, 1, 1001);   // MEM[1001] = result
    asm_cmd(HLT, 0, 0, 0, 0);

    mem[1000] = 4937775;           // input N  (sqrt ≈ 2222)

    fp = fopen(program_name, "w");
    for (int i = 0; i <= 1001; i++) fprintf(fp, "%08x\n", mem[i]);
    fclose(fp);
}

int main(int argc, char* argv[]) {
    if (argc == 2) assemble_program(argv[1]);
    return 0;
}