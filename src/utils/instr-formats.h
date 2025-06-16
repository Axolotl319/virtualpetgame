#ifndef INSTR_FORMATS_H
#define INSTR_FORMATS_H

#include <stdint.h>

#define OP0_INDEX 25 //op0 index
#define OP0_BITS 4   //num op0 bits
#define IMMDP_MASK 0xE //IMMDP relevant bits from op0
#define REGDP_MASK 0x7  //REGDP relevant bits
#define LDSTR_MASK 0x4  //Load/Store relevant bits
#define BR_MASK 0xE     //Branch relevant bits
#define IMMDP_CODE 0x8  //IMMDP op0 code
#define REGDP_CODE 0x5  //REGDP op0 code
#define LDSTR_CODE 0x4  //Load Store op0 code
#define BR_CODE 0xA     //Branch op0 code

typedef enum {
	IMMDP_GROUP,
	REGDP_GROUP,
	LDSTR_GROUP,
	BR_GROUP,
	OP0_INVALID
} op0_group_t;

//struct for extracting bits from 32 bit instruction
typedef struct {
	int index;
	int bits;
} bit_range_t;

//immdp instruction format
typedef struct {
	bit_range_t rd;
	bit_range_t rn;
	bit_range_t imm12;
	bit_range_t sh;
	bit_range_t imm16;
	bit_range_t hw;
	bit_range_t opi;
	bit_range_t opc;
	bit_range_t sf;
} immdp_format_t;

//regdp instruction format
typedef struct {
	bit_range_t rd;
	bit_range_t rn;
	bit_range_t operand;
	bit_range_t ra;
	bit_range_t x;
	bit_range_t rm;
	bit_range_t opr;
	bit_range_t N;
	bit_range_t shift;
	bit_range_t M;
	bit_range_t opc;
	bit_range_t sf;
} regdp_format_t;

//single data transfer format
typedef struct {
	bit_range_t rt;
	bit_range_t simm19;
	bit_range_t xn;
	bit_range_t offset;
	bit_range_t I;
	bit_range_t simm9;
	bit_range_t R;
	bit_range_t xm;
	bit_range_t L;
	bit_range_t U;
	bit_range_t sf;
	bit_range_t type;
} sdt_format_t;

typedef struct {
	bit_range_t simm26;
	bit_range_t xn;
	bit_range_t cond;
	bit_range_t simm19;
	bit_range_t op;
} branch_format_t;

//branch conditions
typedef enum branch_cond {
	BR_EQ,       //equal
	BR_NE,       //not equal
	BR_GE = 0xA, //signed greater than or equal - 0b1010
	BR_LT,       //signed less than
	BR_GT,	     //signed greater than
	BR_LE,       //signed less than or equal
	BR_AL,       //always
} branch_cond;

//operation types
typedef enum operation {
	OP_ADD,
	OP_SUB,
	OP_LOGIC
} operation;

//arithmetic opcode
typedef enum arith_opc {
	ARITH_ADD,  //add
	ARITH_ADDS, //add and set flags
	ARITH_SUB,  //sub
	ARITH_SUBS  //sub and set flags
} arith_opc;

//wide move opc
typedef enum mov_opc {
	MOVN,        //move wide with NOT
	MOV_INVALID, //no move instr corresponding to 0b01
	MOVZ,        //move wide with zero
	MOVK         //move wide with keep
} mov_opc;

//shift type
typedef enum shift_t {
	LSL, //logical shift left
	LSR, //logical shift right
	ASR, //arithmetic shift right
	ROR  //rotate right
} shift_t;

//log operation type
typedef enum log_type_t {
	LOG_AND, 
	LOG_OR,
	LOG_XOR,
	LOG_ANDS //and set flags
} log_type_t;

extern const immdp_format_t immdp_format;
extern const regdp_format_t regdp_format;
extern const sdt_format_t sdt_format;
extern const branch_format_t br_format;
extern op0_group_t get_op0_group(unsigned int op0);

#endif
