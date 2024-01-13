#ifndef utils
#define utils

char *concatenate_and_free(char *s1, char *s2) {
    char *result;
    if(s1 == NULL)
        return s2;
    int len1 = strlen(s1);
    int len2 = strlen(s2);

    result = (char *)malloc((len1 + len2 + 1) * sizeof(char));
    if (result == NULL) {
        printf("Error allocating memory!\n");
        return NULL;
    }

    strcpy(result, s1);
    strcat(result, s2);

    free(s1);
    free(s2);

    return result;
}
short error_code = 0;
char * final_result = NULL;

struct AstNode *init_Ast(short _nodeType, short _dataType, char *_value) 
{
    struct AstNode *Ast = (struct AstNode *) malloc(sizeof(struct AstNode));
    Ast->left = Ast->right = NULL;
    Ast->nodeInfo.nodeType = _nodeType;
    Ast->nodeInfo.dataType = _dataType;
    strncpy(Ast->nodeInfo.value, _value, MAX_VAR_LEN);
    return Ast;
}
struct AstNode *build_Ast(char *_value, struct AstNode *_left, struct AstNode *_right, int _nodeType) 
{
    struct AstNode *Ast = (struct AstNode *) malloc(sizeof(struct AstNode));
    Ast->left = _left;
    Ast->right = _right;
    Ast->nodeInfo.nodeType = _nodeType;
    Ast->nodeInfo.dataType = 6;
    strncpy(Ast->nodeInfo.value, _value, MAX_VAR_LEN); 
    return Ast; 
}

void init_varList(VariableList *varTable) 
{
    varTable->variables = (Variable *) malloc(MAX_VAR_NUM * sizeof(Variable));
    varTable->varNumber = 0;
}

void init_funcList(FunctionList *funcs) 
{
    funcs->functions = (Function *) malloc(MAX_FUNC_NUM * sizeof(Function));
    funcs->funcNumber = 0;
} 
void insert_var(VariableList *varTable, Variable *newVar)
{
    int varNum = varTable->varNumber;

    if(varNum >= MAX_VAR_NUM) {
        printf("Variable number limit is excedeed!");
        error_code = 1;
        return;
    }
    
    strncpy(varTable->variables[varNum].name, newVar->name, MAX_VAR_LEN);
    strncpy(varTable->variables[varNum].scope, newVar->scope, MAX_SCOPE_LEN);
    
    varTable->variables[varNum].line = newVar->line;
    varTable->variables[varNum].typeInfo.typeName = newVar->typeInfo.typeName;
    varTable->variables[varNum].typeInfo.isConst = newVar->typeInfo.isConst;
    varTable->variables[varNum].typeInfo.isArray = newVar->typeInfo.isArray;

    int arrayLen = newVar->typeInfo.arrayLen;

    if(arrayLen >= MAX_ARRAY_LEN) {
        printf("\033[31mArray length limit is excedeed at line %d!\n\033[0m", newVar->line);
        error_code = 1;
        return;
    }

    for(int i = 0; i < arrayLen; ++i) {
        varTable->variables[varNum].value[i] = newVar->value[i];
    }

    varTable->variables[varNum].typeInfo.arrayLen = arrayLen;      
    varNum++;
    varTable->varNumber = varNum;
}

void insert_func(FunctionList *funcTable, Function *newFunc)
{
    int funcNum = funcTable->funcNumber;

    if(funcNum >= MAX_FUNC_NUM) {
        printf("\033[31mFunction number limit is excedeed!\033[0m");
        error_code = 1;
        exit(1);
    }
    
    strncpy(funcTable->functions[funcNum].name, newFunc->name, MAX_VAR_LEN);
    strncpy(funcTable->functions[funcNum].scope, newFunc->scope, MAX_SCOPE_LEN);

    funcTable->functions[funcNum].line = newFunc->line;
    funcTable->functions[funcNum].returnType = newFunc->returnType;

    init_varList(&funcTable->functions[funcNum].parameters);

    for(int i = 0; i < newFunc->parameters.varNumber; ++i) {
        insert_var(&funcTable->functions[funcNum].parameters, &newFunc->parameters.variables[i]);
    }

    funcNum++;
    funcTable->funcNumber = funcNum;
}

void clear_varList(VariableList *varTable) 
{
    free(varTable->variables);
    varTable->varNumber = 0;
}

void clear_funcList(FunctionList *funcs) 
{
    free(funcs->functions);
    funcs->funcNumber = 0;
} 

void add_scope(char *add, short isClass) 
{
    char addPath[MAX_SCOPE_LEN];
    if(!isClass) {
        snprintf(addPath, MAX_SCOPE_LEN, "%s/", add);
    }
    else {
        snprintf(addPath, MAX_SCOPE_LEN, "&%s/", add);
    }
    strcat(currentScope, addPath);
}

void remove_from_scope() 
{
    int scopeLen = strlen(currentScope) - 1;
    currentScope[scopeLen--] = '\0';
    while(currentScope[scopeLen] != '/') {
        currentScope[scopeLen--] = '\0';
    }
}
    
void create_symbol_table() 
{
    if(error_code) return;
    FILE *F = fopen(symb_tables_path, "w");
    if(!F) {
        fprintf(stderr, "Error at opening symbol_table_path!");
        exit(2);
    }
    fprintf(F, "The following variables where declared in the program:\n");
    for(int i = 0; i < allVariables.varNumber; ++i) {
        fprintf(F, "%d. Name: %s, ", i + 1, allVariables.variables[i].name);
        
        if(allVariables.variables[i].typeInfo.isConst) {
            fprintf(F, "State: Constant, ");
        }
        else {
            fprintf(F, "State: Mutable, ");
        }

        fprintf(F, "Structure: ");
        if(allVariables.variables[i].typeInfo.isArray) {
            fprintf(F, "Array of %d elements, ", allVariables.variables[i].typeInfo.arrayLen);
        }
        else {
            fprintf(F, "Simple, ");
        }

        fprintf(F, "Type: %s, ", decodeType[allVariables.variables[i].typeInfo.typeName]);
        fprintf(F, "Line: %d, ", allVariables.variables[i].line);
        if(allVariables.variables[i].typeInfo.isArray || (strcmp(allVariables.variables[i].scope, "/") && strcmp(allVariables.variables[i].scope, "/~/") || 
            allVariables.variables[i].typeInfo.typeName != 0)) {
            // if it's an array or not global / declared in main don't show the value or if its not int 
            fprintf(F, "Scope: %s\n", allVariables.variables[i].scope);
        }   
        else {
            fprintf(F, "Scope: %s, ", allVariables.variables[i].scope);
            if(!allVariables.variables[i].typeInfo.isArray) {
                fprintf(F, "Value: %d\n", allVariables.variables[i].value[0]);
            } 
        }  
    }
    fprintf(F,"\n");

    fprintf(F, "The following functions where defined in the program:\n");
    for(int i = 0; i < allFunctions.funcNumber; ++i) {
        fprintf(F, "%d) Name: %s, ", i + 1, allFunctions.functions[i].name);
        
        fprintf(F, "Return Type: %s, ", decodeType[allFunctions.functions[i].returnType]);
        fprintf(F, "Scope: %s, ", allFunctions.functions[i].scope);
        fprintf(F, "Line: %d, ", allFunctions.functions[i].line);

        fprintf(F, "Parameters: \n");
        
        for(int j = 0; j < allFunctions.functions[i].parameters.varNumber; ++j) {
            Variable *curVar = &(allFunctions.functions[i].parameters.variables[j]);
            fprintf(F, "    %d.%d) Name: %s, ", i + 1, j + 1, curVar->name);
            if(curVar->typeInfo.isConst) {
                fprintf(F, "State: Constant, ");
            }
            else {
                fprintf(F, "State: Mutable, ");
            }
            fprintf(F, "Structure: ");
            if(curVar->typeInfo.isArray) {
                fprintf(F, "Array of %d elements, ", curVar->typeInfo.arrayLen);
            }
            else {
                fprintf(F, "Simple, ");
            }
            fprintf(F, "Type: %s\n", decodeType[curVar->typeInfo.typeName]);
        }
    }

}

void create_function_table() 
{
    if(error_code) return;
    FILE *F = fopen(symb_tables_path, "w");
    if(!F) {
        fprintf(stderr, "Error at opening func_table_path!");
        exit(2);
    }

    fprintf(F, "The following functions where defined in the program:\n");
    for(int i = 0; i < allFunctions.funcNumber; ++i) {
        fprintf(F, "%d) Name: %s, ", i + 1, allFunctions.functions[i].name);
        
        fprintf(F, "Return Type: %s, ", decodeType[allFunctions.functions[i].returnType]);
        fprintf(F, "Scope: %s, ", allFunctions.functions[i].scope);
        fprintf(F, "Line: %d, ", allFunctions.functions[i].line);

        fprintf(F, "Parameters: \n");
        
        for(int j = 0; j < allFunctions.functions[i].parameters.varNumber; ++j) {
            Variable *curVar = &(allFunctions.functions[i].parameters.variables[j]);
            fprintf(F, "    %d.%d) Name: %s, ", i + 1, j + 1, curVar->name);
            if(curVar->typeInfo.isConst) {
                fprintf(F, "State: Constant, ");
            }
            else {
                fprintf(F, "State: Mutable, ");
            }
            fprintf(F, "Structure: ");
            if(curVar->typeInfo.isArray) {
                fprintf(F, "Array of %d elements, ", curVar->typeInfo.arrayLen);
            }
            else {
                fprintf(F, "Simple, ");
            }
            fprintf(F, "Type: %s\n", decodeType[curVar->typeInfo.typeName]);
        }
    }
}

short check_func_already(FunctionList *funcTable, char *funcName, VariableList *parameters, int line) {
    int funcNum = funcTable->funcNumber;
    for(int i = 0; i < funcNum; ++i) {
        if(!strncmp(funcTable->functions[i].name, funcName, MAX_VAR_LEN)) {
            // verificam si parametrii
            short sameSignature = 1;
            VariableList *old_params = &funcTable->functions[i].parameters;
            if(parameters->varNumber != old_params->varNumber) {
                sameSignature = 0;
            }
            else {
                short same_params = 1;
                for(int j = 0; j < old_params->varNumber; ++j) {
                    if(parameters->variables[j].typeInfo.typeName != old_params->variables[j].typeInfo.typeName) {
                        same_params = 0;
                        break;
                    }
                }
                if(!same_params)
                    sameSignature = 0;
            }
            if(sameSignature) {
                printf("\033[31mFunction %s redefined on line %d. There is another function on line %d.\n\033[0m", funcName, line, funcTable->functions[i].line);
                error_code = 1;
                return -1;
            }
        }
    }
    return 0;
}

short check_variable_already(VariableList *varTable, char *name, char *scope, int line) 
{
    int varNum = varTable->varNumber;
    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(name, varTable->variables[i].name) && !strcmp(scope, varTable->variables[i].scope)) {
            printf("\033[31mVariable %s is defined multiple times (line %d and line %d) in the same scope\n\033[0m",
                name, varTable->variables[i].line, line
            );
            error_code = 1;
            return -1;
        }
    }
    return 0;
}

short check_var_defined(VariableList *varTable, char *name, char *scope, int line) 
{
    int varNum = varTable->varNumber;
    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(name, varTable->variables[i].name) && strstr(scope, varTable->variables[i].scope)) {
            return 0;
        }
    }
    printf("\033[31mVariable %s on line %d was not previously defined!\n\033[0m", name, line);
    error_code = 1;
    return -1;
}

short extract_variable_type(VariableList *varTable, char *name, char *scope) 
{
    int varNum = varTable->varNumber, deepestScope = 0, pos = -1;
    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(name, varTable->variables[i].name) && strstr(scope, varTable->variables[i].scope)) {
            int scopeLen = strlen(varTable->variables[i].scope);
            if(deepestScope < scopeLen) {
                deepestScope = scopeLen;
                pos = i;
            }
        }
    }
    if(pos == -1) {
        return -1;
    }
    return varTable->variables[pos].typeInfo.typeName;
}

short endWith(char *str, char *pat) 
{
    int n = strlen(str);
    int m = strlen(pat);
    if(m > n) {
        return 0;
    }
    return (strcmp(str + n - m, pat) == 0);
}

short check_class_var(VariableList *varTable, char *varName, char *objName, int line) 
{
    // variable scope should end with &objName/
    char pattern[MAX_VAR_LEN];
    snprintf(pattern, MAX_VAR_LEN, "&%s/", objName);
    int varNum = varTable->varNumber;

    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(varName, varTable->variables[i].name) && endWith(varTable->variables[i].scope, pattern)) {
            return 0;          
        }
    }

    printf("\033[31mThe %s.%s field used on line %d wasn't previously defined!\n\033[0m", objName, varName, line);
    error_code = 1;
    return -1;
}

short extract_class_varType(VariableList *varTable, char *varName, char *objName) 
{
    char pattern[MAX_VAR_LEN];
    snprintf(pattern, MAX_VAR_LEN, "&%s/", objName);
    int varNum = varTable->varNumber;

    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(varName, varTable->variables[i].name) && endWith(varTable->variables[i].scope, pattern)) {
            return varTable->variables[i].typeInfo.typeName;          
        }
    }

    return -1;
}

short check_array_defined(VariableList *varTable, char *name, char *scope, int index, int line)
{
    int varNum = varTable->varNumber;
    int arrayLen = -1;

    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(name, varTable->variables[i].name) && strstr(scope, varTable->variables[i].scope)) {
            if(varTable->variables[i].typeInfo.isArray) {
                arrayLen = varTable->variables[i].typeInfo.arrayLen;
            }
            break;
        }
    }
    if(arrayLen == -1) {
        printf("\033[31mArray %s on line %d was not previously defined!\n\033[0m", name, line);
        error_code = 1;
        return -1;
    }
    if(index < 0 || index >= arrayLen) {
        printf("\033[31mIndex %d of array %s on line %d is out of bounds!\n\033[0m", index, name, line);
        error_code = 1;
        return -1;
    }
    return 0;
}

short check_func_defined(FunctionList *funcTable, char *name, int line) 
{
    // scope doesn't matter at simple functions
    int funcNum = funcTable->funcNumber;
    for(int i = 0; i < funcNum; ++i) {
        if(!strcmp(name, funcTable->functions[i].name)) {
            return 0;
        }
    }

    printf("\033[31mFunction %s called on line %d is not defined!\n\033[0m", name, line);
    error_code = 1;
    return -1;
}

short check_method_defined(FunctionList *funcTable, char *methodName, char *objName,  int line) 
{
    int funcNum = funcTable->funcNumber;
    char pattern[MAX_VAR_LEN];
    snprintf(pattern, MAX_VAR_LEN, "&%s/", objName);

    for(int i = 0; i < funcNum; ++i) {
        if(!strcmp(methodName, funcTable->functions[i].name) && endWith(funcTable->functions[i].scope, pattern)) {
            // we found the function name whose scope ends with &objName/. so -> methodName is indeed a method
            return 0;
        }
    }

    printf("\033[31mMethod %s->%s called on line %d is not defined!\n\033[0m", objName, methodName, line);
    error_code = 1;
    return -1;
}

short extract_func_return(FunctionList *funcTable, char *name) 
{
    int funcNum = funcTable->funcNumber;
    for(int i = 0; i < funcNum; ++i) {
        if(!strcmp(name, funcTable->functions[i].name)) {
            return funcTable->functions[i].returnType;
        }
    }
    return -1;
}

short extract_method_return(FunctionList *funcTable, char *methodName, char *objName)
{
    int funcNum = funcTable->funcNumber;
    char pattern[MAX_VAR_LEN];
    snprintf(pattern, MAX_VAR_LEN, "&%s/", objName);

    for(int i = 0; i < funcNum; ++i) {
        if(!strcmp(methodName, funcTable->functions[i].name) && endWith(funcTable->functions[i].scope, pattern)) {
            // we found the function name whose scope ends with &objName/. so -> methodName is indeed a method
            return funcTable->functions[i].returnType;
        }
    }
    return -1;
}

short check_var_parameter(FunctionList *funcTable, char *name, char *scope) 
{
    int funcNum = funcTable->funcNumber;
    for(int i = 0; i < funcNum; ++i) {
        int parNum = funcTable->functions[i].parameters.varNumber;
        for(int j = 0; j < parNum; ++j) {
            if(!strcmp(funcTable->functions[i].parameters.variables[j].name, name)) {
                if(strstr(scope, funcTable->functions[i].name)) {
                    // if the name of the function is contained in the variable scope -> variable might be a parameter
                    return 0;
                }
            }
        }
    }
    return -1;
}

short extract_param_type(FunctionList *funcTable, char *name, char *scope) 
{
    int funcNum = funcTable->funcNumber;
    for(int i = 0; i < funcNum; ++i) {
    int parNum = funcTable->functions[i].parameters.varNumber;
    for(int j = 0; j < parNum; ++j) {
        if(!strcmp(funcTable->functions[i].parameters.variables[j].name, name)) {
            if(strstr(scope, funcTable->functions[i].name)) {
                    // if the name of the function is contained in the variable scope -> variable might be a parameter
                    return funcTable->functions[i].parameters.variables[j].typeInfo.typeName;
                }
            }
        }
    }
    return -1;
}

int min(int a, int b) {
    if(a < b) {
        return a;
    }
    return b;
}

void extract_AstTypes(struct AstNode *nod) 
{
    if(nod->nodeInfo.nodeType != 0) {
        astTypes[astTypesLen++] = nod->nodeInfo.dataType;  
    }
    if(nod->left) {
       extract_AstTypes(nod->left);
    }
    if(nod->right) {
        extract_AstTypes(nod->right);
    }
}
short check_AstTypes(struct AstNode *Ast, int line) 
{
    // returns the main type of the expression if found or -1 if it is ambiguous
    astTypesLen = 0;
    extract_AstTypes(Ast);

    short precTypes[MAX_EXP_LEN];
    int precLen = 0;
    // in precTypes we put all the distinct dataTypes from the expression
    
    for(int i = 0; i < astTypesLen; ++i) {
        short found = 0;
        if(astTypes[i] == -1) {
            return -1;
        }
        for(int j = 0; j < precLen; ++j) {
            if(astTypes[i] == precTypes[j]) {
                found = 1;
                break;
            }
        }
        if(!found) {
            precTypes[precLen++] = astTypes[i];
        }
    }

    if(precLen == 1) {
        return precTypes[0];
    }

    printf("\033[31mThe expression from line %d has operands with different types.\n", line);
    printf("    Following types of operands are used in this expression: ");
    error_code = 1;
    for(int i = 0; i < precLen; ++i) {
        if(precTypes[i] < 0) {
            continue;
        }
        printf("\033[31m%s\033[0m", decodeType[precTypes[i]]);
        if(i < precLen - 1) {
            printf("\033[31m, \033[0m");
        }
        else {
            printf("\n");
        }
    }
    return -1;
}

int nr_slashes(char *scope) {
    int i = 0, slashes = 0;
    while(*(scope + i) != '\0') {
        if(*(scope + i) == '/') {
            slashes++;
        }
        i++;
    }
    return slashes;
}

int extract_variable_value(VariableList *varTable, char *name, char *scope) {
    int varNum = varTable->varNumber, deepestScope = 0, pos = -1;
    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(name, varTable->variables[i].name) && strstr(scope, varTable->variables[i].scope)) {
            int scopeLen = nr_slashes(varTable->variables[i].scope);
            if(deepestScope < scopeLen) {
                deepestScope = scopeLen;
                pos = i;
            }
        }
    }
    if(pos == -1) {
        // this shouldn't happen tho
        return 0;
    }
    return varTable->variables[pos].value[0];
}

short extract_arrayElement_value(VariableList *varTable, char *name, int index, char *scope)
{
    int varNum = varTable->varNumber, deepestScope = 0, pos = -1;
    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(name, varTable->variables[i].name) && strstr(scope, varTable->variables[i].scope) && varTable->variables[i].typeInfo.isArray) {
            int scopeLen = nr_slashes(varTable->variables[i].scope);
            if(deepestScope < scopeLen) {
                deepestScope = scopeLen;
                pos = i;
            }
        }
    }
    if(pos == -1 && index >= varTable->variables[pos].typeInfo.arrayLen) {
        // this shouldn't happen tho
        return 0;
    }
    return varTable->variables[pos].value[index];
}

short extract_classVar_value(VariableList *varTable, char *varName, char *objName, char *scope) 
{
    char pattern[MAX_VAR_LEN];
    snprintf(pattern, MAX_VAR_LEN, "&%s/", objName);
    int varNum = varTable->varNumber;

    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(varName, varTable->variables[i].name) && endWith(varTable->variables[i].scope, pattern)) {
            return varTable->variables[i].value[0];          
        }
    }
    // shouldn't happen tho
    return 0;
}

void update_variable_value(VariableList *varTable, char *name, char *scope, int newVal) {
    int varNum = varTable->varNumber, deepestScope = 0, pos = -1;
    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(name, varTable->variables[i].name) && strstr(scope, varTable->variables[i].scope)) {
            int scopeLen = nr_slashes(varTable->variables[i].scope);
            if(deepestScope < scopeLen) {
                deepestScope = scopeLen;
                pos = i;
            }
        }
    }
    if(pos < 0) {
        // this shouldn't happen tho
        return;
    }
    varTable->variables[pos].value[0] = newVal;
}

void update_arrayElement_value(VariableList *varTable, char *name, int index, char *scope, int newVal)
{
    int varNum = varTable->varNumber, deepestScope = 0, pos = -1;
    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(name, varTable->variables[i].name) && strstr(scope, varTable->variables[i].scope) && varTable->variables[i].typeInfo.isArray) {
            int scopeLen = nr_slashes(varTable->variables[i].scope);
            if(deepestScope < scopeLen) {
                deepestScope = scopeLen;
                pos = i;
            }
        }
    }
    if(pos == -1 && index >= varTable->variables[pos].typeInfo.arrayLen) {
        // this shouldn't happen tho
        return;
    }
    varTable->variables[pos].value[index] = newVal;
}

void update_classVar_value(VariableList *varTable, char *varName, char *objName, char *scope, int newVal) 
{
    char pattern[MAX_VAR_LEN];
    snprintf(pattern, MAX_VAR_LEN, "&%s/", objName);
    int varNum = varTable->varNumber;

    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(varName, varTable->variables[i].name) && endWith(varTable->variables[i].scope, pattern)) {
            varTable->variables[i].value[0] = newVal;        
            return;  
        }
    }
    return;
}


void update_var(VariableList *varTable, char *name, char *scope, int newVal) 
{
    short isArray = (strchr(name, '[') != 0);
    short isObj = (strchr(name, '.') != 0);
    if(!isArray && !isObj) {
        update_variable_value(varTable, name, scope, newVal);
    }
    if(isArray) {
        char arrName[MAX_VAR_LEN];
        int index;
        sscanf(name, "%[^[][%d]", arrName, &index);
        update_arrayElement_value(varTable, arrName, index, scope, newVal);
    }
    if(isObj) {
        char objName[MAX_VAR_LEN], varName[MAX_VAR_LEN];
        sscanf(name, "%[^'.'].%s", objName, varName);
        update_classVar_value(varTable, varName, objName, scope, newVal);
    }
}

int computeAst(struct AstNode *nod, char *scope, int line)
{
    if(!nod) {
        return 0;
    }
    if(nod->nodeInfo.nodeType != 0) {
        if(nod->nodeInfo.nodeType == 2 && !nod->nodeInfo.dataType) {
            return atoi(nod->nodeInfo.value);
        }
        if(nod->nodeInfo.nodeType == 1 && !nod->nodeInfo.dataType) {
            short isArray = (strchr(nod->nodeInfo.value, '[') != NULL);
            short isObj = (strchr(nod->nodeInfo.value, '.') != NULL);
            if(!isArray && !isObj) {
                return extract_variable_value(&allVariables, nod->nodeInfo.value, scope);
            }
            if(isArray) {
                char arrName[MAX_VAR_LEN];
                int index;
                sscanf(nod->nodeInfo.value, "%[^[][%d]", arrName, &index);
                return extract_arrayElement_value(&allVariables, arrName, index, scope);
            }
             if(isObj) {
                char objName[MAX_VAR_LEN], varName[MAX_VAR_LEN];
                sscanf(nod->nodeInfo.value, "%[^'.'].%s", objName, varName);
                return extract_classVar_value(&allVariables, varName, objName, scope);
            }
        }
        return 0;
    }
    char *op = nod->nodeInfo.value;
    if(!strcmp(op, "+")) {
        return computeAst(nod->left, scope, line) + computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,"-")){
        return computeAst(nod->left, scope, line) - computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,"*")){
        return computeAst(nod->left, scope, line) * computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,"/")){
        int right_node_value=computeAst(nod->right, scope, line);
        if(right_node_value==0)
        {
            error_code=1;
            return 0;
        }
        return computeAst(nod->left, scope, line) / computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,"||")){
        return computeAst(nod->left, scope, line) || computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,"&&")){
        return computeAst(nod->left, scope, line) && computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,"!=")){
        return computeAst(nod->left, scope, line) != computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,"==")){
        return computeAst(nod->left, scope, line) == computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,"<=")){
        return computeAst(nod->left, scope, line) <= computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,"<")){
        return computeAst(nod->left, scope, line) < computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,">=")){
        return computeAst(nod->left, scope, line) >= computeAst(nod->right, scope, line);
    }
    if(!strcmp(op,">")){
        return computeAst(nod->left, scope, line) > computeAst(nod->right, scope, line);
    }
}

void check_and_update_variable(VariableList *varTable, char *varName, int varType, char *scope, struct AstNode *Ast, int line) 
{
    int expType = check_AstTypes(Ast, line);

    if(expType == -1) {
        return;
    }

    if(varType != expType) {
        printf("\033[31mThe left side of the assignment on line %d has a different type than the right side!\n", line);
        printf("    The type of left side is '%s' while the type of right side is '%s'\n\033[0m", decodeType[varType], decodeType[expType]);
        error_code = 1;
        return;
    }

    int astResult = computeAst(Ast, scope, line);
    update_var(varTable, varName, scope, astResult);
}

 short check_const_param(FunctionList *funcTable, char *name, char *scope) 
 {
    int funcNum = funcTable->funcNumber;
    for(int i = 0; i < funcNum; ++i) {
    int parNum = funcTable->functions[i].parameters.varNumber;
    for(int j = 0; j < parNum; ++j) {
        if(!strcmp(funcTable->functions[i].parameters.variables[j].name, name)) {
            if(strstr(scope, funcTable->functions[i].name)) {
                    // if the name of the function is contained in the variable scope -> variable might be a parameter
                    if(funcTable->functions[i].parameters.variables[j].typeInfo.isConst) {
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
 }

short check_const_var(VariableList *varTable, char *name, char *scope)
{
    int varNum = varTable->varNumber, deepestScope = 0, pos = -1;
    for(int i = 0; i < varNum; ++i) {
        if(!strcmp(name, varTable->variables[i].name) && strstr(scope, varTable->variables[i].scope)) {
            int scopeLen = strlen(varTable->variables[i].scope);
            if(deepestScope < scopeLen) {
                deepestScope = scopeLen;
                pos = i;
            }
        }
    }
    if(pos == -1) {
        return 0;
    }
    return varTable->variables[pos].typeInfo.isConst;
}

void do_var_assign(char *varName, char *scope, struct AstNode* Ast, int line, short paramFlag) 
{
    int varType;
    short isConst;

    if(paramFlag) {
        if(!isClass) {
            isConst = check_const_param(&allFunctions, varName, scope);
            varType = extract_param_type(&allFunctions, varName, scope);
        }
        else {
            isConst = check_const_param(&classFunctions, varName, scope);
            varType = extract_param_type(&classFunctions, varName, scope);
        }
    }
    else { 
        if(!isClass) {
            isConst = check_const_var(&allVariables, varName, scope);
            varType = extract_variable_type(&allVariables, varName, scope);   
        }
        else {
            isConst = check_const_var(&classVariables, varName, scope);
            varType = extract_variable_type(&classVariables, varName, scope);   
        }
    }
    if(isConst) {
        printf("\033[31mThe variable %s on line %d is a constant and can't be reassigned.\n\033[0m", varName, line);
        error_code = 1;
        return;
    }

    if(!isClass) {
        check_and_update_variable(&allVariables, varName, varType, scope, Ast, line);   
    }
    else {
        check_and_update_variable(&classVariables, varName, varType, scope, Ast, line);
    }
}

void do_declVar_assign(VariableList *varTable, char *varName, char *scope, struct AstNode *Ast, int line, short dataType)
{
    check_and_update_variable(&allVariables, varName, dataType, scope, Ast, line);
}

void do_classVar_assign(char *varName, char *obj, char *scope, struct AstNode* Ast, int line) 
{
    char actualName[MAX_VAR_LEN];
    snprintf(actualName, MAX_VAR_LEN, "%s.%s", obj, varName);
    int varType = extract_class_varType(&allVariables, varName, obj);
    
    check_and_update_variable(&allVariables, actualName, varType, scope, Ast, line);
}

void do_arrayElem_assign(char *varName, int index, char *scope, struct AstNode* Ast, int line) 
{
    char actualName[MAX_VAR_LEN];
    snprintf(actualName, MAX_VAR_LEN, "%s[%d]", varName, index);
    int varType = extract_variable_type(&allVariables, varName, currentScope);
    
    check_and_update_variable(&allVariables, actualName, varType, scope, Ast, line);
}

void match_arguments(char *funcName, int *arg_types, int argNr, VariableList *parameters, int line, short isMethod) 
{
    int paramNr = parameters->varNumber;
    char func_or_method[MAX_VAR_LEN];
    if(!isMethod) {
        strncpy(func_or_method, "function", MAX_VAR_LEN);
    }
    else {
        strncpy(func_or_method, "method", MAX_VAR_LEN);
    }

    if(argNr != paramNr) {
        printf("\033[31mIncorrect %s call at line %d.\n", func_or_method, line);
        printf("    The %s %s has %d parameters, while %d arguments has been passed.\n\033[0m", func_or_method, funcName, paramNr, argNr);
        error_code = 1;
        return;
    }

    // arg number match the paramNumber ==> paramNr == argNrs
    short typeMatch = 1;
    for(int i = 0; i < paramNr; ++i) {
        if(parameters->variables[i].typeInfo.typeName != arg_types[i] && arg_types[i] != -1) {
            // if it's -1 then it was already broken so another error will appear on the screen
            typeMatch = 0;
        }
    }
    
    if(typeMatch) {
        // all good -> number matched and types matched 
        return;
    }

    printf("\033[31mIncorrect %s %s call at line %d. The types of the following arguments and parameters doesn't match:\n\033[0m", func_or_method, funcName, line);
    error_code = 1;
    for(int i = 0; i < paramNr; ++i) {
        if(parameters->variables[i].typeInfo.typeName != arg_types[i] && arg_types[i] != -1) {
            printf("\033[31m    Argument %d ishaving the type '%s', while type '%s' is expected.\n\033[0m", i + 1, decodeType[arg_types[i]], decodeType[parameters->variables[i].typeInfo.typeName]);
        }
    }
}

void check_func_arguments(FunctionList *funcTable, char *funcName, int *arg_types, int argNr, int line) 
{
    int funcNum = funcTable->funcNumber;
    
    int pos = -1;
    for(int i = 0; i < funcNum; ++i) {
        if(!strcmp(funcName, funcTable->functions[i].name)) {
            pos = i;
        }
    }

    if(pos == -1) {
        return;
    }  
    match_arguments(funcName, arg_types, argNr, &funcTable->functions[pos].parameters, line, 0);
}

void check_method_arguments(FunctionList *funcTable, char *methodName, char *objName, int *arg_types, int argNr, int line) 
{
    int funcNum = funcTable->funcNumber;
    char pattern[MAX_VAR_LEN];
    snprintf(pattern, MAX_VAR_LEN, "&%s/", objName);

    int pos = -1;
    for(int i = 0; i < funcNum; ++i) {
        if(!strcmp(methodName, funcTable->functions[i].name) && endWith(funcTable->functions[i].scope, pattern)) {
            // we found the function name whose scope ends with &objName/. so -> methodName is indeed a method
            pos = i;
        }
    }

    if(pos == -1) {
        return;
    }  

    char fullName[MAX_VAR_LEN];
    snprintf(fullName, MAX_VAR_LEN, "%s->%s", objName, methodName);
    match_arguments(fullName, arg_types, argNr, &funcTable->functions[pos].parameters, line, 1);
}

void from_class_to_all(char *clsName, char *scope) {
    char pattern[MAX_SCOPE_LEN];
    snprintf(pattern, MAX_SCOPE_LEN, "&%s/", clsName);

    // copy every variable from classVariables that contains the scope /clsName/ to AllVariables with scope changed to newScope
    for(int i = 0; i < classVariables.varNumber; ++i) {
        if(strstr(classVariables.variables[i].scope, pattern)) {
            // we found a variable from that class :))
            char aux[MAX_SCOPE_LEN];
            
            char *end = strstr(classVariables.variables[i].scope, pattern);
            while(*end != '/' && *end != '\0') {
                end++;
            } 
            end++;
            char newScope[MAX_SCOPE_LEN];
            
            strncpy(newScope, scope, MAX_SCOPE_LEN);    
            strcat(newScope, end);
            
            strncpy(aux, classVariables.variables[i].scope, MAX_SCOPE_LEN);
            strncpy(classVariables.variables[i].scope, newScope, MAX_SCOPE_LEN);
            insert_var(&allVariables, &classVariables.variables[i]);
            strncpy(classVariables.variables[i].scope, aux, MAX_SCOPE_LEN);
        }
    }

    // copy functions from classFunctions with scope changed 
    for(int i = 0; i < classFunctions.funcNumber; ++i) {
        if(endWith(classVariables.variables[i].scope, pattern)) {
            // we found a variable from that class :))
            char aux[MAX_SCOPE_LEN];
            strncpy(aux, classFunctions.functions[i].scope, MAX_SCOPE_LEN);
            strncpy(classFunctions.functions[i].scope, scope, MAX_SCOPE_LEN);
            insert_func(&allFunctions, &classFunctions.functions[i]);
            strncpy(classFunctions.functions[i].scope, aux, MAX_SCOPE_LEN);
        }
    }
}

short checkClass(char *clsName) {
    for(int i = 0; i < classNr; ++i) {
        if(!strncmp(classList[i], clsName, MAX_VAR_LEN)) {
            return 1;
        }
    }
    return 0;
}

#endif