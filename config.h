#ifndef config
#define config

#define MAX_VAR_LEN 32
#define MAX_SCOPE_LEN 1024
#define MAX_ARRAY_LEN 128
#define MAX_VAR_NUM 128
#define MAX_FUNC_NUM 128
#define MAX_EXP_LEN 128 
#define MAX_ARGS_NR 128

//const char *symb_table_path = "symbol_tables/symbol_table.txt";
//const char *func_table_path = "symbol_tables/symbol_table_functions.txt";
const char *symb_tables_path = "symbol_tables/symbol_tables.txt";
char decodeType[6][20] = {"int", "boolean", "real", "char", "string", "void"};
char currentScope[MAX_SCOPE_LEN];

int arrayInitPos = 0;

int inObj = 0;
short isClass;

char objName[MAX_VAR_LEN];

int nrArgs;
int funcArgTypes[MAX_ARGS_NR];

char classList[MAX_FUNC_NUM][MAX_VAR_LEN];
int classNr = 0;

short astTypes[MAX_EXP_LEN];
int astTypesLen = 0;

short nodeType, dataType, isFunction;
char currentValue[MAX_VAR_LEN], currentOperation[MAX_VAR_LEN];
int forCounter = 0, whileCounter = 0, ifCounter = 0, elseCounter = 0;

int stackCount = 0;
int myStack[MAX_ARGS_NR];

typedef struct {
    short typeName;
    short isConst;
    short isArray;
    short arrayLen;
    /*
        0 - int, 1 - boolean, 2 - real, 3 - char, 4 - string
        isConst = 0 if variabile is not constant, 1 otherwise
        isArray = 0 if variable is not an array, 1 otherwise 
        if isArray = 0 -> arrayLen will always be 1 (convention)
    */
} 
Type;

typedef struct {
    Type typeInfo;
    char name[MAX_VAR_LEN];
    char scope[MAX_SCOPE_LEN];
    int value[MAX_ARRAY_LEN];
    int line;
    /* 
        if isArray = 1 -> there will be arrayLen values (an int value for each element of the array)
        else, only value[0] matters 
        if the variable is not initialized, it gets a 0 default value :)
        scope will be of the following form: 
        only a / means global  
        /~/ means inside main,
        '/for/' or '/while' means inside a for/while   
        /funcName/ - means inside the funcName function
        /&objName/ - means a class  
        /&objName/method/ - means inside a method of an object
        Example of scopes : '/~/for/while/for/' OR '/~/printAns/' OR /&myClass/ OR /~/&myClass/printAns/ 
    */
}
Variable;

typedef struct {
    Variable *variables;
    int varNumber;
}
VariableList;

typedef struct {
    VariableList parameters;
    char name[MAX_VAR_LEN];
    char scope[MAX_SCOPE_LEN];
    short returnType;
    int line;
} 
Function;

typedef struct {
    Function *functions;
    int funcNumber;
} 
FunctionList;

struct NodeInfo{
    short nodeType;
    short dataType;
    char value[MAX_VAR_LEN]; 
};

struct AstNode{
    struct NodeInfo nodeInfo;
    struct AstNode* left;
    struct AstNode* right;
};

VariableList allVariables, classVariables;
FunctionList allFunctions, classFunctions;

Variable currentVariable;
Function currentFunction;

#endif 