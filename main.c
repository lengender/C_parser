#include"parser.h"

int debug; //打印调试信息
int assembly;
int *idmain;  //the 'main' function

int main(int argc, char **argv)
{
	int i, fd;
	int *tmp;

	argc--;
	argv++;
    
	//解析参数
	if(argc > 0 && **argv == '-' && (*argv)[1] == 's')
	{
		assembly = 1;
		--argc;
		++argv;
	}
	if(argc > 0 && **argv == '-' && (*argv)[1] == 'd')
	{
	  
	   debug = 1;
		--argc;
		++argv;
	}

	if(argc < 1)
	{
		printf("usage: xc [-s] [-d] file ...\n");
	}
	poolsize = 1024 * 1024;  // default
	line = 1;

	if((fd = open(*argv, 0)) < 0)
	{
		printf("can not open(%s)\n", *argv);
		return -1;
	}


	
    
    //给虚拟机分配内存
	if(!(text = malloc(poolsize)))
	{
		printf("can not malloc (%d) for text area.\n", poolsize);
		return -1;
	}

	if(!(data = malloc(poolsize)))
	{
		printf("can not malloc (%d) for data area.\n", poolsize);
		return -1;
	}

	if(!(stack = malloc(poolsize)))
	{
		printf("could not malloc (%d) for stack area.\n", poolsize);
		return -1;
	}
    
	if(!(symbols = malloc(poolsize)))
	{
		printf("can not malloc (%d) for symbol table.\n", poolsize);
	}
	memset(text, 0, poolsize);
	memset(data, 0, poolsize);
	memset(stack, 0, poolsize);
	memset(symbols, 0, poolsize);

	old_text = text;

	src = "char else enum if int return sizeof while "
		  "open read close printf malloc memset memcmp exit void main";
  
	//将关键字加入到符号表
	i = Char;
	while(i <= While)
	{
		next();
		current_id[Token] = i++;
	}

	//将库函数加入到符号表
    i = OPEN;
	while(i <= EXIT)
	{
		next();
		current_id[Class] = Sys;
		current_id[Type] = INT;
		current_id[Value] = i++;
	}

	next(); current_id[Token] = Char;  // 将void 加入符号表
	next(); idmain = current_id;    // keep track of main

    
	if(!(src = malloc(poolsize)))
    {
         printf("can not malloc (%d) for source area\n", poolsize);
         return -1;
	}
	old_src = src;
	//读入源文件
	if((i = read(fd, src, poolsize - 1)) <= 0)
	{
		printf("read() returned %d\n", i);
		return -1;
	}
     
	src[i] = 0;
	close(fd);  

    //printf("%s\n", src);


    program();
    if(!(PC = (int*)idmain[Value]))
	{
		printf("main() not defined \n");
		return -1;
	}

	if(assembly)
		return 0;

	SP = (int*)((int)stack + poolsize);
	*--SP = EXIT;
	*--SP = PUSH;  tmp = SP;
	*--SP = argc;
	*--SP = (int)argv;
	*--SP = (int)tmp;
    
	return eval();
}
