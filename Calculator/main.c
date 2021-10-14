#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdlib.h>
#include <math.h>

typedef enum {
    NUMBER,
    ADD, MINUS,
    DIVIDE, MULTIPLY,
    POWER, FACTORIAL,
    START_BRACKET, END_BRACKET,
    END_OF_EXPRESSION
} Symbol;

typedef struct {
    const char *value;
    Symbol type;
    int bindingPower;
} Token;

void beep();
void blue();
void green();
void purple();
void error(char *str);

void printHelpManual();

char *lowercase(char *str);
double factorial(double left);
Token initToken(const char *val, Symbol ty);
Token advance();
void resetGlobalVariables();
void stripTrailingZeros(char *str);

void checkParenthesesMatch(char *inputString);
void checkExpressionValidity(char *exp);

bool isValidCharacter(char c);
bool isBlank(char *string);
bool isNumeric(char c);

void tokenize(char *exp);
void tokenizeNumber();
int findNumberOfTokens(char *exp);

double expression(int bindingPower);
double led(Token tempToken, double left);
double nud(Token tempToken);

bool hadError = false;
int start = 0;
int current = 0;
int parseCurrent = 0;

Token tokens[1024];
Token token;

char userExp[1024];

int main() {
  green();
  puts("Calculator (9 d.p.)");
  puts("-------------------\n");
  puts("Type 'help' for help, 'clear' to clear the screen, or 'exit' to exit the program.\n");
  puts("Supported operations: +, -, *, /, ^, !, ()");
  puts("Supported characters: [0-9], [.]\n");

  while (true) {
    resetGlobalVariables();
    purple();
    printf("> ");
    fgets(userExp, 1024, stdin);
    userExp[strcspn(userExp, "\n")] = 0;
    beep();
    if (strstr(lowercase(userExp), "exit") != NULL) {
      blue();
      printf("Goodbye!\n");
      return 0;
    } else if (strstr(lowercase(userExp), "clear") != NULL) {
      system("clear");
      continue;
    } else if (strstr(lowercase(userExp), "help") != NULL) {
      printHelpManual();
      continue;
    }

    checkExpressionValidity(userExp);

    if (hadError) {
      continue;
    }

    checkParenthesesMatch(userExp);

    if (hadError) {
      continue;
    }

    tokenize(userExp);

    if (findNumberOfTokens(userExp) == 0) {
      continue;
    }

    token = tokens[parseCurrent];

    double result = expression(0);

    if (result == INFINITY || result == -INFINITY) {
      error("Result reached ±∞.");
      error("Hint: this may be because of double factorials (e.g., '5!!'), exponentiation or divide by 0.");
    } else if (isnan(result)) {
      error("Result is not a number.");
      error("Hint: this may be because of divide by 0.");
      error("Hint: this may be because result is imaginary or complex.");
    } else if (!hadError) {
      char resultString[1024];
      sprintf(resultString, "%.9f", result);
      stripTrailingZeros(resultString);

      blue();
      printf("%s\n\n", resultString);
    }
  }
}

void stripTrailingZeros(char *str) {
  int index = 0;
  bool hadDecimal = false;
  while (str[index] != '.' && index + 1 < strlen(str)) {
    index++;
    if (str[index] == '.') {
      hadDecimal = true;
    }
  }
  if (hadDecimal) {
    for (int indice = index; indice < strlen(str); indice++) {
      bool allAfterAreZeros = true;
      for (int ind = indice + 1; ind < strlen(str); ind++) {
        if (str[ind] != '0') {
          allAfterAreZeros = false;
        }
      }
      if (allAfterAreZeros) {
        if (indice == index) {
          str[indice] = ' ';
        }
        for (int ind = indice + 1; ind < strlen(str); ind++) {
          str[ind] = ' ';
        }
      }
    }
  }
}

void resetGlobalVariables() {
  hadError = false;
  parseCurrent = 0;
  current = 0;
  start = 0;
  token = initToken("", END_OF_EXPRESSION);
}

double led(Token tempToken, double left) {
  switch (tempToken.type) {
    case ADD:
      return left + expression(10);
    case MINUS:
      return left - expression(10);
    case MULTIPLY:
      return left * expression(20);
    case DIVIDE:
      return left / expression(20);
    case POWER:
      return pow(left, expression(30 - 1));
    case FACTORIAL:
      return factorial(left);
    default:
      error("Unable to parse expression.");
      return 0;
  }
}

double factorial(double left) {
  double result = 1;
  for (int i = 1; i <= left; i++) {
    result *= i;
  }
  return result;
}

double nud(Token tempToken) {
  switch (tempToken.type) {
    case NUMBER:
      return strtod(tempToken.value, NULL);
    case MINUS:
      return -expression(25);
    case START_BRACKET: {
      double val = expression(0);
      if (tokens[parseCurrent].type != END_BRACKET) {
        error("Expected ending bracket ')'.");
      }
      token = advance();
      return val;
    }
    case END_BRACKET:
      error("Unnecessary brackets: there isn't anything in the brackets.");
    default: {
      char errorMessage[200];
      sprintf(errorMessage, "Unexpected token '%s'.", tempToken.value);
      error(errorMessage);
      return 0;
    }
  }
}

Token advance() {
  if (parseCurrent + 1 < findNumberOfTokens(userExp)) {
    return tokens[++parseCurrent];
  } else {
    return initToken("", END_OF_EXPRESSION);
  }
}

double expression(int bindingPower) {
  Token t = token;
  token = advance();

  double left = nud(t);
  while (bindingPower < token.bindingPower) {
    t = token;
    token = advance();

    left = led(t, left);
  }
  return left;
}

void tokenize(char *exp) {
  int indexToken = 0;
  start = 0;
  current = 0;

  while (current < strlen(exp)) {
    char c = exp[current];
    if (isNumeric(c)) {
      tokenizeNumber();
      char *sub = malloc(strlen(exp));
      strncpy(sub, exp + start, current + 1 - start);
      tokens[indexToken++] = initToken(sub, NUMBER);
      current++;
      start = current;
      continue;
    }
    switch (c) {
      case ' ':
        break;
      case '(':
        tokens[indexToken++] = initToken("(", START_BRACKET);
        break;
      case ')':
        tokens[indexToken++] = initToken(")", END_BRACKET);
        break;
      case '+':
        tokens[indexToken++] = initToken("+", ADD);
        break;
      case '-':
        tokens[indexToken++] = initToken("-", MINUS);
        break;
      case '*':
        tokens[indexToken++] = initToken("*", MULTIPLY);
        break;
      case '/':
        tokens[indexToken++] = initToken("/", DIVIDE);
        break;
      case '^':
        tokens[indexToken++] = initToken("^", POWER);
        break;
      case '!':
        tokens[indexToken++] = initToken("!", FACTORIAL);
        break;
      case '.':
        error("Error: Unexpected '.', please have digits before '.' (e.g., 0.1 instead of .1)");
        error("       Also, numbers can only have one '.' (e.g., no 1.1.1)\n");
        break;
      default: {
        char errorMessage[200];
        sprintf(errorMessage, "Error: Unknown character: '%c'\n", c);
        error(errorMessage);
        break;
      }
    }
    current++;
    start = current;
  }
}

int findNumberOfTokens(char *exp) {
  start = 0;
  current = 0;

  int numTokens = 0;

  while (current < strlen(exp)) {
    char c = exp[current];
    if (isNumeric(c)) {
      tokenizeNumber();
      numTokens++;
      current++;
      start = current;
      continue;
    }
    switch (c) {
      case '(':
      case ')':
      case '+':
      case '-':
      case '*':
      case '/':
      case '^':
      case '!':
        numTokens++;
        break;
      default:
        break;
    }
    current++;
    start = current;
  }
  return numTokens;
}

void tokenizeNumber() {
  while (current + 1 < strlen(userExp) && (isNumeric(userExp[current + 1]))) {
    current++;
  }

  if (current + 1 < strlen(userExp) && userExp[current + 1] == '.') {
    current++;
    while (current + 1 < strlen(userExp) && isNumeric(userExp[current + 1])) {
      current++;
    }
  }
}

int hasMatchingBrackets(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    if (i + 1 < strlen(string) &&
        string[i] == '(') {
      int tempI = i + 1;
      while (string[tempI] == ' ') {
        tempI++;
      }
      if (string[tempI] == ')') {
        return true;
      }
    }
  }
  return false;
}

bool isBlank(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    if (string[i] != ' ') {
      return false;
    }
  }
  return true;
}

void checkParenthesesMatch(char *inputString) {
  char copyOfInput[strlen(inputString)];
  strcpy(copyOfInput, inputString);
  for (int i = 0; i < strlen(copyOfInput); i++) {
    switch (copyOfInput[i]) {
      case '+':
      case '-':
      case '*':
      case '/':
      case '^':
      case ' ':
      case '.':
      case '!':
        copyOfInput[i] = ' ';
        break;
      default:
        if (isNumeric(copyOfInput[i])) {
          copyOfInput[i] = ' ';
        }
        break;
    }
  }

  while (hasMatchingBrackets(copyOfInput)) {
    for (int i = 0; i < strlen(copyOfInput); i++) {
      if (i + 1 < strlen(copyOfInput) &&
          copyOfInput[i] == '(') {
        int tempI = i + 1;
        while (copyOfInput[tempI] == ' ') {
          tempI++;
        }
        if (copyOfInput[tempI] == ')') {
          copyOfInput[i] = ' ';
          copyOfInput[tempI] = ' ';
        }
      }
    }
  }

  hadError = hadError || !isBlank(copyOfInput);

  if (!isBlank(copyOfInput)) {
    error("Unmatched brackets.");
  }
}

void checkExpressionValidity(char *exp) {
  for (int index = 0; index < strlen(exp); index++) {
    if (!isValidCharacter(exp[index])) {
      return;
    }
  }
}

bool isValidCharacter(char c) {
  switch (c) {
    case '+':
    case '-':
    case '*':
    case '/':
    case '^':
    case '(':
    case ')':
    case ' ':
    case '.':
    case '!':
    case '\n':
    case '\0':
      return true;
    default:
      if (isNumeric(c)) {
        return true;
      } else {
        char errorMessage[200];
        sprintf(errorMessage, "Invalid character: '%c' (character code: %d)\n", c, c);
        error(errorMessage);
        return false;
      }
  }
}

bool isNumeric(char c) {
  return c >= '0' && c <= '9';
}

void blue() {
  printf("\033[0;34m");
}

void green() {
  printf("\033[0;32m");
}

void purple() {
  printf("\033[0;35m");
}

void beep() {
  printf("\a");
}

void error(char *str) {
  hadError = true;
  printf("\033[1;31m");
  puts(str);
}

Token initToken(const char *val, Symbol ty) {
  int bindingPower;
  switch (ty) {
    case ADD:
    case MINUS:
      bindingPower = 10;
      break;
    case MULTIPLY:
    case DIVIDE:
      bindingPower = 20;
      break;
    case POWER:
      bindingPower = 30;
      break;
    case FACTORIAL:
      bindingPower = 40;
      break;
    default:
      bindingPower = 0;
      break;
  }
  Token returnToken = {val, ty, bindingPower};
  return returnToken;
}


char *lowercase(char *str) {
  for (int i = 0; i < strlen(str); i++) {
    str[i] = (char) tolower(str[i]);
  }
  return str;
}

void printHelpManual() {
  blue();
  puts("Calculator Help Manual");
  puts("----------------------");

  purple();
  puts("SPECIFICATIONS");
  blue();
  puts("- This calculator is accurate up to 9 decimal digits.");
  puts("- You can enter numbers (including decimals)");
  puts("- Operations supported:");
  puts("\t  Operation      Symbol");
  puts("\t- Addition       [+]");
  puts("\t- Subtraction    [-]");
  puts("\t- Multiplication [*]");
  puts("\t- Division       [/]");
  puts("\t- Exponentiation [^]");
  puts("\t- Factorials     [!]");
  puts("\t- Parentheses    [()]");
  purple();
  puts("OTHER COMMANDS");
  blue();
  puts("- You can type any message that contains the word 'help' to summon this help manual.");
  puts("- You can type any message that contains the word 'clear' to clear the console screen.");
  puts("- You can type any message that contains the word 'exit' to exit the program.");

  puts("");
  puts("Hope this helped!");
}