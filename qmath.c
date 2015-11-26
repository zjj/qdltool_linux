#include "qmath.h"
//from amss

static int operatorHigherPrecedenceThan(char left, char right) {
    char operator_string[] = "-+*/";
    char* left_pos = strchr(operator_string, left);
    char* right_pos = strchr(operator_string, right);
    if (left_pos != NULL && right_pos != NULL && left_pos > right_pos) {
        return 1;
    }   
    return 0;
}


boolean calc(char* buffer, SIZE_T* result) {
    SIZE_T i = 0;
    char ch = 0;
    boolean num_conversion = TRUE;
    // The length of this buffer dictates how large a number we
    // can handle in the expression
    char number_buffer[16];
    SIZE_T number_length = 0, number_start = 0, number_end = 0;
    SIZE_T number = 0;

    // Parsing stack
    // We will use the token stack to store chars such as (, +, -, n, *, etc.
    // where n represents a number which must be popped off the number stack
    char postfix_string[16];
    SIZE_T postfix_string_number_list[8];
    char operator_stack[16];
    SIZE_T postfix_string_size = 0,
           postfix_string_number_list_size = 0,
           operator_stack_size = 0;

    // Declare variables and stacks used in calculating the final result
    SIZE_T calculation_stack[8];
    SIZE_T calculation_stack_size = 0;
    SIZE_T operand_first, operand_second;

    enum {
        EXPR_NOT_IN_NUMBER,
        EXPR_IN_NUMBER
    } expr_state;
    expr_state = EXPR_NOT_IN_NUMBER;

    while (1) {
        ch = buffer[i];
        if (expr_state == EXPR_NOT_IN_NUMBER) {
            if (isdigit(ch)) {
                number_start = i;
                expr_state = EXPR_IN_NUMBER;
            }
            else if (ch == '(') {
                operator_stack[operator_stack_size++] = ch;
            }
            else if (ch == ')') {
                while (operator_stack_size > 0) {
                    ch = operator_stack[--operator_stack_size];
                    if (ch == '(') {
                        break;
                    }
                    postfix_string[postfix_string_size++] = ch;
                }
            }
            else if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
                while (operator_stack_size > 0 && operatorHigherPrecedenceThan(operator_stack[operator_stack_size - 1], ch)) {
                    postfix_string[postfix_string_size++] = operator_stack[--operator_stack_size];
                }
                operator_stack[operator_stack_size++] = ch;
            }
        }
        else if (expr_state == EXPR_IN_NUMBER) {
            if (!isdigit(ch)) {
                number_end = i;
                expr_state = EXPR_NOT_IN_NUMBER;
                // Roll back one position to act as though we have not seen this character
                // This is so that we do not have to duplicate the token handling logic
                // from the EXPR_NOT_IN_NUMBER state
                i--;
                number_length = number_end - number_start;
                if (number_length > sizeof(number_buffer) - 1) {
                    return FALSE;
                }
                memcpy(number_buffer, &buffer[number_start], number_length);
                number_buffer[number_length] = '\0';
                number = strtoul(number_buffer, NULL, 0);
                postfix_string[postfix_string_size++] = postfix_string_number_list_size + '0';
                postfix_string_number_list[postfix_string_number_list_size++] = number;
            }
        }
        if (ch == '\0') {
            break;
        }
        i++;
    }
    while (operator_stack_size > 0) {
        postfix_string[postfix_string_size++] = operator_stack[--operator_stack_size];
    }

    for (i = 0; i < postfix_string_size; i++) {
        switch (postfix_string[i]) {
            case '/':
                operand_second = calculation_stack[--calculation_stack_size];
                operand_first = calculation_stack[--calculation_stack_size];
                calculation_stack[calculation_stack_size++] = operand_first / operand_second;
                break;

            case '*':
                operand_second = calculation_stack[--calculation_stack_size];
                operand_first = calculation_stack[--calculation_stack_size];
                calculation_stack[calculation_stack_size++] = operand_first * operand_second;
                break;

            case '+':
                operand_second = calculation_stack[--calculation_stack_size];
                operand_first = calculation_stack[--calculation_stack_size];
                calculation_stack[calculation_stack_size++] = operand_first + operand_second;
                break;

            case '-':
                operand_second = calculation_stack[--calculation_stack_size];
                operand_first = calculation_stack[--calculation_stack_size];
                calculation_stack[calculation_stack_size++] = operand_first - operand_second;
                break;

            default:
                calculation_stack[calculation_stack_size++] = postfix_string_number_list[postfix_string[i] - '0'];
                break;
        }
    }
    if (result != NULL && calculation_stack_size > 0) {
        *result = calculation_stack[--calculation_stack_size];
        return TRUE;
    }
    else {
        return FALSE;
    }
}

int main()
{
    int a = 0;
    calc("(1+10)-2", &a);
    printf("%d\n", a);
}
