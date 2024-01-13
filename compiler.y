%{
    #include <stdio.h>
    #include <string.h>
    #include <math.h>
    #include <stdlib.h>
    
    #include "config.h"
    #include "utils.h"

    extern FILE* yyin;
    extern char* yytext;
    extern int yylineno; 

%}

%union 
{
    int intval;
    char* strval;
    
}

%start s

%token INT C_FLOAT ARRAY CLASS VOID FUNCTION FLOAT STRING WHILE TRUE FALSE BOOL IF ELSE BGN END CONST FOR RET TYPEOF EVAL CHAR MCALL
<strval>ID EQ NEQ LEQ GEQ OR AND LT GT C_CHAR C_STRING 
<intval>C_INT 


%left OR
%left AND 
%left EQ NEQ
%left LT GT LEQ GEQ
%left '+' '-' 
%left '*' '/'
%left '(' ')'

%%

s : declaratii main { 
   if(!error_code)
   {
      create_symbol_table();
      create_function_table();
      printf("cod sintactic corect! ;) \n");  if(final_result){printf("\033[32m%s\033[0m\n", final_result); free(final_result);}
   }
}
  ;

declaratii : declaratii declarare
           | declarare
           | 
           ;

declarare : var_decl 
          | func_decl 
          | class_decl
          ;

type : is_const primitive_type
     ;

primitive_type : INT {
                  if(isFunction) 
                    currentFunction.returnType = 0;
                  else currentVariable.typeInfo.typeName = 0;
               }
               | BOOL {
                  if(isFunction) 
                    currentFunction.returnType = 1;
                  else currentVariable.typeInfo.typeName = 1;
               }
               | FLOAT {
                  if(isFunction) 
                    currentFunction.returnType = 2;
                  else currentVariable.typeInfo.typeName = 2;
               }
               | CHAR {
                  if(isFunction) 
                    currentFunction.returnType = 3;
                  else currentVariable.typeInfo.typeName = 3;
               }
               | STRING {
                  if(isFunction) 
                    currentFunction.returnType = 4;
                  else currentVariable.typeInfo.typeName = 4;
               }
               ;

return_type : primitive_type  
            | VOID {
              currentFunction.returnType = 5;
            }
            ;

func_decl : FUNCTION {
            isFunction = 1;
          } 
          return_type ID 
          {
            strncpy(currentFunction.scope, currentScope, MAX_SCOPE_LEN);
            strncpy(currentFunction.name, $4, MAX_VAR_LEN);
            add_scope($4, 0); 
            init_varList(&currentFunction.parameters); 
            currentFunction.line = yylineno;
            isFunction = 0;
          } 
          '(' param_list ')' 
          {
            int return_value;
            if(!isClass) {
               return_value = check_func_already(&allFunctions, currentFunction.name, &currentFunction.parameters, yylineno);
            }
            else {
               return_value = check_func_already(&classFunctions, currentFunction.name, &currentFunction.parameters, yylineno);
            }
            if(!return_value) {
               if(!isClass) {
                  insert_func(&allFunctions, &currentFunction);
               }
               else {
                  insert_func(&classFunctions, &currentFunction);
               }
            }
            clear_varList(&currentFunction.parameters);
          }
          '{' scope_body '}' 
          {
            remove_from_scope(); 
          }
          ;
          

param_list : param {
              insert_var(&currentFunction.parameters, &currentVariable);
           }
           | param {
              insert_var(&currentFunction.parameters, &currentVariable);
           } ',' param_list
           |
           ;

param : type ID {
        currentVariable.value[0] = 0; 
        // initialized with 0
        currentVariable.typeInfo.isArray = 0;
        currentVariable.typeInfo.arrayLen = 1;
        currentVariable.line = yylineno;
        strncpy(currentVariable.name, $2, MAX_VAR_LEN);
        strncpy(currentVariable.scope, currentScope, MAX_SCOPE_LEN);
      }
      | ARRAY type '[' C_INT ']' ID {
        currentVariable.typeInfo.isArray = 1;
        if($4 > MAX_ARRAY_LEN) {
          printf("\033[31mArray elements limit exceeded on line %d\n\033[0m", yylineno);
          error_code = 1;
        }
        
        currentVariable.typeInfo.arrayLen = min($4, MAX_ARRAY_LEN);
        for(int i = 0; i < currentVariable.typeInfo.arrayLen; ++i) {
          currentVariable.value[i] = 0;
          // whole array initialized with 0
        }

        currentVariable.line = yylineno;
        strncpy(currentVariable.name, $6, MAX_VAR_LEN);
        strncpy(currentVariable.scope, currentScope, MAX_SCOPE_LEN);
      }
     ;

is_const : CONST {
            currentVariable.typeInfo.isConst = 1;
         }
         | {
            currentVariable.typeInfo.isConst = 0;
         }
         ;


class_decl : CLASS ID {
               isClass = 1; 
               add_scope($2, 1);
               if(classNr >= MAX_FUNC_NUM) {
                  printf("The maximum number of classes has been reached\n");
               }
               else {
                  strncpy(classList[classNr++], $2, MAX_VAR_NUM);
               }
            } 
            '{' class_content '}' 
            {
               isClass = 0;
               remove_from_scope();
            }
           ;
class_content : var_decl class_content
              | func_decl  class_content
              |
              ;

var_decl : type 
         {
            currentVariable.typeInfo.isArray = 0;
            currentVariable.typeInfo.arrayLen = 1;
            currentVariable.value[0] = 0;
         } var_list ';'
         | ARRAY type '[' C_INT ']' 
         {
            currentVariable.typeInfo.isArray = 1;
            currentVariable.typeInfo.arrayLen = $4;
            for(int i = 0; i < $4; ++i) {
               currentVariable.value[i] = 0;
            }
         } array_list ';'
         ;

array_list : array_var 
           | array_var ',' array_list
           ;

array_var : ID {
              currentVariable.line = yylineno;
              strncpy(currentVariable.name, $1, MAX_VAR_LEN);
              strncpy(currentVariable.scope, currentScope, MAX_SCOPE_LEN);
              int returnVal = check_variable_already(&allVariables, currentVariable.name, currentScope, yylineno);
              if(!returnVal) {
                  if(!isClass) {
                     insert_var(&allVariables, &currentVariable);
                  }
                  else {
                     insert_var(&classVariables, &currentVariable);
                  }
              }
          }
          | ID {
              currentVariable.line = yylineno;
              strncpy(currentVariable.name, $1, MAX_VAR_LEN);
              strncpy(currentVariable.scope, currentScope, MAX_SCOPE_LEN);
              arrayInitPos = 0;
          } '=' '{' array_content '}' {
               int returnVal = check_variable_already(&allVariables, currentVariable.name, currentScope, yylineno);
               if(!returnVal) {
                  if(!isClass) {
                     insert_var(&allVariables, &currentVariable);
                  }
                  else {
                     insert_var(&classVariables, &currentVariable);
                  }
              }
          }
          ;
array_content : atomic_value {
                
              }
              | atomic_value {             
                
              } ',' array_content
              ;

var_list : variable 
         | variable ',' var_list
         ;

variable : ID {
            currentVariable.line = yylineno;
            strncpy(currentVariable.name, $1, MAX_VAR_LEN);
            strncpy(currentVariable.scope, currentScope, MAX_SCOPE_LEN);
            int returnVal = check_variable_already(&allVariables, currentVariable.name, currentScope, yylineno);
            if(!returnVal) {
               if(!isClass) {
                  insert_var(&allVariables, &currentVariable);
               }
               else {
                  insert_var(&classVariables, &currentVariable);
               }
            }
         }
         | ID 
         {
            currentVariable.line = yylineno;
            strncpy(currentVariable.name, $1, MAX_VAR_LEN);
            strncpy(currentVariable.scope, currentScope, MAX_SCOPE_LEN);
            int returnVal = check_variable_already(&allVariables, currentVariable.name, currentScope, yylineno);
            if(!returnVal) {
               if(!isClass) {
                   insert_var(&allVariables, &currentVariable);
               }
               else {
                  insert_var(&classVariables, &currentVariable);
               }
            }
          } 
          '=' expression_value 
          {
            
          }
         ;

atomic_value : C_INT {
                
             }
             | C_FLOAT
             {
                
             }
             | C_CHAR
             {
                
             }
             | C_STRING
             {
               
             }
             | FALSE
             {
                
             } 
             | TRUE
             {
                
             }
             | ID 
             {
                
             }
             | function_call
             {
                
             }
             | class_method_call {
                 
               }
             | ID '.' ID 
             {
               
             }
             | ID '[' C_INT ']'
             {
               
             }
             ;

expression_value : 
                 atomic_value {
                    
                 }
                 | 
                 expression_value '+' expression_value {
                    
                 }  
                 |
                 expression_value '-' expression_value {
                    
                 }  
                 |
                 expression_value '*' expression_value {
                   
                 }  
                 |
                 expression_value '/' expression_value {
                    
                 }  
                 |
                 expression_value EQ expression_value {
                    
                 }  
                 |
                 expression_value NEQ expression_value {
                    
                 }  
                 |
                 expression_value LEQ expression_value {
                    
                 }  
                 |
                 expression_value GEQ expression_value {
                    
                 }  
                 |
                 expression_value OR expression_value {
                    
                 }  
                 |
                 expression_value AND expression_value {
                    
                 }  
                 |
                 expression_value LT expression_value {
                    
                 }  
                 |
                 expression_value GT expression_value {
                    
                 }
                 | '(' expression_value ')' {
                    
                 }
                 ;

var_assignment : ID '=' expression_value 
               {
                  
               }
               ;

class_assignment : ID '.' ID '=' expression_value 
                 {
                    
                 }
                 ;

array_assignment : ID '[' C_INT ']' '=' expression_value
                 {
                    
                 }
                 ;

object_init : '&' ID ID {
   if(!checkClass($2)) {
      error_code = 1;
      printf("The class %s used to initialize object %s on line %d has not been defined\n", $2, $3, yylineno);
   }
   else {
      add_scope($3, 1);
      from_class_to_all($2, currentScope);
      remove_from_scope();
   }
}  

main : BGN { } scope_body END { }
     ;

scope_body : var_decl scope_body
           | function_call ';' scope_body
           | class_method_call ';' scope_body
           | typeof_call ';' scope_body
           | eval_call ';' scope_body 
           | var_assignment ';' scope_body
           | object_init ';' scope_body
           | class_assignment ';' scope_body
           | array_assignment ';' scope_body
           | return_statement ';'
           | repetitive_loop scope_body
           | if_statement scope_body
           | 
           ;

func_arguments : expression_value {
                  
               }
               | expression_value {
                  
               } ',' func_arguments
               |
 
function_call : ID {
                  
              }
              '(' func_arguments ')' 
              {
                  
              }
              ;

class_method_call : ID MCALL {
                     
                  } 
                  function_call 
                  {
                     
                  }

typeof_call : TYPEOF '(' expression_value ')' {
   
}

eval_call : EVAL '(' expression_value ')' {
   
}

repetitive_loop : for_loop 
                | while_loop
                ; 

for_loop : FOR 
         {
            forCounter++; 
            char newScope[MAX_VAR_LEN];
            snprintf(newScope, MAX_VAR_LEN, "for_%d", forCounter);
            add_scope(newScope, 0);
         } 
         '(' for_init ';' for_condition ';' for_step ')' '{' scope_body '}' 
         {
            remove_from_scope();
         }
         ;

for_init : primitive_type var_list
         | expression_value 
         | var_assignment 
         |
         ;

for_condition : expression_value
              | 
              ;

for_step : var_assignment
         | 
         ;

while_loop : WHILE 
            {
              whileCounter++; 
              char newScope[MAX_VAR_LEN];
              snprintf(newScope, MAX_VAR_LEN, "while_%d", whileCounter);
              add_scope(newScope, 0);
            } 
            '(' expression_value ')' '{' scope_body '}' 
            {
              remove_from_scope();
            }
           ;

if_statement : IF 
             {
                ifCounter++; 
                char newScope[MAX_VAR_LEN];
                snprintf(newScope, MAX_VAR_LEN, "if_%d", ifCounter);
                add_scope(newScope, 0);
             } 
             '(' expression_value ')' '{' scope_body '}' 
             {
              remove_from_scope();
             } 
             else_statement
             ;

else_statement : ELSE 
               {
                  elseCounter++; 
                  char newScope[MAX_VAR_LEN];
                  snprintf(newScope, MAX_VAR_LEN, "else_%d", elseCounter);
                  add_scope(newScope, 0);
               } 
               '{' scope_body '}' 
               {
                  remove_from_scope();
               }
               | 
               ;


return_statement : RET expression_value

%%

void yyerror(char *s)
{
  printf("eroare: %s la linia:%d\n", s, yylineno);
  error_code = 1;
}

int main(int argc, char** argv)
{
  yyin = fopen(argv[1], "r");

  init_varList(&allVariables);
  init_varList(&classVariables);
  init_funcList(&allFunctions);
  init_funcList(&classFunctions);
  // information about every assignation (varName, scope and AstTree coresponding to some Id)

  strncpy(currentScope, "/", MAX_SCOPE_LEN);

  yyparse();


  clear_varList(&allVariables);
  clear_varList(&classVariables);
  clear_funcList(&allFunctions);
  clear_funcList(&classFunctions);
} 
