#include"parser.h"


int token;    //当前标记
char *src, *old_src;   //指向源代码字符串的指针
int poolsize;  //定义数据块大小
int line;   //行号

int *text; //代码段
int *old_text; //用于转储代码段
int *stack;  //栈
char *data;   //数据段


int token_val;      //标识符的值(主要是整数)
int *current_id;    //当前解析ID
int *symbols;       //符号表


void next()
{
	char *last_pos;
	int hash;

	while(token = *src)
	{
    	//printf("next(): token %d ,  *src ==%c==\n", token, *src);
		++src;

		if(token == '\n')    //遇到换行符，当前行号加一
		{
			if(assembly)
			{
				printf("%d: %.*s", line, src - old_src, old_src);
				old_src = src;

				while(old_text < text)
				{
					printf("%8.4s", & "LEA, IMM, JMP, CALL, JZ, JNZ, ENT, ADJ, LEV, LI, LC, SI, SC, PUSH, OR, XOR, AND, EQ, NE, LT, GT, LE, GE, SHL, SHR, ADD, SUB, MUL, DIV, MOD, OPEN, READ, CLOS, PRTF, MALC, MSET, MCMP, EXIT"[*++old_text * 15]);

					if(*old_text <= ADJ)
					    printf(" %d\n", *++old_text);
					else printf("\n");
				}
			}
			++line;
			
		}
		else if(token == '#') //编译器不支持宏定义，直接跳过
		{
			while(*src != 0 && *src != '\n')
				src++;
		}
		else if((token >= 'a' && token <= 'z') || (token >= 'A' && token <= 'Z') || (token == '_'))
		{
			
			//解析标识符
			last_pos = src - 1;
			hash = token;

			while((*src >= 'a' && *src <= 'z') || (*src >= 'A' && *src <= 'Z') || (*src >= '0' && *src <= '9') || (*src == '_'))
			{
				hash = hash * 147 + *src;
				src++;
			}
//            hash = (hash << 6) + (src - last_pos);   
			//查找已经存在的标识符
			current_id = symbols;
			
			while(current_id[Token])
			{
				if(current_id[Hash] == hash && !memcmp((char*)current_id[Name], last_pos, src - last_pos))
				{
					//找到，返回
					token = current_id[Token];
					return;
				}
				current_id = current_id + IdSize;
			}
			//保存新的ID
			current_id[Name] = (int)last_pos;
			current_id[Hash] = hash;
			token = current_id[Token] = Id;

			return;
		}
		else if(token >= '0' && token <= '9') 
		{
			//解析数字，三种情况，十进制,十六进制(0x),八进制(0)
			token_val = token - '0';
			if(token_val > 0)
			{
				//十进制
				while(*src >= '0' && *src <= '9')
					token_val = token_val * 10 + *src++ - '0';
			}else{
				if(*src == 'x' || *src == 'X')
				{
					//十六进制
					token = *++src;
					while((token) >= '0' && token <= '9' || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F'))
					{
						token_val = token_val * 16 + (token & 15) + (token >= 'A' ? 9 : 0);
						token = *++src;
					}
				}else{
					//八进制
					while(*src >= '0' && *src <= '7')
						token_val = token_val * 8 + (*src++ - '0');
				}
			}
			
			token = Num;
			return;
		}
		else if(token == '"' || token == '\'')
		{
			//解析字符串,转义字符除'\n'，其他都不支持，将解析出的字符串存储在data中
			last_pos = data;
			while(*src != 0 && *src !=token)
			{
				token_val = *src++;
				if(token_val == '\\')
				{
					//转移字符
					token_val = *src++;
					if(token_val == 'n')
						token_val = '\n';
				}

				if(token == '"')
					*data++ = token_val;
			}

			src++;
			//如果是单个字符，作为Num返回
			if(token == '"')
				token_val = (int)last_pos;
			else token = Num;

			return;
		}
		else if(token == '/')   //解析注释，支持//, 不支持/**/
		{
			if(*src == '/')
			{
				//跳过注释
				while(*src != 0 && *src != '\n')
					++src;
			}else{
				//除号
				token = Div;
				return;
			}
		}
		else if(token == '=')
		{
			
		  // printf("dao zhe li lai le ma ? \n");
			//解析= 和 ==
			if(*src == "=")
			{
				src++;
				token = Eq;
			}
			else token = Assign;
			return;
		}
		else if(token == '+')
		{
			//解析 + 和 ++
			if(*src == '+')
			{
				src++;
				token = Inc;
			}
			else token = Add;
			return;
		}
		else if(token == '-')
		{
			//解析　- 和 --
			if(*src == '-')
			{
				src++;
				token = Dec;
			}
			else token = Sub;
			return;
		}
		else if(token == '!')
		{
			//解析!=
			if(*src == '=')
			{
				src++;
				token = Ne;
			}
			return;
		}
		else if(token == '<')
		{
			//解析 <   <=  <<
			if(*src == '=')
			{
				src++;
				token = Le;
			}else if(*src == '<'){
				src++;
				token = Shl;
			}else token = Lt;
			return;
		}
		else if(token == '>')
		{
			//解析　> 　>=   >>
			if(*src == '=')
			{
				src++;
				token = Ge;
			}else if(token == '>'){
				src++;
				token = Shr;
			}else token = Gt;
			return;
		}
		else if(token == '|')
		{
			//解析　|  ||
			if(*src == '|')
			{
				src++;
				token = Lor;
			}else token = Or;
			return;
		}
		else if(token == '&')
		{
			//解析　&  &&
			if(*src == '&')
			{
				src++;
				token = Lan;
			}else token = And;
			return;
		}
		else if(token == '^')
		{
			token = Xor;
			return;
		}
		else if(token == '%')
		{
			token = Mod;
			return;
		}
		else if(token == '*')
		{
			token = Mul;
			return;
		}
		else if(token == '[')
		{
			token = Brak;
			return;
		}
		else if(token == '?')
		{
			token = Cond;
			return;
		}
		else if(token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':')
		    return;  // 作为标识符直接返回

	}
	return;
}

