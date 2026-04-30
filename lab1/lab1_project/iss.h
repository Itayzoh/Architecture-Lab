#ifndef ISS_H
#define ISS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Opcodes ── */
#define ADD  0
#define SUB  1
#define LSF  2
#define RSF  3
#define AND  4
#define OR   5
#define XOR  6
#define LHI  7
#define LD   8
#define ST   9
#define JLT  16
#define JLE  17
#define JEQ  18
#define JNE  19
#define JIN  20
#define HLT  24

/* ── Memory ── */
#define MEM_SIZE_BITS 16
#define MEM_SIZE      (1 << MEM_SIZE_BITS)   /* 65536 words */
#define MEM_MASK      (MEM_SIZE - 1)

/* ── Global state ── */
extern unsigned int mem[MEM_SIZE];
extern unsigned int reg[8];

/* ── Helper function declarations ── */
const char* opcode_name(int op);
int sign_extend16(unsigned int v);

#endif /* ISS_H */