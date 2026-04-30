/*
 * SP ISS: Simple Processor Instruction Set Simulator
 *
 * Usage: iss <program.bin>
 * Outputs: trace.txt, sram_out.txt
 */

#include "iss.h"

 /* ── Global state definitions ── */
unsigned int mem[MEM_SIZE];
unsigned int reg[8];

/* ── Opcode name table ── */
const char* opcode_name(int op)
{
    switch (op) {
    case ADD: return "ADD";
    case SUB: return "SUB";
    case LSF: return "LSF";
    case RSF: return "RSF";
    case AND: return "AND";
    case OR:  return "OR";
    case XOR: return "XOR";
    case LHI: return "LHI";
    case LD:  return "LD";
    case ST:  return "ST";
    case JLT: return "JLT";
    case JLE: return "JLE";
    case JEQ: return "JEQ";
    case JNE: return "JNE";
    case JIN: return "JIN";
    case HLT: return "HLT";
    default:  return "???";
    }
}

/* ── Sign-extend a 16-bit value to 32 bits ── */
int sign_extend16(unsigned int v)
{
    if (v & 0x8000)
        return (int)(v | 0xFFFF0000);
    return (int)v;
}

int main(int argc, char* argv[])
{
    if (argc != 2) {
        printf("usage: iss program_name\n");
        return -1;
    }

    /* ── Load memory image ── */
    FILE* fp = fopen(argv[1], "r");
    if (!fp) {
        printf("couldn't open file %s\n", argv[1]);
        return -1;
    }

    int lines_loaded = 0;
    for (int addr = 0; addr < MEM_SIZE; addr++) {
        if (fscanf_s(fp, "%x", &mem[addr]) != 1)
            break;
        lines_loaded++;
    }
    fclose(fp);

    /* ── Open output files ── */
    FILE* trace = fopen("trace.txt", "w");
    if (!trace) { printf("couldn't open trace.txt\n"); return -1; }

    /* ── Print header ── */
    fprintf(trace, "program %s loaded, %d lines\n\n", argv[1], lines_loaded);

    /* ── Simulator state ── */
    unsigned int pc = 0;
    int inst_count = 0;

    /* ── Main execution loop ── */
    for (;;) {
        /* Fetch */
        unsigned int inst = mem[pc & MEM_MASK];

        /* Decode */
        int opcode = (inst >> 25) & 0x1F;
        int dst = (inst >> 22) & 0x07;
        int src0 = (inst >> 19) & 0x07;
        int src1 = (inst >> 16) & 0x07;
        unsigned int imm16 = inst & 0xFFFF;
        int imm_signed = sign_extend16(imm16);

        /* r1 = sign-extended immediate of CURRENT instruction */
        reg[0] = 0;
        reg[1] = (unsigned int)imm_signed;

        /* Resolve source values */
        unsigned int v0 = reg[src0];
        unsigned int v1 = reg[src1];

        /* ── Print trace header for this instruction ── */
        fprintf(trace,
            "--- instruction %d (%04x) @ PC %d (%04x) ---"
            "--------------------------------------------------------\n",
            inst_count, inst_count, pc, pc);

        fprintf(trace,
            "pc = %04x, inst = %08x, opcode = %d (%s), "
            "dst = %d, src0 = %d, src1 = %d, immediate = %08x\n",
            pc, inst, opcode, opcode_name(opcode),
            dst, src0, src1, (unsigned int)imm_signed);

        fprintf(trace,
            "r[0] = %08x r[1] = %08x r[2] = %08x r[3] = %08x \n",
            reg[0], reg[1], reg[2], reg[3]);
        fprintf(trace,
            "r[4] = %08x r[5] = %08x r[6] = %08x r[7] = %08x \n",
            reg[4], reg[5], reg[6], reg[7]);
        fprintf(trace, "\n");

        /* ── Execute ── */
        int jump_taken = 0;
        unsigned int jump_target = 0;

        switch (opcode) {

            /* Arithmetic / Logic */
        case ADD:
            fprintf(trace, ">>>> EXEC: R[%d] = %d ADD %d <<<<\n",
                dst, (int)v0, (int)v1);
            if (dst) reg[dst] = v0 + v1;
            break;

        case SUB:
            fprintf(trace, ">>>> EXEC: R[%d] = %d SUB %d <<<<\n",
                dst, (int)v0, (int)v1);
            if (dst) reg[dst] = v0 - v1;
            break;

        case LSF:
            fprintf(trace, ">>>> EXEC: R[%d] = %d LSF %d <<<<\n",
                dst, (int)v0, (int)v1);
            if (dst) reg[dst] = v0 << (v1 & 31);
            break;

        case RSF:
            fprintf(trace, ">>>> EXEC: R[%d] = %d RSF %d <<<<\n",
                dst, (int)v0, (int)v1);
            if (dst) reg[dst] = (unsigned int)((int)v0 >> (v1 & 31));
            break;

        case AND:
            fprintf(trace, ">>>> EXEC: R[%d] = %d AND %d <<<<\n",
                dst, (int)v0, (int)v1);
            if (dst) reg[dst] = v0 & v1;
            break;

        case OR:
            fprintf(trace, ">>>> EXEC: R[%d] = %d OR %d <<<<\n",
                dst, (int)v0, (int)v1);
            if (dst) reg[dst] = v0 | v1;
            break;

        case XOR:
            fprintf(trace, ">>>> EXEC: R[%d] = %d XOR %d <<<<\n",
                dst, (int)v0, (int)v1);
            if (dst) reg[dst] = v0 ^ v1;
            break;

        case LHI:
            fprintf(trace, ">>>> EXEC: R[%d] = LHI %d <<<<\n",
                dst, (int)v1);
            if (dst) reg[dst] = (reg[dst] & 0x0000FFFF) | ((imm16 & 0xFFFF) << 16);
            break;

            /* Load / Store */
        case LD: {
            unsigned int addr = v1 & MEM_MASK;
            fprintf(trace, ">>>> EXEC: R[%d] = MEM[%d] = %08x <<<<\n",
                dst, addr, mem[addr]);
            if (dst) reg[dst] = mem[addr];
            break;
        }

        case ST: {
            unsigned int addr = v1 & MEM_MASK;
            fprintf(trace, ">>>> EXEC: MEM[%d] = R[%d] = %08x <<<<\n",
                addr, src0, v0);
            mem[addr] = v0;
            break;
        }

               /* Flow Control */
        case JLT:
            if ((int)v0 < (int)v1) {
                jump_taken = 1;
                jump_target = imm16 & MEM_MASK;
            }
            fprintf(trace, ">>>> EXEC: JLT %d, %d, %d <<<<\n",
                (int)v0, (int)v1, imm16);
            break;

        case JLE:
            if ((int)v0 <= (int)v1) {
                jump_taken = 1;
                jump_target = imm16 & MEM_MASK;
            }
            fprintf(trace, ">>>> EXEC: JLE %d, %d, %d <<<<\n",
                (int)v0, (int)v1, imm16);
            break;

        case JEQ:
            if (v0 == v1) {
                jump_taken = 1;
                jump_target = imm16 & MEM_MASK;
            }
            fprintf(trace, ">>>> EXEC: JEQ %d, %d, %d <<<<\n",
                (int)v0, (int)v1, imm16);
            break;

        case JNE:
            if (v0 != v1) {
                jump_taken = 1;
                jump_target = imm16 & MEM_MASK;
            }
            fprintf(trace, ">>>> EXEC: JNE %d, %d, %d <<<<\n",
                (int)v0, (int)v1, imm16);
            break;

        case JIN:
            /* Unconditional indirect jump — always taken */
            jump_taken = 1;
            jump_target = v0 & MEM_MASK;
            fprintf(trace, ">>>> EXEC: JIN %d <<<<\n", (int)v0);
            break;

        case HLT:
            fprintf(trace, ">>>> EXEC: HALT at PC %04x<<<<\n", pc);
            fprintf(trace, "sim finished at pc %d, %d instructions\n",
                pc, inst_count + 1);
            goto halt;

        default:
            /* Unknown opcode — treat as NOP (hardware-friendly) */
            fprintf(trace, ">>>> EXEC: UNKNOWN OPCODE %d (NOP) <<<<\n", opcode);
            break;
        }

        /* Save return address to r7 if jump taken */
        if (jump_taken) {
            reg[7] = pc;
            pc = jump_target;
        }
        else {
            pc = (pc + 1) & MEM_MASK;
        }

        /* r0 is always 0 */
        reg[0] = 0;

        fprintf(trace, "\n");
        inst_count++;
    }

halt:
    fclose(trace);

    /* ── Write sram_out.txt ── */
    FILE* sram = fopen("sram_out.txt", "w");
    if (!sram) { printf("couldn't open sram_out.txt\n"); return -1; }
    for (int i = 0; i < MEM_SIZE; i++)
        fprintf(sram, "%08x\n", mem[i]);
    fclose(sram);

    return 0;
}