#include"parser.h"

int basetype;    //声明一个基础类型，使其在全局使用更加方便
int expr_type;   //表达式类型

int index_of_bp;     //函数调用时，new_bp 在栈中的指针位置


void match(int tk)
{
	if(token == tk)
		next();
	else{
		printf("%d: expected token : %d\n", line , tk);
		exit(-1);
	}
}


void enum_declaration()
{
	//解析 enmu [id] { a = 1, b = 3, ...}
	int i;
	i = 0;
	while(token != '}')
	{
		if(token != Id)
		{
			printf("%d: bad enmu identifier %d\n", line, token);
			exit(-1);
		}
		next();
		if(token == Assign)
		{
			//like { a = 1}
			next();
			if(token != Num)
			{
				printf("%d: bad enum initializer\n", line);
				exit(-1);
			}
			i = token_val;
			next();
		}

		current_id[Class] = Num;
		current_id[Type] = INT;
		current_id[Value] = i++;

		if(token == ',')
			next();
	}
}


void statement()
{
	//编译器支持６种语句
	// 1. if(...) <statement> [else <statement>]
	// 2. while(...) <statement>
	// 3. { <statement>}
	// 4. return xxx
	// 5. <empty statement>
	// 6. expression ;

	int *a, *b;   // 分支控制标识

	if(token == If)
	{
		// if(...) <statement> [else <statement>]
		// 
		//   if(...)                       <cond>
		//                                 JZ a
		//      <true_statement>            <true_statement>
		//   else                           JMP b
		// a:                               a:
		//      <false_statement>              <false_statement>
		// b:                               b:
		//
		
		match(If);
		match('(');
		expression(Assign);     // 解析条件判断式
		match(')');

		//保存if过程
		*++text = JZ;
		b = ++text; 

		statement();   // 解析语句
		if(token == Else)
		{
			//解析else
			match(Else);

			*b = (int)(text + 3);
			*++text = JMP;
			b = ++text;

			statement();
		}

		*b = (int)(text + 1);
	}
	else if(token == While)
	{
		//   a:                              a:
		//      while(<cond>)                  <cond>
		//                                      JZ   b
		//           <statement>                <statement>
		//                                      JMP a
		//   b:

		match(While);
		a = text + 1;

		match('(');
		expression(Assign);
		match(')');

		*++text = JZ;
		b = ++text;

		statement();

		*++text = JMP;
		*++text = (int)a;
		*b = (int)(text + 1);
	}
	else if(token == '{')
	{
		// {  <statement> }
		match('{');

		while(token != '}')
		{
			statement();
		}

		match('}');
	}
	else if(token == Return)
	{
		//return [expression];
		match(Return);

		if(token != ';')
		{
			expression(Assign);
		}

		match(';');

		//保存return语句
		*++text = LEV;
	}
	else if(token == ';')
	{
		// empty statement
		match(';');
	}
	else
	{
		// a = b; or function_call()
		expression(Assign);
		match(';');
	}
}


void function_parameter()
{
	int type;
	int params;
	params = 0;

	while(token != ')')
	{
		//int  name
		type = INT;
		if(token == Int)
			match(Int);
		else if(token = Char)
		{
			type = CHAR;
			match(Char);
		}

		//指针类型
		while(token == Mul)
		{
			match(Mul);
			type = type + PTR;
		}

		//参数名字
		if(token != Id)
		{
			printf("%d: bad parameter declaration.\n", line);
			exit(-1);
		}
		if(current_id[Class] == Loc)
		{
			printf("%d: duplicate parameter declaration.\n", line);
			exit(-1);
		}

		match(Id);

		//保存局部变量
		current_id[BClass] = current_id[Class];
		current_id[Class] = Loc;
		
		current_id[BType] = current_id[Type];
		current_id[Type] = type;

		current_id[BValue] = current_id[Value];
		current_id[Value] = params++;    //局部参数的位置

		if(token == ',')
			match(',');
	}

	index_of_bp = params + 1;     //保存　new_bp　的位置
}

void function_body()
{
	// type func_name(...) { ...}
	//
	//...{
	//  1. 局部变量声明
	//  2. 语句
	//   }

	int pos_local;   //局部变量在栈中的位置
	int type;
	pos_local = index_of_bp;

	while(token == Int || token == Char)
	{
		//局部变量声明
		basetype = (token == Int) ? INT : CHAR;
		match(token);

		while(token != ';')
		{
			type = basetype;
			while(token == Mul)
			{
				match(Mul);
				type = type + PTR;
			}

			if(token != Id)
			{
				//无效声明
				printf("%d: bad local declaration.\n", line);
				exit(-1);
			}

			if(current_id[Class] == Loc)
			{
				//重复声明
				printf("%d: duplicate local declaration.\n", line);
				exit(-1);
			}
	        match(Id);

			//保存局部变量
			current_id[BClass] = current_id[Class];
			current_id[Class] = Loc;

			current_id[BType] = current_id[Type];
			current_id[Type] = type;

			current_id[BValue] = current_id[Value];
			current_id[Value] = ++pos_local;    //保存现在参数的位置
			

			if(token == ',')
				match(',');
		}
		match(';');
	}

	//保存局部变量在栈中的大小
	*++text = ENT;
	*++text = pos_local - index_of_bp;

	//语句部分
	while(token != '}')
	{
		statement();
	}

	//退出子函数时，恢复到调用前的状态
	*++text = LEV;
}


void function_declaration()
{
	// type func_name(...) { ... }

	match('(');
	function_parameter();
	match(')');
	match('{');
	function_body();
	//match('}');        // variable_decl 与 function_decl　是放在一起解析的，而variable_decl是以字符;结束的。
	                     // 而function_decl是以字符}结束的，若在此消耗了｝，那么外层的while就不知道函数定义已经结束，所以将
						 //　结束符的解析放在外层的while中

	current_id = symbols;                                //本段的作用是将符号表中的信息恢复成全局变量，这是因为，局部变量是可以
	while(current_id[Token])                             //和全局变量同名的。一旦同名，在函数体内局部变量就会覆盖全局变量，出了函数体
	{                                                    //,全局变量就恢复了原先的作用。这段代码线性遍历所有标识符，并将保存在BXXX 中的信息还原
		if(current_id[Class] == Loc)
		{
			current_id[Class] = current_id[BClass];
			current_id[Type] = current_id[BType];
			current_id[Value] = current_id[BValue];
		}
		current_id = current_id + IdSize;
	}
}

void global_declaration()
{
	// global_declaration ::= enum_decl  | variable_decl | function_decl
	//
	//enum_decl ::= 'enum' [id] '{' id ['=' 'num'] {',' id ['=' 'num']} '}'
	//
	//variable_decl ::= type {'*'} id {',' {'*'} id } ';'
	//
	// function_decl ::= type {'*'} id '(' parameter_decl ')' '{' body_decl '}'

	int type;   //tmp, 实际变量类型
	int i;      //tmp

	basetype = INT;

	//解析　enum
	if(token == Enum)
	{
		//enum [id] { a = 10, b = 20, ...}
		match(Enum);
		if(token != '{')
			match(Id);      //跳过[id]部分
		if(token == '{')
		{
			match('{');
			enum_declaration();
			match('}');
		}

		match(';');
		return;
	}
	//解析不同类型
	if(token == Int)
	{
		match(Int);
	}else if(token == Char){
		match(Char);
		basetype = CHAR;
	}
	
	//解析逗号分隔的变量声明
	while(token != ';' && token != '}')
	{
		type = basetype;
        
		//解析指针类型，有可能存在　int ***x;
		while(token == Mul)
		{
			match(Mul);
			type = type + PTR;
		}

		if(token != Id)
		{
			//无效声明
			printf("%d: id  bad global declaration.\n", line);
			exit(-1);
		}
		if(current_id[Class])
		{
			//标识符已经存在
			printf("%d: duplicate gloabl declaration.\n", line);
			exit(-1);
		}
     

		match(Id);
		current_id[Type] = type;
		if(token == '(') 
		{
			current_id[Class] = Fun;
			current_id[Value] = (int)(text + 1);   // 函数的内存地址

			function_declaration();
			

		}else{
			//变量声明
			current_id[Class] = Glo;    // 全局变量
			current_id[Value] = (int)data;   // 分配内存地址
			data = data + sizeof(int);
		}

		
		if(token == ',')
			match(',');
	}
	next();
}


void  program()
{
	next(); //get next token
	while(token > 0)
	{
		global_declaration();
	}
}


void expression(int level)
{
	int *id;
	int tmp;
	int *addr;
	
	//一元运算符
	{
		if(!token)
		{
			printf("%d: unexpected token EOF of expression.\n", line);
			exit(-1);
		}
		if(token == Num)   // 常量
		{
			match(Num);
           
			//使用IMM 直接加载到AX中
			*++text = IMM;
			*++text = token_val;
			expr_type = INT;
		}
		else if(token == '"')
		{
			//连续的字符串 "abc" "abc"

			//加载代码
			*++text = IMM;
			*++text = token_val;

			match('"');

			//保存剩下的字符串
			while(token == '"')
			{
				match('"');
			}

			//在字符串末尾追加\0,所有的data默认都是0
			data = (char*)(((int)data + sizeof(int)) & (-sizeof(int)));
			expr_type = PTR;
		}
		else if(token == Sizeof)
		{
			//sizeof　实际上是一个一元操作符
			//现只支持　sizeof(int) sizeof(char),  sizeof(*...)
			//

			match(Sizeof);
			match('(');
			expr_type = INT;

			if(token == Int)
			{
				match(Int);
			}else if(token == Char){
				match(Char);
				expr_type = CHAR;
			}

			while(token == Mul)
			{
				match(Mul);
				expr_type = expr_type + PTR;
			}

			match(')');

			//加载目标代码
			*++text = IMM;
			*++text = (expr_type == CHAR) ? sizeof(char) : sizeof(int);

			expr_type = INT;
		}
		else if(token == Id)
		{
			//当标识符为Id时，可能出现以下情况
			//因为这是一元操作符解析,所以只能是
			// 1. 函数调用
			//2. Enum变量
			// 3. 全局/局部变量

			match(Id);
			
			id = current_id;

			if(token == '(')
			{
				//函数调用
				match('(');

				//传入参数
				tmp = 0;  //记录参数个数
				while(token != ')')
				{
					expression(Assign);
					*++text = PUSH;
					tmp++;

					if(token == ',')
					{
						match(',');
					}
				}
				match(')');
               
				//加载目标代码
				if(id[Class] == Sys)
				{
					//系统函数
	     			*++text = id[Value];
				}else if(id[Class] == Fun){
					//函数调用
					*++text = CALL;
					*++text = id[Value];
				}else{
					printf("%d: bad function call.\n", line);
					exit(-1);
				}

				//清理栈的参数,由于我们不在乎出栈的值，直接修改栈指针的大小即可
				if(tmp > 0)
				{
					*++text = ADJ;
					*++text = tmp;
				}
				expr_type = id[Type];
			}
			else if(id[Class] == Num)
			{
				//enum 变量
				*++text = IMM;
				*++text = id[Value];
				expr_type = INT;
			}
			else
			{
				//全局/局部变量
				if(id[Class] == Loc)
				{
					*++text = LEA;
					*++text = index_of_bp - id[Value];
                }
				else if(id[Class] == Glo)
				{
					*++text = IMM;
					*++text = id[Value];
				}
				else
				{
					printf("%d: undefined variable.\n", line);
					exit(-1);
				}

				//加载代码，默认是将值的地址保存到AX中
				expr_type = id[Type];
				*++text = (expr_type == Char) ?  LC  : LI;
			}
		}
		else if(token == '(')
		{
			//强制转换或者括号
			match('(');

			if(token == Int || token == Char)
			{
				tmp = (token == Char) ? CHAR : INT; // 强制类型
				match(token);
				while(token == Mul)
				{
					match(Mul);
					tmp = tmp + PTR;
				}

				match(')');
				expression(Inc);  //强制转换有同自增一样的优先级
				expr_type = tmp;
			}
			else
			{
				//正常括号
				expression(Assign);
				match(')');
			}
		}
		else if(token == Mul)
		{
			//解引用　*<addr>

			match(Mul);
			expression(Inc);   //解引用同自增优先级

			if(expr_type >= PTR)
			{
				expr_type = expr_type - PTR;
			}else{
				printf("%d: bad dereference.\n", line);
				exit(-1);
			}

			*++text = (expr_type == CHAR) ? LC : LI;
		}

		else if(token == And)
		{    
		    //取地址操作
			match(And);
			expression(Inc);
			if(*text == LC || *text == LI)
			{
				text--;
			}else{
				printf("%d: bad address of.\n", line);
				exit(-1);
			}

			expr_type = expr_type + PTR;
		}
		else if(token == '!')
		{
			//逻辑取反
			match('!');
			expression(Inc);

			//加载代码, 通过判断是否与０相等
			*++text = PUSH;
			*++text = IMM;
			*++text = 0;
			*++text = EQ;

			expr_type = INT;
		}
		else if(token == '~')
		{
			//按位取反
			match('~');
			expression(Inc);

			//加载代码,使用　<expr> XOR -1
			*++text = PUSH;
			*++text = IMM;
			*++text = -1;
			*++text = XOR;

			expr_type = INT;
		}
		else if(token == Add)
		{
			// + 号
			match(Add);
			expression(Inc);

			expr_type = INT;
		}
		else if(token == Sub)
		{
			// - 号

			if(token == Num)
			{
				*++text = IMM;
				*++text = -token_val;
				match(Num);
			}else{
				*++text = IMM;
				*++text = -1;
				*++text = PUSH;
				expression(Inc);
				*++text = MUL;
			}

			expr_type = INT;
		}
		else if(token == Inc || token == Dec)
		{
			tmp = token;
			match(token);
			expression(Inc);

			//实现++p时，需要使用变量p的地址两次，所以先push
			if(*text == LC)
			{
				*text = PUSH;
				*++text = LC;
			}else if(*text == LI){
				*text = PUSH;
				*++text = LI;
			}else{
				printf("%d: bad lvalue of pre-increment.\n", line);
				exit(-1);
			}

			*++text = PUSH;
			*++text = IMM;

			//自增自减还需要处理时指针的情形
			*++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
			*++text = (tmp == Inc) ? ADD : SUB;
			*++text = (expr_type == CHAR) ? SC : SI;
		}
	}

    //二元操作
	{
		while(token >= level)
		{
			//根据当前操作的优先级
			tmp = expr_type;

			if(token == Assign)
			{
				// 等号表达式
				match(Assign);
				if(*text == LC || *text == LI)
				{
					*text = PUSH;  // save lvalue's pointer
				}else{
					printf("%d: bad lvalue in assignment\n", line);
					exit(-1);
				}

				expression(Assign);

				expr_type = tmp;
				*++text = (expr_type == CHAR) ? SC : SI;
			}
			else if(token == Cond)
			{
				//三目运算符
				match(Cond);
				*++text = JZ;
				addr = ++text;
				expression(Assign);
				if(token == ':')
				{
					match(':');
				}else{
					printf("%d: missing colon in conditional.\n", line);
					exit(-1);
				}

				*addr = (int)(text + 3);
				*++text = JMP;
				addr = ++text;
				expression(Cond);
				*addr = (int)(text + 1);
			}
			else if(token == Lor)
			{
				//逻辑或
				match(Lor);
				*++text = JNZ;
				addr = ++text;
				expression(Lan);
				*addr = (int)(text + 1);
				expr_type = INT;
			}
			else if(token == Lan)
			{
				//逻辑与
				match(Lan);
				*++text = JZ;
				addr = ++text;
				expression(Or);
				*addr = (int)(text + 1);
				expr_type = INT;
			}
			else if(token == Or)
			{
				//按位或
				match(Or);
				*++text = PUSH;
				expression(Xor);
				*++text = OR;
				expr_type = INT;
			}
			else if(token == Xor)
			{
				//按位异或
				match(Xor);
				*++text = PUSH;
				expression(And);
				*++text = XOR;
				expr_type = INT;
			}
			else if(token == And)
			{
				//按位与
				match(And);
				*++text = PUSH;
				expression(Eq);
				*++text = AND;
				expr_type = INT;
			}
			else if(token == Eq)
			{
			// ==
				match(Eq);
				*++text = PUSH;
				expression(Ne);
				*++text = EQ;
				expr_type = INT;
			}
			else if(token == Ne)
			{
				// !=
				match(Ne);
				*++text = PUSH;
				expression(Lt);
				*++text = NE;
				expr_type = INT;
			}
			else if(token == Lt)
			{
				//　小于
				match(Lt);
				*++text = PUSH;
				expression(Shl);
				*++text = LT;
				expr_type = INT;
			}
			else if(token == Gt)
			{
				// 大于
				match(Gt);
				*++text = PUSH;
				expression(Shl);
				*++text = GT;
				expr_type = INT;
			}
			else if(token == Le)
			{
				// <=
				match(Le);
				*++text = PUSH;
				expression(Shl);
				*++text = LE;
				expr_type = INT;
			}
			else if(token == Ge)
			{
				// >=
				match(Ge);
				*++text = PUSH;
				expression(Shl);
				*++text = GE;
				expr_type = INT;
			}
			else if(token == Shl)
			{
				// <<
				match(Shl);
				*++text = PUSH;
				expression(Add);
				*++text = SHL;
				expr_type = INT;
			}
			else if(token == Shr)
			{
				// >>
				match(Shr);
				*++text = PUSH;
				expression(Add);
				*++text = SHR;
				expr_type = INT;
			}
			else if(token == Add)
			{
				// 加法
				match(Add);
				*++text = PUSH;
				expression(Mul);

				expr_type = tmp;
				if(expr_type > PTR)
				{
					//指针类型
					
					*++text = PUSH;
					*++text = IMM;
					*++text = sizeof(int);
					*++text = MUL;
				}
				*++text = ADD;
			}
			else if(token == Sub)
			{
				//减法
				match(Sub);
				*++text = PUSH;
				expression(Mul);
				if(tmp > PTR && tmp == expr_type)
				{
					//指针减
					*++text = SUB;
					*++text = PUSH;
					*++text = IMM;
					*++text = sizeof(int);
					*++text = DIV;
					expr_type = INT;
				}else if(tmp > PTR){
					//指针移动
					*++text = PUSH;
					*++text = IMM;
					*++text = sizeof(int);
					*++text = MUL;
					*++text = SUB;
					expr_type = tmp;
				}else {
					//数值减
					*++text = SUB;
					expr_type = tmp;
				}
			}
			else if(token == Mul)
			{
				match(Mul);
				*++text = PUSH;
				expression(Inc);
				*++text = MUL;
				expr_type = tmp;
			}
			else if(token == Div)
			{
				match(Div);
				*++text = PUSH;
				expression(Inc);
				*++text = DIV;
				expr_type = tmp;
			}
			else if(token == Mod)
			{
				//取模
				match(Mod);
				*++text = PUSH;
				expression(Inc);
				*++text = MOD;
				expr_type = tmp;
			}
			else if(token == Inc || token == Dec)
			{
				//后缀自减和自加
				if(*text == LI)
				{
					*text = PUSH;
					*++text = LI;
				}else if(*text == LC){
					*text = PUSH;
					*++text = LC;
				}else {
					printf("%d: bad value in increment.\n", line);
					exit(-1);
				}

				*++text = PUSH;
				*++text = IMM;
				*++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
				*++text = (token == Inc) ? ADD : SUB;
				*++text = (expr_type == CHAR) ? SC : SI;
				*++text = PUSH;
				*++text = IMM;
				*++text = (expr_type > PTR) ? sizeof(int) : sizeof(char);
				*++text = (token == Inc) ? SUB : ADD;
				match(token);
			}
			else if(token == Brak)
			{
				// 数组　[]
				match(Brak);
				*++text = PUSH;
				expression(Assign);
				match(']');

				if(tmp > PTR)
				{
					//指针，没有 char*
					*++text = PUSH;
					*++text = IMM;
					*++text = sizeof(int);
					*++text = MUL;
				}
				else if(tmp < PTR)
				{
					printf("%d: pointer type expected.\n", line);
					exit(-1);
				}
				expr_type = tmp - PTR;
				*++text = ADD;
				*++text = (expr_type == CHAR) ? LC : LI; 
			}
			else
			{
				printf("%d: complier error, token = %d\n", line, token);
				exit(-1);
			}
		}
	}
}

