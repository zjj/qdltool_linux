#include "utils.h"

static int operatorHigherPrecedenceThan(char left, char right) {
    char operator_string[] = "-+*/";
    char* left_pos = strchr(operator_string, left);
    char* right_pos = strchr(operator_string, right);
    if (left_pos != NULL && right_pos != NULL && left_pos > right_pos) {
        return 1;
    }   
    return 0;
}


boolean evaluator(char* buffer, size_t* result) {
    size_t i = 0;
    char ch = 0;
    boolean num_conversion = TRUE;
    // The length of this buffer dictates how large a number we
    // can handle in the expression
    char number_buffer[16];
    size_t number_length = 0, number_start = 0, number_end = 0;
    size_t number = 0;

    // Parsing stack
    // We will use the token stack to store chars such as (, +, -, n, *, etc.
    // where n represents a number which must be popped off the number stack
    char postfix_string[16];
    size_t postfix_string_number_list[8];
    char operator_stack[16];
    size_t postfix_string_size = 0,
           postfix_string_number_list_size = 0,
           operator_stack_size = 0;

    // Declare variables and stacks used in calculating the final result
    size_t calculation_stack[8];
    size_t calculation_stack_size = 0;
    size_t operand_first, operand_second;

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

char *strrep(const char *string, const char *substr, const char *replacement)
{
  char *tok = NULL;
  char *newstr = NULL;
  char *oldstr = NULL;
  /* if either substr or replacement is NULL, duplicate string a let caller handle it */
  if ( substr == NULL || replacement == NULL ) return strdup (string);
  newstr = strdup (string);
  while ( (tok = strstr ( newstr, substr ))){
    oldstr = newstr;
    newstr = malloc ( strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) + 1 );
    /*failed to alloc mem, free old string and return NULL */
    if ( newstr == NULL ){
      free (oldstr);
      return NULL;
    }
    memcpy ( newstr, oldstr, tok - oldstr );
    memcpy ( newstr + (tok - oldstr), replacement, strlen ( replacement ) );
    memcpy ( newstr + (tok - oldstr) + strlen( replacement ), tok + strlen ( substr ), strlen ( oldstr ) - strlen ( substr ) - ( tok - oldstr ) );
    memset ( newstr + strlen ( oldstr ) - strlen ( substr ) + strlen ( replacement ) , 0, 1 );
    free (oldstr);
  }
  return newstr;
}

size_t firehose_strtoint(char *s)
{
    size_t ret;
    char buffer[128] = {0};
    char num_disk_sectors[128] = {0};
    sprintf(num_disk_sectors, "%zu", NUM_DISK_SECTORS);
    char *eval;

    eval =  strrep(s, "NUM_DISK_SECTORS", num_disk_sectors);
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, eval);
    free(eval);
    eval = NULL;
    
    eval = strrep(buffer, ".", "");
    memset(buffer, 0, sizeof(buffer));
    strcpy(buffer, eval);  
    free(eval);
    eval = NULL;

    bool cal = evaluator(buffer, &ret);
    if (!cal){
        xerror("failed to firehose_strtoint %s\n", s);
    }
    return ret;
}
