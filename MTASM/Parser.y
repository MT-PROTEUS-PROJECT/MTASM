%{

#include <stdio.h>

	FILE* yyin;
	int yyerror();

	extern int yylex();

%}

%union symval
{
	int val;
};

%token <val> ELEM
%type <val> LIST

%%

LIST: ELEM
{
	int test = $$;
	$$ = $1;
	int val = $1;
	printf("%d\n", val);
}
| LIST ELEM{
	int test = $$;
	$$ = $1;
	int val1 = $1;
	int val2 = $2;
	printf("%d\n", val2);
}

%%

int parser_main(int argc, char* argv[])
{
	FILE* fp = NULL;
	if (argc == 2)
	{
		fopen_s(&fp, argv[1], "rb");

		if (fp == NULL)
		{
			perror("Failed to open file.");
			return -1;
		}
		else
		{
			yyin = fp;
		}
	}

	yyparse();

	if (fp != NULL)
	{
		fclose(fp);
	}

	return 0;
}

int yyerror(const char* p)
{

	printf("%s\n", p);
	return 0;
}
