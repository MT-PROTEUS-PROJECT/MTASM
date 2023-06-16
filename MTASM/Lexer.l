%{

#include "Parser.tab.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define fileno _fileno

%}

%option noyywrap

%%

[0-9]+ {
	int intval = atoi(yytext);
	yylval.val = intval;
	return ELEM;
}

. { }

%%
