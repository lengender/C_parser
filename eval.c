#include"parser.h"

int *text; //代码段
int *old_text; //用于转储代码段
int *stack;  //栈
char *data;   //数据段

int *PC, *BP, *SP, AX, cycle; // 虚拟机寄存器


int eval()
{
	int op, *tmp;
	cycle = 0;
	while(1)
	{
	   cycle++;
		op = *PC++; // 得到下一个指令

		if(debug)
		{
			printf("%d> %.4s", cycle, &"LEA, IMM , JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, PUSH, OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD ,SUB, MUL, DIV, MOD, OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT"[op * 5]);
			if(op <= ADJ)
				printf("　%d\n", *PC);
			else printf("\n");
		}
		
		if(op == IMM)
		{
			AX = *PC++;  //将当前PC指向的值赋给AX
		}
		else if(op == LC)
		{
			AX = *(char*)AX; //将当前AX中字符地址赋给AX
		}
		else if(op == LI) 
		{
			AX = *(int*)AX;   //将当前AX中整数地址赋给AX
		}
		else if(op == SC)
		{
			AX = *(char*)*SP++ = AX;  //将AX中的数据作为字符存放地址中，要求栈顶存放地址
		}
		else if(op == SI)
		{
			*(int*)*SP++ = AX;  //将AX中的数据作为整数存放入地址中，要求栈顶存放地址 
		}
		else if(op == PUSH)
		{
			*--SP = AX;  //将AX中的值存放入栈中　
		}
		else if(op == JMP)
		{	
			PC = (int*)*PC;  //跳转
		}
		else if(op == JZ)
		{
			PC = AX ? PC + 1 : (int *)*PC;// AX中的值为零或者不为零跳转
		}
		else if(op == JNZ)
		{
			PC = AX ? (int *)*PC : PC + 1;
		}
		else if(op == CALL) 
		{ 
			*--SP = (int)(PC + 1); 
			PC = (int*)*PC;                  //调用子函数过程
		}
		else if(op == ENT) 
		{
			*--SP = (int)BP;
			BP = SP; 
			SP = SP - *PC++;             // 生成新的栈框架，为子函数调用分配内存
		}
		else if(op == ADJ)  
		{
		    SP = SP + *PC++;  // add esp, <size>
		}
		else if(op == LEV) 
		{
			SP = BP; 
			BP = (int*)*SP++; 
			PC = (int*)*SP++;           //restore call frame and PC
		}
		else if(op == LEA)
		{
			AX = (int)(BP + *PC++); //load address for arguments
		}
		else if(op == OR)
		{
			AX = *SP++ | AX;            //操作符
		}
		else if(op == XOR) AX = *SP++ ^ AX;
		else if(op == AND) AX = *SP++ & AX;
		else if(op == EQ) AX = *SP++ == AX;
		else if(op == NE) AX = *SP++ != AX;
		else if(op == LT) AX = *SP++ < AX;
		else if(op == LE) AX = *SP++ <= AX;
		else if(op == GT) AX = *SP++ > AX;
		else if(op == GE) AX = *SP++ >= AX;
		else if(op == SHL) AX = *SP++ << AX;
		else if(op == SHR) AX = *SP++ >> AX;
		else if(op == ADD) 
		{
			AX = *SP++ + AX;
		}
		else if(op == SUB)
		{
			AX = *SP++ - AX;
		}
		else if(op == MUL)
		{
			AX = *SP++ * AX;
		}
		else if(op == DIV)
		{
			AX = *SP++ / AX;
		}
		else if(op == MOD) AX = *SP++ % AX;

		else if(op == EXIT) { printf("exit(%d)\n", *SP); return *SP;}     //预定义函数
		else if(op == OPEN) { AX = open((char*)SP[1], SP[0]);}
		else if(op == CLOS) { AX = close(*SP);}
		else if(op == READ) { AX = read(SP[2], (char*)SP[1], *SP);}
		else if(op == PRTF) { tmp = SP + PC[1]; AX = printf((char*)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);}
		else if(op == MALC) { AX = (int)malloc(*SP);}
		else if(op == MSET) { AX = (int)memset((char*)SP[2], SP[1], *SP);}
		else if(op == MCMP) { AX = memcmp((char*)SP[2], (char*)SP[1], *SP);}  
		else 
		{
			printf("unknown instruction: %d\n", op);
			return -1;
		}
	}
	return 0;
	
}

