//
// Created by 15461 on 2022/11/27.
//计算器
//输入包含若干行，每行为一个表达式或赋值语句
//对于每一个输入的表达式或赋值语句，输出 Error 或相应的值。
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define Len 1024
typedef enum {
    VARIABLE, INTEGER, OPERATOR, FLOAT, ERROR
} Type;
typedef struct {
    char str[32];
    Type type;
    union {
        double fVal;
    } val;
    bool assignment;
} Value;
typedef struct {
    Type type;
    char str[32];
    union {
        double fVal;
    } val;
} Token;
//variable中的信息：类型，数值,是否赋值
Value variable[Len];
//token中的信息：类型，数值
Token token[Len];
int numbercount = 0;

void PrintValue(Value p);

Value Eval(int l, int r);

Value EvalAssign(int l, int r);

bool CheckParentheses(int l, int r);

Value MeetValue(Value v1, Value v2, Token operator);

int FindMainOperator(int l, int r);

bool Check_Minus(int l, int r);

int main() {
    //首先进行输入并分析词法
    char *rule = "= + - * / ( ) . 0 1 2 3 4 5 6 7 8 9 _ a b c d e f g h i j k l m n o p q"
                 "r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z ";
    int Left[Len] = {0};
    int Right[Len] = {0};
    char mid = ' ';
    for (int i = 0; i < Len; ++i) {
        token[i].type = ERROR;
        memset(token[i].str, '\0', sizeof(char) * 32);
        variable[i].assignment = false;
        memset(variable[i].str, '\0', sizeof(char) * 32);
        token[i].val.fVal=0;
    }
    for (int i = 0, k = 0; scanf("%s", token[i].str) != EOF; ++i) {
        scanf("%c", &mid);
        if (mid == '\n') {
            Right[k] = i;
            k++;
        }
        int len = strlen(token[i].str);
        int judge1 = 1, judge2 = 1, judge3 = 1, judge4 = 1, judge5 = 1;
        if (strstr(rule, token[i].str) >= strstr(rule, "=") &&
            strstr(rule, token[i].str) <= strstr(rule, ")")) {
            token[i].type = OPERATOR;
            goto integer;
        } else if (token[i].str[0] == '0' && token[i].str[1] == '.') {
            if (len == 2) {
                token[i].type = ERROR;
                goto integer;
            }
            for (int j = 2; j < len; j++) {
                if (strchr(rule, token[i].str[j]) <= strchr(rule, '.') ||
                    strchr(rule, token[i].str[j]) > strchr(rule, '9')) {
                    judge2--;
                    break;
                }
            }
            if (judge2 == 1) {
                token[i].val.fVal += (token[i].str[0] - 48);
                for (int j = 2; j < len; ++j) {
                    token[i].val.fVal += (token[i].str[j] - 48) * pow(10, -j + 1);
                }
                token[i].type = FLOAT;
                goto integer;
            }
        } else if (token[i].str[0] > '0' && token[i].str[0] <= '9') {
            int index = 0;
            for (int j = 1; j < len; j++) {
                if (token[i].str[j] == '.' && j != len - 1) {
                    judge4--;
                    index = j;
                }
                if (token[i].str[j] < '0' || token[i].str[j] > '9') {
                    judge5--;
                }
            }
            if (judge4 == 0 && judge5 == 0) {
                token[i].type = FLOAT;
                for (int j = 0; j < index; ++j) {
                    token[i].val.fVal += (token[i].str[index - 1 - j] - 48) * pow(10, j);
                }
                for (int j = index + 1; j < len; ++j) {
                    token[i].val.fVal += (token[i].str[j] - 48) * pow(10, -j + index);
                }
                goto integer;
            }
        }
        if (token[i].str[0] >= '0' && token[i].str[0] <= '9') {
            if (len != 1 && token[i].str[0] == '0') {
                token[i].type = ERROR;
                goto integer;
            }
            for (int j = 1; j < len; ++j) {
                if (*(token[i].str + j) < 48 || *(token[i].str + j) > 57) {
                    judge1--;
                    break;
                }
            }
            if (judge1 == 1) {
                token[i].type = INTEGER;
                for (int j = 0; j < len; ++j) {
                    token[i].val.fVal += (token[i].str[len - 1 - j] - 48) * pow(10, j);
                }
            }
        }
        if (token[i].str[0] < '0' ||
            token[i].str[0] > '9') {
            for (int j = 0; j < len; ++j) {
                if (strchr(rule, token[i].str[j]) <= strchr(rule, '.')) {
                    judge3--;
                    break;
                }
            }
            if (judge3 == 1) {
                token[i].type = VARIABLE;
                int judge = 0;
                for (int j = 0; j < numbercount; ++j) {
                    if (strcmp(token[i].str, variable[j].str) == 0) {
                        judge++;
                        break;
                    }
                }
                if (judge == 0) {
                    for (int j = 0; j < len; ++j) {
                        variable[numbercount].str[j] = token[i].str[j];
                    }
                    numbercount++;
                }
            }
        }
        integer:;
    }
    for (int i = 1; Right[i] != 0; ++i) {
        Left[i] = Right[i - 1] + 1;
    }
    //Left[i],Right[i]分别表示输入的第i行在token中的左右坐标
    //再进行语法分析及赋值
    for (int i = 0; Right[i] != 0 || i == 0; ++i) {
        PrintValue(EvalAssign(Left[i], Right[i]));
        printf("\n");
    }
    return 0;
}

//打印函数
void PrintValue(Value p) {
    if (p.type == ERROR) {
        printf("Error");
        return;
    }
    if (p.type == INTEGER) {
        printf("%d", (int) p.val.fVal);
    } else {
        printf("%.6f", p.val.fVal);
    }
}

//判断合法及求值函数（主函数）
Value Eval(int l, int r) {
    Value error;
    error.type = ERROR;
    for (int j = l; j <= r; ++j) {
        if (token[j].type == ERROR) {
            return error;
        }
    }
    if (l > r) {
        return error;
    } else if (l == r) {
        if (token[l].type == OPERATOR) {
            return error;
        }
        if (token[l].type == INTEGER && token[l].str[0] >= '0' && token[l].str[0] <= '9') {
            Value p;
            p.type = token[l].type;
            p.val.fVal = token[l].val.fVal;
            return p;
        } else if (token[l].type == FLOAT && token[l].str[0] >= '0' && token[l].str[0] <= '9') {
            Value p;
            p.type = token[l].type;
            p.val.fVal = token[l].val.fVal;
            return p;
        }
        int index = 0;
        for (int i = 0; i < numbercount; ++i) {
            if (strcmp(token[l].str, variable[i].str) == 0) {
                index = i;
                break;
            }
        }
        Value p;
        if (variable[index].assignment == false) {
            return error;
        }
        p.type = variable[index].type;
        p.val.fVal = variable[index].val.fVal;
        return p;
    } else if (CheckParentheses(l, r) == true) {
        return Eval(l + 1, r - 1);
    } else if (Check_Minus(l, r) == true) {
        Value p,q;
        q=Eval(l + 1, r);
        p.type = q.type;
        p.val.fVal = -q.val.fVal;
        return p;
    } else {
        if (FindMainOperator(l, r) == 114514) {
            return error;
        }
        Value v1, v2;
        v1 = Eval(l, FindMainOperator(l, r) - 1);
        v2 = Eval(FindMainOperator(l, r) + 1, r);
        return MeetValue(v1, v2, token[FindMainOperator(l, r)]);
    }
}

//赋值函数
Value EvalAssign(int l, int r) {
    Value error;
    error.type = ERROR;
    int judge = 0;
    for (int i = l; i <= r; ++i) {
        if (token[i].type == ERROR) {
            return error;
        }
        if (token[i].type == OPERATOR && token[i].str[0] == '=') {
            judge++;
        }
    }
    if (judge > 0) {
        if (token[l].type != VARIABLE || token[l + 1].str[0] != '=') {
            return error;
        }
        int index = 0;
        for (int i = 0; i < numbercount; ++i) {
            if (strcmp(token[l].str, variable[i].str) == 0) {
                index = i;
                break;
            }
        }
        Value p;
        p = EvalAssign(l + 2, r);
        if (p.type != ERROR) {
            variable[index].val.fVal = p.val.fVal;
            variable[index].type = p.type;
            variable[index].assignment = true;
            return variable[index];
        }
        if (p.type == ERROR) {
            return error;
        }
    } else {
        return Eval(l, r);
    }
    return error;
}

bool CheckParentheses(int l, int r) {
    if (token[l].str[0] != '(' || token[r].str[0] != ')') {
        return false;
    }
    int num = 0;
    for (int i = l; i < r; ++i) {
        if (token[i].str[0] == '(') {
            num++;
        }
        if (token[i].str[0] == ')') {
            num--;
        }
        if (num <= 0) {
            return false;
        }
    }
    num--;
    if (num == 0) {
        return true;
    } else {
        return false;
    }
}

Value MeetValue(Value v1, Value v2, Token operator) {
    Value error, p;
    error.type = ERROR;
    if (v1.type == ERROR || v2.type == ERROR) {
        return error;
    }
    if (v1.type != v2.type) {
        p.type = FLOAT;
    } else {
        p.type = v1.type;
    }
    if (operator.str[0] == '+') {
        p.val.fVal = v1.val.fVal + v2.val.fVal;
        return p;
    }
    if (operator.str[0] == '-') {
        p.val.fVal = v1.val.fVal - v2.val.fVal;
        return p;
    }
    if (operator.str[0] == '*') {
        p.val.fVal = v1.val.fVal * v2.val.fVal;
        return p;
    }
    if (operator.str[0] == '/') {
        if (p.type == INTEGER) {
            p.val.fVal = (int) v1.val.fVal / (int) v2.val.fVal;
            return p;
        }
        p.val.fVal = v1.val.fVal / v2.val.fVal;
        return p;
    }
    return error;
}

int FindMainOperator(int l, int r) {
    int left[Len] = {0}, right[Len] = {0};
    int k = 1;
    for (int i = l; i <= r; ++i) {
        if (token[i].str[0] == '(' && token[i].type == OPERATOR) {
            left[k] = i;
        } else if (token[i].str[0] == ')' && token[i].type == OPERATOR) {
            right[k] = i;
            k++;
        }
    }
    left[k] = r;
    right[0] = l;
    for (int i = k; i >= 1; --i) {
        for (int j = left[i]; j > right[i - 1]; --j) {
            if (j != l && j != r &&
                (token[j].str[0] == '+' || (token[j].str[0] == '-' && token[j - 1].str[0] != '+' &&
                                            token[j - 1].str[0] != '-' && token[j - 1].str[0] != '*' &&
                                            token[j - 1].str[0] != '/'))) {
                int judge = 0;
                for (int m = l; m < j; ++m) {
                    if (token[m].str[0] == '(') {
                        judge++;
                    } else if (token[m].str[0] == ')') {
                        judge--;
                    }
                }
                if (judge == 0) { return j; }
            }
        }
    }
    for (int i = k; i >= 1; --i) {
        for (int j = left[i]; j > right[i - 1]; --j) {
            if ((token[j].str[0] == '*' || token[j].str[0] == '/') && j != l && j != r) {
                int judge = 0;
                for (int m = l; m < j; ++m) {
                    if (token[m].str[0] == '(') {
                        judge++;
                    } else if (token[m].str[0] == ')') {
                        judge--;
                    }
                }
                if (judge == 0) { return j; }
            }
        }
    }
    return 114514;
}

bool Check_Minus(int l, int r) {
    if (FindMainOperator(l, r) == 114514 && token[l].str[0] == '-') {
        return true;
    }
    return false;
}