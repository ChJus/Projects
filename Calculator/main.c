// Include libraries
#include <stdio.h>    // I/O functions (fgets(), stdin, sprintf(), printf())
#include <string.h>   // String functions (strcspn(), strstr(), strlen(), strncpy(), strcpy())
#include <stdbool.h>  // Define booleans (bool, true, false)
#include <ctype.h>    // Lowercase character function tolower()
#include <stdlib.h>   // Standard library (system(), strtod(), malloc())
#include <math.h>     // Math library (INFINITY, isnan(), pow())
#include <unistd.h>   // Thread sleep function (usleep())

// Define enumeration containing labels for various different token types
typedef enum {
    NUMBER,                     // Numerical values (RegExp [0-9.])
    ADD, MINUS,                 // Addition [+] and subtraction [-]
    DIVIDE, MULTIPLY,           // Division [/] and multiplication [*]
    POWER, FACTORIAL,           // Exponentiation [^] and factorials [!]
    START_BRACKET, END_BRACKET, // Parentheses [(], [)]
    END_OF_EXPRESSION           // End of user expression
} Symbol;

// Define struct Token
// Tokens hold more information about themselves than the Symbol type.
// Each token has a string value (that represents their value in the user expression),
// type, and binding power. The larger the binding power, the higher the precedence the operator has.
// Note that binding power is only assigned to operator tokens.
// To create a token, call initToken().
typedef struct {
    const char *value;
    Symbol type;
    int bindingPower;
} Token;

// Function prototype declarations.
// Small utility functions
void beep();            // Makes computer play 'beep'
void blue();            // Change text color to blue
void green();           // Change text color to green
void purple();          // Change text color to purple
void error(char *str);  // Change text color to bold red and print an error message, changes hadError to true
void type(char *str);   // Types out a message character by character

// De-clutter main() method (code ported off into a method)
void printHelpManual(); // prints help manual

// Helper functions
char *lowercase(char *str);                    // converts string to lowercase
double factorial(double left);                 // returns factorial of value
Token initToken(const char *val, Symbol ty);   // creates a token and returns to callee
Token advance();                               // advances and returns tokens, used during expression parsing
void resetGlobalVariables();                   // reset global variables to default values
void stripTrailingZeros(char *str);            // remove trailing 0s from result when printing

// Small validation functions
bool isValidCharacter(char c);   // checks if a character is valid, used in checkExpressionValidity()
bool isBlank(char *string);      // checks if a string only contains blank spaces
bool isNumeric(char c);          // checks if character is a digit from 0 to 9

// Validation functions
void checkParenthesesMatch(char *inputString); // checks if parentheses '()' match throughout expression
void checkExpressionValidity(char *exp);       // checks if expression contains *only* valid characters

// Tokenization functions - converts user expression to a list of tokens
void tokenize(char *exp);           // tokenizes user expression
void tokenizeNumber();              // tokenizes a number token
int findNumberOfTokens(char *exp);  // counts the number of tokens in the expression

// Pratt-parsing specific functions
double expression(int bindingPower);       // evaluates expression at current binding power
double led(Token tempToken, double left);  // left-denotation - evaluates binary expressions
double nud(Token tempToken);               // null-denotation - evaluates unary expressions

// Global variables
// Stores whether an error occurred
bool hadError = false;

// Stores the index of the start of a token
int start = 0;

// Stores the index of the currently being parsed character
int current = 0;

// Stores the index of the token that will be parsed
int parseCurrent = 0;

// Token array
// Note the max is 1024 because userExp has a max of 1024, and the max number of tokens
// would therefore be 1024 (one token is at least one character long)
Token tokens[1024];

// Stores the token to be parsed by expression()
Token token;

// Stores the user's expression
char userExp[1024];

int main() {
  // Change text to green and display a short prompt message
  // Note that type() types out the prompt
  green();
  type("Calculator (9 d.p.)\n");
  type("-------------------\n\n");
  type("Type 'help' for help, 'clear' to clear the screen, or 'exit' to exit the program.\n\n");
  type("Supported operations: +, -, *, /, ^, !, ()\n");
  type("Supported characters: [0-9], [.]\n\n");

  // Keep asking for user input until they type a message containing 'exit' or
  // forcefully terminate the program.
  while (true) {
    // Reset all global variables
    // These variables have values that change depending on what the user enters during
    // each iteration of the while loop.
    resetGlobalVariables();

    // Ask for user input in purple
    purple();
    type("> ");

    // Get the user's input as a line including the trailing '\n' character.
    // Note the max is 1024 characters, and the input comes from stdin
    fgets(userExp, 1024, stdin);

    // Remove the trailing newline character
    userExp[strcspn(userExp, "\n")] = 0;

    // Play a beep sound
    beep();

    // If the user inputs a message containing the word 'exit', say goodbye and terminate program.
    if (strstr(lowercase(userExp), "exit") != NULL) {
      blue();
      type("Goodbye!\n");
      return 0;
    } else if (strstr(lowercase(userExp), "clear") != NULL) { // Input message contains 'clear'
      // Clear the screen and ask for new input
      system("clear");
      continue;
    } else if (strstr(lowercase(userExp), "help") != NULL) { // Input message contains 'help'
      // Print help manual and ask for new input
      printHelpManual();
      continue;
    }

    // Check if user expression contains *only* valid characters
    checkExpressionValidity(userExp);

    // If it doesn't, exit the current iteration and ask for input again
    if (hadError) {
      continue;
    }

    // Check if user expression has matching parentheses (e.g., parentheses are in pairs)
    checkParenthesesMatch(userExp);

    // If the expression doesn't have matching parentheses, then exit the current iteration
    // and ask for user input again.
    if (hadError) {
      continue;
    }

    // Tokenize the user's expression
    tokenize(userExp);

    // If the user's expression doesn't have any tokens (it is empty), then ask for input again.
    if (findNumberOfTokens(userExp) == 0) {
      continue;
    }

    // Let token be the first token in the list of tokens
    token = tokens[parseCurrent];

    // Start the parsing section with binding power 0.
    // After recursing through the entire expression, the final result will be in here.
    double result = expression(0);

    // If the result is ±∞, notify the user
    if (result == INFINITY || result == -INFINITY) {
      error("Result reached ±∞.");
      error("Hint: this may be because of double factorials (e.g., '5!!'), exponentiation or divide by 0.");
    } else if (isnan(result)) { // If the result is NaN, notify the user
      error("Result is not a number.");
      error("Hint: this may be because of divide by 0.");
      error("Hint: this may be because result is imaginary or complex.");
    } else if (!hadError) { // If no error occurred, then print the result up to 9 d.p.
      // Final result string will be at most 1024 characters
      // This won't be reached because double value range is <1E1024
      char resultString[1024];

      // Format the string to 9 d.p.
      // Note that whole numbers and numbers that fit in less than 9 d.p. are also formatted
      // into 9 d.p. (by adding trailing 0s)
      sprintf(resultString, "%.9f", result);

      // Call a function that removes the trailing 0s.
      stripTrailingZeros(resultString);

      // Type the final result
      blue();
      type(resultString);
      type("\n\n");
    }
  }
}

// Removes trailing 0s from a string representing a number
void stripTrailingZeros(char *str) {
  int index = 0;

  // Ignore all digits before the decimal point '.'
  while (str[index] != '.' && index + 1 < strlen(str)) {
    index++;
  }

  // Iterate through the digits after the decimal.
  // Keep checking if digits after the digit at indice are all 0.
  // If so, then remove the 0s.
  for (int indice = index; indice < strlen(str); indice++) {
    bool allAfterAreZeros = true;
    for (int ind = indice + 1; ind < strlen(str); ind++) {
      if (str[ind] != '0') {
        allAfterAreZeros = false;
      }
    }
    if (allAfterAreZeros) {
      if (indice == index) {
        str[indice] = '\0';
      }
      for (int ind = indice + 1; ind < strlen(str); ind++) {
        str[ind] = '\0';
      }
    }
  }
}

// Set global variables to default values
void resetGlobalVariables() {
  hadError = false;
  parseCurrent = 0;
  current = 0;
  start = 0;
  token = initToken("", END_OF_EXPRESSION);
}

// Left denotation - evaluates binary expressions
double led(Token tempToken, double left) {
  // When expression() calls this, note that it will have already
  // consumed the left operand and operator.
  switch (tempToken.type) { // Check the type of the operator
    case ADD: // Addition
      return left + expression(10);
    case MINUS: // Subtraction
      return left - expression(10);
    case MULTIPLY: // Multiplication
      return left * expression(20);
    case DIVIDE: // Division
      return left / expression(20);
    case POWER: // Exponentiation
      // Note how the binding power is 30 - 1 not 30.
      // This is because exponents are right-associative, so exponents on the rightmost
      // need to be evaluated first (thus having higher precedence than binding power 29)
      return pow(left, expression(30 - 1));
    case FACTORIAL: // Factorials
      return factorial(left);
    default: // This should never happen, but if it does, handle the error.
      error("Unable to parse expression.");
      return 0;
  }
}

// Hand-implemented factorial calculator
// Note it is iterative not recursive
// Recursion has some overhead and also risks stack overflow.
// Iterative is faster but less elegant.
double factorial(double left) {
  double result = 1;
  for (int i = 1; i <= left; i++) {
    result *= i;
  }
  return result;
}

// Null denotation - evaluates unary expressions
double nud(Token tempToken) {
  switch (tempToken.type) { // Check the type of the token
    case NUMBER: // if it is a number, simply parse token.value into a double
      return strtod(tempToken.value, NULL);
    case MINUS: // Negation
      // Note that negation has higher precedence than subtraction, and therefore
      // the binding power is higher.
      return -expression(25);
    case START_BRACKET: { // Evaluate expressions in parentheses
      double val = expression(0);
      if (tokens[parseCurrent].type != END_BRACKET) {
        error("Expected ending bracket ')'.");
      }
      token = advance(); // consume the ')' ending parentheses
      return val; // return the result of the expression in the parentheses
    }
    case END_BRACKET: // Handles expression '()'
      error("Unnecessary brackets: there isn't anything in the brackets.");
    default: { // Only happens in invalid (syntax-wise) expressions
      char errorMessage[200];
      sprintf(errorMessage, "Unexpected token '%s'.", tempToken.value);
      error(errorMessage);
      return 0;
    }
  }
}

// Return the next token
Token advance() {
  if (parseCurrent + 1 < findNumberOfTokens(userExp)) {
    return tokens[++parseCurrent];
  } else {
    // If there are no more tokens, return a token that indicates the end of the expression
    return initToken("", END_OF_EXPRESSION);
  }
}

// Evaluate user expression
// Note that this function is recursive
// Algorithm detailed in DF document
double expression(int bindingPower) {
  // Consume an operand
  Token t = token;
  token = advance();

  // Evaluate the operand as a unary expression
  double left = nud(t);

  // If the binding power currently is smaller than the binding power
  // of the next token, keep looping
  while (bindingPower < token.bindingPower) {
    // Consume tokens
    t = token;
    token = advance();

    // Evaluate binary expression
    left = led(t, left);
  }

  // Return result to callee
  return left;
}

// Tokenize a string expression into an array of tokens
void tokenize(char *exp) {
  int indexToken = 0;
  start = 0;
  current = 0;

  // Go through entire expression character by character
  while (current < strlen(exp)) {
    char c = exp[current];
    if (isNumeric(c)) { // Consume number if program reads a digit
      tokenizeNumber();
      char *sub = malloc(strlen(exp));
      strncpy(sub, exp + start, current + 1 - start);
      tokens[indexToken++] = initToken(sub, NUMBER);
      current++;
      start = current;
      continue; // Continue onwards to the next iteration
    }

    // Check character type if it is not a number
    switch (c) {
      case ' ':  // spaces are ignored
      case '\t': // tabs are ignored
        break;
      case '(': // consume '(' as start parentheses
        tokens[indexToken++] = initToken("(", START_BRACKET);
        break;
      case ')': // consume ')' as end parentheses
        tokens[indexToken++] = initToken(")", END_BRACKET);
        break;
      case '+': // '+' - addition
        tokens[indexToken++] = initToken("+", ADD);
        break;
      case '-': // '-' - subtraction/negation
        tokens[indexToken++] = initToken("-", MINUS);
        break;
      case '*': // '*' - multiplication
        tokens[indexToken++] = initToken("*", MULTIPLY);
        break;
      case '/': // '/' - division
        tokens[indexToken++] = initToken("/", DIVIDE);
        break;
      case '^': // '^' - exponentiation
        tokens[indexToken++] = initToken("^", POWER);
        break;
      case '!': // '!' - factorial
        tokens[indexToken++] = initToken("!", FACTORIAL);
        break;
      case '.': // '.' - unexpected as we handle '.' in numbers in the tokenizeNumber() function
        error("Error: Unexpected '.', please have digits before '.' (e.g., 0.1 instead of .1)");
        error("       Also, numbers can only have one '.' (e.g., no 1.1.1)\n");
        break;
      default: {
        // Unknown characters are reported (shouldn't happen as it
        // happens already in checkExpressionValidity())
        char errorMessage[200];
        sprintf(errorMessage, "Error: Unknown character: '%c'\n", c);
        error(errorMessage);
        break;
      }
    }
    current++;
    start = current; // end of token, the start of the next token must be the next character
  }
}

// Finds the number of tokens in a string expression
// Note that this function is very similar to the tokenize() function,
// just that it increments a counter instead of creating tokens.
int findNumberOfTokens(char *exp) {
  start = 0;
  current = 0;

  int numTokens = 0;

  // Iterate through the expression
  while (current < strlen(exp)) {
    char c = exp[current];
    if (isNumeric(c)) { // +1 for numbers
      tokenizeNumber();
      numTokens++;
      current++;
      start = current;
      continue;
    }
    switch (c) { // +1 for each of these characters
      // Notice the chaining of cases.
      // Each of these cases below should increment token count by 1.
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
        // Spaces, etc. aren't tokens, so ignore them
      default:
        break;
    }
    current++;
    start = current;
  }
  return numTokens;
}

// Tokenize a number
void tokenizeNumber() {
  // Consume numeric part
  // If we reach the end of the expression or the next character isn't numeric, stop
  while (current + 1 < strlen(userExp) && (isNumeric(userExp[current + 1]))) {
    current++;
  }

  // If the next character is '.', expect to see more numbers afterwards
  if (current + 1 < strlen(userExp) && userExp[current + 1] == '.') {
    current++; // consume the '.'
    // Consume numbers afterwards
    while (current + 1 < strlen(userExp) && isNumeric(userExp[current + 1])) {
      current++;
    }
  }
  // Note that no token is created here, it is created back in the function tokenize().
}

// Checks whether a string still has a pair of parentheses '()'
// Note that blank spaces are ignored
int hasMatchingBrackets(char *string) {
  for (int i = 0; i < strlen(string); i++) { // For each character in the string
    if (i + 1 < strlen(string) && // Ensure that there is a character after the current one
        string[i] == '(') {  // Check if the current character is '('
      int tempI = i + 1;
      while (string[tempI] == ' ') { // While it is only spaces, advance
        tempI++;
      }
      if (string[tempI] == ')') {
        // Return true if we see a ')' next
        // If not, then start scanning again from the next character
        // This function only returns true when the innermost pair of parentheses are found.
        return true;
      }
    }
  }
  // Return false if no more matching parentheses are found
  return false;
}

// If the entire string is just spaces, then return true.
// Else, the string contains some other character, return false.
bool isBlank(char *string) {
  for (int i = 0; i < strlen(string); i++) {
    if (string[i] != ' ') {
      return false;
    }
  }
  return true;
}

// Validates that parentheses are matching in the expression
void checkParenthesesMatch(char *inputString) {
  // Make a copy of the input string because we are performing temporary functions on it.
  char copyOfInput[strlen(inputString)];
  strcpy(copyOfInput, inputString);

  // For each character of the string
  for (int i = 0; i < strlen(copyOfInput); i++) {
    switch (copyOfInput[i]) {
      // Remove non-parentheses characters
      case '+':
      case '-':
      case '*':
      case '/':
      case '^':
      case ' ':
      case '\t':
      case '.':
      case '!':
        copyOfInput[i] = ' ';
        break;
      default: // Remove numbers as well
        if (isNumeric(copyOfInput[i])) {
          copyOfInput[i] = ' ';
        }
        break;
    }
  }

  // While there are still matching brackets
  while (hasMatchingBrackets(copyOfInput)) {
    // Go through and replace them with spaces
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

  // If hadError isn't already true, set it to be true if the result string is not just a bunch of spaces
  hadError = hadError || !isBlank(copyOfInput);

  // If there are still parentheses in the string, then
  // the parentheses are unmatched, an error is thrown.
  if (!isBlank(copyOfInput)) {
    error("Unmatched parentheses.");
  }
}

// Check if the expression contains only valid characters
void checkExpressionValidity(char *exp) {
  for (int index = 0; index < strlen(exp); index++) {
    // Iterate through the expression and check if each character is valid
    // Technically the loop can exit after one invalid character is found,
    // but we want to spot as many (valid) errors as possible before asking for input again.
    // That way, if the user is trying to fix the expression, they can fix multiple errors at once.
    isValidCharacter(exp[index]);
  }
}

// Checks if a single character is valid
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
    case '\t':
    case '\0':
      return true;
    default: // If it is a number, also valid
      if (isNumeric(c)) {
        return true;
      } else { // If not, throw an error
        char errorMessage[200];
        sprintf(errorMessage, "Invalid character: '%c' (character code: %d)\n", c, c);
        error(errorMessage);
        return false;
      }
  }
}

// Checks if character is digit from 0 to 9
bool isNumeric(char c) {
  return c >= '0' && c <= '9';
}

// Print in blue
void blue() {
  printf("\033[0;34m");
}

// Print in green
void green() {
  printf("\033[0;32m");
}

// Print in purple
void purple() {
  printf("\033[0;35m");
}

// Play beep sound
void beep() {
  printf("\a");
}

// Print errors in bold red and set the hadError flag on.
void error(char *str) {
  hadError = true;
  printf("\033[1;31m");
  type(str);
  type("\n");
}

// Return an initialized token
Token initToken(const char *val, Symbol ty) {
  // The main purpose of this function is to assign binding power to operators.
  int bindingPower;
  switch (ty) { // Check the type of the token to be created
    // Assign binding powers based on which operator it is
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
    default: // If it isn't an operator, assign a default binding power value of 0.
      bindingPower = 0;
      break;
  }
  // Return the new token to the callee
  Token returnToken = {val, ty, bindingPower};
  return returnToken;
}

// Returns the lowercase version of a string
char *lowercase(char *str) {
  // Go through each character in the string and make it lowercase
  for (int i = 0; i < strlen(str); i++) {
    str[i] = (char) tolower(str[i]);
  }

  // Return the string
  // Note that the string itself is also modified
  return str;
}

// Prints out a more extensive version of the starting prompt.
// This help manual is also color coded (headings in purple, text in blue)
void printHelpManual() {
  blue();
  type("Calculator Help Manual\n");
  type("----------------------\n");

  purple();
  type("SPECIFICATIONS\n");
  blue();
  type(" - This calculator is accurate up to 9 decimal digits.\n");
  type(" - You can enter expressions that are \033[1;34mat most 1024 characters long\033[0;34m.\n");
  type(" - You can enter numbers (including decimals)\n");
  type(" - Operations supported:\n");
  type("\t  Operation      Symbol\n");
  type("\t- Addition       [+]\n");
  type("\t- Subtraction    [-]\n");
  type("\t- Multiplication [*]\n");
  type("\t- Division       [/]\n");
  type("\t- Exponentiation [^]\n");
  type("\t- Factorials     [!]\n");
  type("\t- Parentheses    [()]\n");

  purple();
  type("OTHER COMMANDS\n");
  blue();
  type(" - You can type any message that contains the word 'help' to summon this help manual.\n");
  type(" - You can type any message that contains the word 'clear' to clear the console screen.\n");
  type(" - You can type any message that contains the word 'exit' to exit the program.\n");

  type("Hope this helped!\n");
}

// Typing function
void type(char *str) {
  // Find the amount of milliseconds to delay printing each character by.
  // Note that strings will be typed out at different speeds depending on their length.
  // Each string will take ~500 milliseconds to type out
  int timeInMs = (int) (500.0l / strlen(str));

  // If the string is very short, type it out in
  // less than 300 milliseconds
  if (strlen(str) < 10) {
    timeInMs = 30;
  }

  // Type the string with delay
  // See DF design brief: How can you display text with delay?
  for (int i = 0; i < strlen(str); i++) {
    // Sleep the thread for timeInMs * 1000 microseconds
    // (1 μs = 1000 ns)
    usleep(timeInMs * 1000);
    printf("%c", str[i]);

    // Flush it onto the console
    fflush(stdout);
  }
}
