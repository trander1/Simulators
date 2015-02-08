
//accepted LD symbols: 0-9 for digits, and
//operators:
#define BLANK	10	//blank character
#define COIL	11	//(
#define AND	12	//-
#define OR	13	//|
#define NOT	14	//!
#define NODE	15	//+
#define SET 	16	//[ NEW! set
#define RESET	17	//] NEW! reset
#define	DOWN	18	//) NEW! negate coil
//operands
#define	INPUT	19	//i
#define FALLING	20	//f
#define MEMORY	21	//m NEW!
#define COMMAND	22	//c
#define	BLINKOUT 23	//b
#define	RISING	24	//r
#define TIMEOUT 25	//t
#define OUTPUT	 26  	//q
//coils (work for IL too)
#define CONTACT	27	//Q
#define	START	28	//T
#define	PULSEIN	29	//M NEW!
#define WRITE	30	//W
#define END	31	//0

//IL OPCODES: no operand
#define IL_POP		1	//)
#define IL_RET		2	//RET
//arithmetic LABEL
#define IL_JMP		3	//JMP
//subroutine call (unimplemented)
#define IL_CAL		4	//CAL	
//boolean, no modifier
#define IL_SET		5	//S
#define IL_RESET 	6	//R
//boolean, all modifiers
#define IL_AND		7 	//AND
#define IL_OR		8	//OR
#define IL_XOR		9	//XOR
//any operand, only negation
#define IL_LD		10	//LD
#define IL_ST		11	//ST
//any operand, only push
#define IL_ADD		12
#define IL_SUB		13
#define IL_MUL		14
#define IL_DIV		15
#define IL_GT		16
#define IL_GE		17
#define IL_EQ		18
#define IL_NE		19
#define IL_LT		20
#define IL_LE		21
    
#define N_IL_INSN	22
#define NEGATE 128//negate second operand, not return value.
#define BOOL   64
//IL modifiers
#define IL_NEG	1// '!'
#define IL_PUSH	2// '('
#define IL_NORM	3// ' '
#define IL_COND	4//'?'
#define NOP	0

#define RESOLVED 	-1
#define FINAL	2
int extract_number(char * , int );
int read_char(char * , int );
int minmin(int * , int , int );

int LD_task(struct PLC_regs *);
int resolve_operand(struct PLC_regs *,int , int );
int resolve_coil(struct PLC_regs *,int ,int ,int);
int resolve_set(struct PLC_regs *,int ,int ,int);
int resolve_reset(struct PLC_regs *,int ,int,int );
int resolve_lines(struct PLC_regs *);

void push(BYTE , BYTE );
BYTE pop(BYTE );
BYTE operate(BYTE , BYTE , BYTE );

struct opcode
{
    BYTE operation;//AND, OR, XOR, ANDN, ORN, XORN. TODO byte type operations. if op > 128 then value is negated first.
    BYTE value;
    struct opcode * next;
} * Stack;

struct instruction
{
    char label[MAXSTR]; 
    BYTE operation;
    BYTE operand;
    BYTE modifier;
    BYTE byte;
    BYTE bit;
};

int parse_il_line(char * , struct instruction * ,int);
int instruct(struct PLC_regs *, struct instruction *,int *);
