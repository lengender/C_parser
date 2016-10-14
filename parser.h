#ifndef _PARSER_H_
#define _PARSER_H_

#include<stdio.h>
#include<stdlib.h>
#include<memory.h>
#include<string.h>
#include<fcntl.h>


extern int debug;  //打印调试指令
extern int assembly;  

extern int token;    //当前标记
extern char *src, *old_src;   //指向源代码字符串的指针
extern int poolsize;  //定义数据块大小
extern int line;   //行号

extern int *text; //代码段
extern int *old_text; //用于转储代码段
extern int *stack;  //栈
extern char *data;   //数据段

extern int *PC; // 虚拟机寄存器
extern int *BP;
extern int *SP;
extern int AX;
extern int cycle;

//指令集
enum {
	LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, PUSH,
	OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD,
	OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT
};

//标记和类（操作符和优先顺序）
enum{
	Num = 128, Fun, Sys, Glo, Loc, Id,
	Char, Else, Enum, If, Int, Return, Sizeof, While, 
	Assign, Cond, Lor, Lan, Or, Xor, And, Eq, Ne, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div, Mod, Inc, Dec, Brak
};

extern int token_val;      //标识符的值(主要是整数)
extern int *current_id;    //当前解析ID
extern int *symbols;       //符号表

//标识符的几个标识
enum{
	Token, Hash, Name, Type, Class, Value, BType, BClass, BValue, IdSize
};

//用以表示标识符的类型(type)
enum{
	CHAR, INT, PTR
};

extern int *idmain;   // the 'main' function

extern int basetype;    //声明一个基础类型，使其在全局使用更加方便
extern int expr_type;   //表达式类型

extern int index_of_bp;     //函数调用时，new_bp 在栈中的指针位置


void next();
void match(int tk);
void enum_declaration();
void statements();
void function_parameter();
void function_body();
void function_declaration();
void global_declaration();
void pargram();
void expression(int level);
int eval();
#endif


