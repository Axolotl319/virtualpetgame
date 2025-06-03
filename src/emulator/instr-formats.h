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

extern const immdp_format_t immdp_format;
extern const regdp_format_t regdp_format;
extern const sdt_format_t sdt_format;
extern const branch_format_t br_format;
extern op0_group_t get_op0_group(unsigned int op0);
