%{
    #include <stdio.h>
    #include "compiler.tab.h"
%}
%option noyywrap
%%

"const" {return CONST;}
"array" {return ARRAY;}
"integer" {return INT;}
"float" {return FLOAT;}
"boolean" {return BOOL;}
"string" {return STRING;}
"char" {return CHAR;}
"true" {return TRUE;}
"false" {return FALSE;}
"for" {return FOR;}
"while" {return WHILE;}
"if" {return IF;}
"else" {return ELSE;}
"function" {return FUNCTION;}
"class" {return CLASS;}
"begin_progr" {return BGN;}
"end_progr" {return END;}
"void" {return VOID;}
"return" {return RET;}

"==" {
    yylval.strval = strdup(yytext); 
    return EQ;
}
"->" {
    return MCALL;
}
"!=" {
    yylval.strval = strdup(yytext);
    return NEQ;
}
"<=" {
    yylval.strval = strdup(yytext);
    return LEQ;
}
">=" {
    yylval.strval = strdup(yytext);
    return GEQ;
}
"<" {
    yylval.strval = strdup(yytext);
    return LT;
}
">" {
    yylval.strval = strdup(yytext);
    return GT;
}
"||" {
    yylval.strval = strdup(yytext);
    return OR;
}
"&&" {
    yylval.strval = strdup(yytext);
    return AND;
}
"Eval" {return EVAL;}
"TypeOf" {return TYPEOF;}

[a-zA-Z][a-zA-Z0-9]* {
    yylval.strval = strdup(yytext); 
    return ID;
}

-?(0|[1-9][0-9]*) {
    yylval.intval = atoi(yytext); 
    return C_INT; 
}

-?((0|[1-9][0-9]*)\.[0-9]+) {return C_FLOAT;}

\"[?!,.;" "a-zA-Z0-9]*\" {
    yylval.strval = strdup(yytext); 
    return C_STRING;
}
\'[?!,.;" "a-zA-Z0-9]\' {
    yylval.strval = strdup(yytext); 
    return C_CHAR;
}
[ \t] ;
\n {yylineno++;}
\r ;
. {return yytext[0];}
