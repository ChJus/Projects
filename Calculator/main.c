// Include libraries
#include <stdio.h>    // I/O functions (fgets(), stdin, sprintf(), printf())
#include <string.h>   // String functions (strcspn(), strstr(), strlen(), strncpy(), strcpy())
#include <stdbool.h>  // Define booleans (bool, true, false)
#include <ctype.h>    // Lowercase character function tolower()
#include <stdlib.h>   // Standard library (system(), strtod(), malloc())
#include <math.h>     // Math library (INFINITY, isnan(), pow())

#if defined(WIN32)        // Add support for thread sleeping in Windows
#include <windows.h>
#else
#include <unistd.h>     // Thread sleep function (usleep())
#endif

#ifndef M_PI // Define pi if not defined previously in math.h header
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E // Define e if not defined previously in math.h header
#define M_E 2.71828182845904523536
#endif

// Define enumeration containing labels for various different token types
typedef enum Symbol {
    NUMBER,                     // Numerical values (RegExp [0-9.])
    ADD, MINUS,                 // Addition [+] and subtraction [-]
    DIVIDE, MULTIPLY,           // Division [/] and multiplication [*]
    MODULO,                     // Modulo (%)
    POWER, FACTORIAL,           // Exponentiation [^] and factorials [!]
    MATH_PI, MATH_E,            // π and exp
    RAND_NUM,                   // obtain random number
    SQRT,                       // performs square root ('sqrt')
    CBRT,                       // performs cube root ('cbrt')
    LOG,                        // performs logarithm ('log')
    LN,                         // performs natural logarithm ('ln')
    SIN, COS, TAN,              // performs trigonometric functions ('sin', 'cos', 'tan') IN DEGREES
    ASIN, ACOS, ATAN,           // performs inverse trigonometric functions ('asin', 'acos', 'atan')
    SINH, COSH, TANH,           // performs hyperbolic trigonometric functions ('sinh', 'cosh', 'tanh')
    ASINH, ACOSH, ATANH,        // performs inverse hyperbolic trigonometric functions ('asinh', 'acosh', 'atanh')
    ABS,                        // performs absolute value ('abs')
    DEGTORAD, RADTODEG,         // performs conversion between degrees and radians ('degtorad', 'radtodeg')
    FLOOR, CEIL, ROUND,         // performs floor, ceil and round functions ('floor', 'ceil', 'round')
    PASS_TOKEN,                 // Ignore this token space
    START_BRACKET, END_BRACKET, // Parentheses [(], [)]
    IDENTIFIER,                 // Function/constant names
    END_OF_EXPRESSION           // End of user expression
} Symbol;

// Define struct Token
// Tokens hold more information about themselves than the Symbol type.
// Each token has a string value (that represents their value in the user expression),
// type, and binding power. The larger the binding power, the higher the precedence the operator has.
// Note that binding power is only assigned to operator tokens.
// To create a token, call initToken().
typedef struct Token {
    const char *value;
    Symbol type;
    int bindingPower;
} Token;

// Function prototype declarations.
// Small utility functions
void beep();            // Makes computer play 'beep'
void blue();            // Change text color to blue/cyan
void boldBlue();        // Change text color to bold blue/cyan
void green();           // Change text color to green
void boldRed();         // Change text color to bold red for error messages
void purple();          // Change text color to purple
void error(char *str, int index);  // Change text color to bold red and print an error message, changes hadError to true
void type(const char *str);   // Types out a message character by character

// De-clutter main() method (code ported off into a method)
void printHelpManual(); // prints help manual
bool evaluateExpression(); // evaluates expression stored in userExp

// Helper functions
char *lowercase(char *str);                    // converts string to lowercase
double factorial(double left);                 // returns factorial of value
double integerFactorial(double left);          // returns factorial of integer ≥0
double spouge(double z);                       // implementation of Spouge approximation for factorials
Token initToken(const char *val, Symbol ty);   // creates a token and returns to callee
Token advance();                               // advances and returns tokens, used during expression parsing
void resetGlobalVariables();                   // reset global variables to default values
void stripTrailingZeros(char *str);            // remove trailing 0s from result when printing
void stripTrailingZerosScientificNotation(char *str); // remove trailing 0s in result expressed in scientific notation.
bool match(const char *value, char *anotherValue);   // checks if two values match
void omitToken(int index);                     // removes token at index + 1 from expression
double degtorad(double degrees);                     // converts degrees to radians
double radtodeg(double radians);                     // converts radians to degrees

// Small validation functions
bool isBlank(char *string);      // checks if a string only contains blank spaces
bool isNumeric(char c);          // checks if character is a digit from 0 to 9
bool isAlpha(char c);            // checks if character is alphabet from a to z

// Validation functions
void checkParenthesesMatch(char *inputString); // checks if parentheses '()' match throughout expression
void checkExpressionValidity();                // checks if expression contains *only* valid characters

// Tokenization functions - converts user expression to a list of tokens
void tokenize(char *exp);           // tokenizes user expression
void tokenizeNumber();              // tokenizes a number token
void tokenizeAlpha();               // tokenizes an identifier for a constant/function
void tokenizeFunction(int index, const char *tempTokenValue); // tokenizes a function token
int findNumberOfTokens(char *exp);  // counts the number of tokens in the expression

// Pratt-parsing specific functions
double expression(int bindingPower);       // evaluates expression at current binding power
double led(Token tempToken, double left);  // left-denotation - evaluates binary expressions
double nud(Token tempToken);               // null-denotation - evaluates unary expressions

// Global variables
// Stores whether an error occurred
bool hadError = false;

// Stores which stage currently in when error occurred
// false - before tokenize stage or during parsing stage
// true - during tokenize stage
bool inTokenizeStage = false;

// Stores the number of errors
// If there are more than 5 errors, the rest are omitted.
int numErrors = 0;

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

int main(int argc, char **argv) {
  if (argc == 1) {
    // If no expression is given directly in command line run, ask for user input
    // Change text to green and display a short prompt message
    // Note that type() types out the prompt
    green();
    type("Calculator (9 d.p.)\n");
    type("-------------------\n\n");
    type("Commands:\n");
    type("\t'help': help manual\n");
    type("\t'clear': clear screen\n");
    type("\t'exit': exit program\n\n");
    type("Supported operations: +, -, *, /, %, ^, !, ()\n");
    type("Supported characters: [0-9], [.], pi, exp (2.718...), and others (see help manual)\n\n");

    // Keep asking for user input until they type a message containing 'exit' or
    // forcefully terminate the program.
    do {
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

    } while (evaluateExpression());
  } else {
    // Get input from command line arguments
    // Concatenate all arguments into a single string (as each
    // argument is delimited by a ' ')
    for (int i = 1; i < argc; i++) {
      strcat(userExp, argv[i]);
      strcat(userExp, " ");
    }
    evaluateExpression();
  }
}

bool evaluateExpression() {
  inTokenizeStage = true;

  // If the user inputs a message containing the word 'exit', say goodbye and terminate program.
  if (strstr(lowercase(userExp), "exit") != NULL) {
    blue();
    type("Goodbye!\n");
    return false;
  } else if (strstr(lowercase(userExp), "clear") != NULL) { // Input message contains 'clear'
    // Clear the screen and ask for new input
    system("clear");
    return true;
  } else if (strstr(lowercase(userExp), "help") != NULL) { // Input message contains 'help'
    // Print help manual and ask for new input
    printHelpManual();
    return true;
  }

  // Check if user expression has matching parentheses (e.g., parentheses are in pairs)
  checkParenthesesMatch(userExp);

  // If the expression doesn't have matching parentheses, then exit the current iteration
  // and ask for user input again.
  if (hadError) {
    return true;
  }

  // Tokenize the user's expression
  tokenize(userExp);

  // Check tokens are valid
  checkExpressionValidity();

  // If they aren't, exit the current iteration and ask for input again
  if (hadError) {
    return true;
  }

  // If the user's expression doesn't have any tokens (it is empty), then ask for input again.
  // If an error occurred during tokenization, don't try to parse it.
  if (findNumberOfTokens(userExp) == 0 || hadError) {
    return true;
  }

  // Let token be the first token in the list of tokens
  token = tokens[parseCurrent];

  // Start the parsing section with binding power 0.
  // After recursing through the entire expression, the final result will be in here.
  inTokenizeStage = false;
  double result = expression(0);

  if (!hadError) {
    // If the result is ±∞, notify the user
    if (result == INFINITY || result == -INFINITY) {
      error("Result reached positive/negative infinity.", -1);
      error("Hint: this may be because of double factorials (e.g., '5!!'), exponentiation or divide by 0.", -1);
    } else if (isnan(result)) { // If the result is NaN, notify the user
      error("Result is not a number.", -1);
      error("Hint: this may be because of divide by 0.", -1);
      error("Hint: this may be because result is imaginary or complex.", -1);
    } else { // If no error occurred, then print the result up to 9 d.p.
      // Final result string will be at most 1024 characters
      // This won't be reached because double value range is <1E1024
      char resultString[1024];

      if (result > 1e16 || result < -1e16 || (result > -1e-16 && result < 1e-16 && result != 0)) {
        // If the result is bigger than 1e16 or less than -1e16,
        // or the result is between -1e-16 and 1e-16,
        // express the result in approximated scientific notation, as
        // C floating-point arithmetic isn't very accurate in these ranges.
        sprintf(resultString, "%.9e", result);

        // Remove unnecessary 0s in scientific notation
        stripTrailingZerosScientificNotation(resultString);
      } else {
        // Format the string to 9 d.p.
        // Note that whole numbers and numbers that fit in less than 9 d.p. are also formatted
        // into 9 d.p. (by adding trailing 0s)
        sprintf(resultString, "%.9f", result);

        // Call a function that removes the trailing 0s.
        stripTrailingZeros(resultString);
      }

      // Type the final result
      blue();
      type(resultString);
      type("\n\n");
    }
  }
  return true;
}

void stripTrailingZerosScientificNotation(char *str) {
  int maxIndex = 0;

  // Find stopping point (when character is 'e')
  while (str[maxIndex] != 'e' && maxIndex + 1 < strlen(str)) {
    maxIndex++;
  }

  // Preserve the 'e____' part (10^____) that will be concatenated afterwards.
  char *magnitude = malloc(strlen(str));
  strncpy(magnitude, str + maxIndex, strlen(str) - maxIndex);

  // Find where the decimal point '.' is (if there is one)
  int decimalIndex = -1;

  for (int i = 0; i < strlen(str); i++) {
    if (str[i] == '.') {
      decimalIndex = i;
      break;
    }
  }

  // Iterate through the digits after the decimal.
  // Keep checking if digits after the digit at indice are all 0.
  // If so, then remove the 0s.
  for (int indice = 0; indice < maxIndex; indice++) {
    bool allAfterAreZeros = true;
    for (int ind = indice + 1; ind < maxIndex; ind++) {
      if (str[ind] != '0') {
        allAfterAreZeros = false;
      }
    }
    if (allAfterAreZeros) {
      if (indice == decimalIndex) {
        str[indice] = '\0';
      }
      for (int ind = indice + 1; ind < strlen(str); ind++) {
        str[ind] = '\0';
      }
    }
  }

  // add e____ part back to string
  strcat(str, magnitude);
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
  inTokenizeStage = false;
  numErrors = 0;
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
    case MODULO: // Modulo
      return fmod(left, expression(20));
    case POWER: // Exponentiation
      // Note how the binding power is 30 - 1 not 30.
      // This is because exponents are right-associative, so exponents on the rightmost
      // need to be evaluated first (thus having higher precedence than binding power 29)
      return pow(left, expression(30 - 1));
    case FACTORIAL: // Factorials
      return factorial(left);
    default: // This should never happen, but if it does, handle the error.
      error("Unable to parse expression.", parseCurrent);
      return 0;
  }
}

// Performs factorial on integer values ≥0
double integerFactorial(double left) {
  double result = 1;
  for (int i = 1; i <= left; i++) {
    result *= i;
  }
  return result;
}

// Implementation of Spouge approximation of factorials
// See https://en.wikipedia.org/wiki/Spouge%27s_approximation
// Note: ε_a(z) was discarded, error because of discarding the term is small.
double spouge(double z) {
  int a = 15;
  double result = pow(z + a, z + 0.5) * pow(M_E, -(z + a));
  double prodValue = sqrt(2 * M_PI);
  for (int k = 1; k <= a - 1; k++) {
    prodValue += ((pow(-1, k - 1) / (integerFactorial(k - 1))) * pow(-k + a, k - 0.5) * pow(M_E, -k + a)) / (z + k);
  }
  result *= prodValue;
  return result;
}

// Hand-implemented factorial calculator
double factorial(double left) {
  if (left < 0) {
    error("Factorial is only defined for non-negative numbers.", parseCurrent);
    return 0;
  }

  double result;
  if (floor(left) == left) {
    result = integerFactorial(left);
  } else {
    result = spouge(left);
  }
  return result;
}

// Null denotation - evaluates unary expressions
double nud(Token tempToken) {
  switch (tempToken.type) { // Check the type of the token
    case NUMBER: // if it is a number, simply parse token.value into a double
      return strtod(tempToken.value, NULL);
    case MATH_PI: // π
      return M_PI;
    case MATH_E:  // exp
      return M_E;
    case RAND_NUM: // returns random number
      return ((double) rand()) / RAND_MAX;
    case MINUS: // Negation
      // Note that negation has higher precedence than subtraction, and therefore
      // the binding power is higher.
      return -expression(25);
    case START_BRACKET: { // Evaluate expressions in parentheses
      double val = expression(0);
      if (tokens[parseCurrent].type != END_BRACKET) {
        error("Expected ending bracket ')'.", parseCurrent);
      }
      token = advance(); // consume the ')' ending parentheses
      return val; // return the result of the expression in the parentheses
    }
    case END_BRACKET: // Handles expression '()'
      error("Parsed unexpected ')' token.", parseCurrent);
    case SQRT: { // Square root
      return sqrt(expression(40));
    }
    case CBRT: { // Cube root
      return cbrt(expression(40));
    }
    case LOG: { // Log base 10
      return log10(expression(40));
    }
    case LN: { // Log base e
      return log(expression(40));
    }
    case SIN: { // Sine
      return sin(degtorad(expression(40)));
    }
    case COS: { // Cosine
      return cos(degtorad(expression(40)));
    }
    case TAN: { // Tangent
      return tan(degtorad(expression(40)));
    }
    case ASIN: { // Inverse sine
      return radtodeg(asin(expression(40)));
    }
    case ACOS: { // Inverse cosine
      return radtodeg(acos(expression(40)));
    }
    case ATAN: { // Inverse tangent
      return radtodeg(atan(expression(40)));
    }
    case SINH: { // Hyperbolic sine
      return radtodeg(sinh(degtorad(expression(40))));
    }
    case COSH: { // Hyperbolic cosine
      return radtodeg(cosh(degtorad(expression(40))));
    }
    case TANH: { // Hyperbolic tangent
      return radtodeg(tanh(degtorad(expression(40))));
    }
    case ASINH: { // Inverse hyperbolic sine
      return radtodeg(asinh(degtorad(expression(40))));
    }
    case ACOSH: { // Inverse hyperbolic cosine
      return radtodeg(acosh(degtorad(expression(40))));
    }
    case ATANH: { // Inverse hyperbolic tangent
      return radtodeg(atanh(degtorad(expression(40))));
    }
    case ABS: { // Absolute value
      return fabs(expression(40));
    }
    case FLOOR: { // Floor
      return floor(expression(40));
    }
    case CEIL: { // Ceiling
      return ceil(expression(40));
    }
    case ROUND: { // Round
      return round(expression(40));
    }
    case DEGTORAD: { // Degrees to radians
      return degtorad(expression(40));
    }
    case RADTODEG: { // Radians to degrees
      return radtodeg(expression(40));
    }
    default: { // Only happens in invalid (syntax-wise) expressions
      char errorMessage[1024];
      sprintf(errorMessage, "Unexpected token '%s'.", tempToken.value);
      error(errorMessage, parseCurrent);
      return 0;
    }
  }
}

double degtorad(double degrees) {
  return degrees * M_PI / 180.0;
}

double radtodeg(double radians) {
  return radians * 180.0 / M_PI;
}

// Return the next token
Token advance() {
  while (parseCurrent + 1 < findNumberOfTokens(userExp)) {
    if (tokens[parseCurrent + 1].type != PASS_TOKEN) {
      return tokens[++parseCurrent];
    } else {
      parseCurrent++;
    }
  }

  // If there are no more tokens, return a token that indicates the end of the expression
  return initToken("", END_OF_EXPRESSION);
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

  // Throw exception for input like '1 1'
  if (t.type == NUMBER && token.type == NUMBER) {
    error("Not expecting a number after a number (with no valid operator in between).", parseCurrent);
  }

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
      if (indexToken - 1 >= 0 && (tokens[indexToken - 1].type == END_BRACKET ||
                                  tokens[indexToken - 1].type == FACTORIAL ||
                                  tokens[indexToken - 1].type == IDENTIFIER)) { // If we have a number after ')'
        tokens[indexToken++] = initToken("*", MULTIPLY);
      }

      tokenizeNumber();
      char *sub = malloc(strlen(exp));
      strncpy(sub, exp + start, current + 1 - start);

      tokens[indexToken++] = initToken(sub, NUMBER);
      current++;
      start = current;
      continue; // Continue onwards to the next iteration
    } else if (isAlpha(c)) {
      if (indexToken - 1 >= 0 && (tokens[indexToken - 1].type == END_BRACKET ||
                                  tokens[indexToken - 1].type == FACTORIAL ||
                                  tokens[indexToken - 1].type == NUMBER ||
                                  tokens[indexToken - 1].type == IDENTIFIER)) {
        tokens[indexToken++] = initToken("*", MULTIPLY);
      }

      tokenizeAlpha();
      char *sub = malloc(strlen(exp));
      strncpy(sub, exp + start, current + 1 - start);

      tokens[indexToken++] = initToken(sub, IDENTIFIER);
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
        // If we have a number before '('
        // Or we have a factorial term before '(' (e.g., '5!(4)')
        if (indexToken - 1 >= 0 &&
            (tokens[indexToken - 1].type == NUMBER || tokens[indexToken - 1].type == FACTORIAL ||
             tokens[indexToken - 1].type == IDENTIFIER)) {
          tokens[indexToken++] = initToken("*", MULTIPLY);
        }
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
      case '%': // '%' - modulo
        tokens[indexToken++] = initToken("%", MODULO);
        break;
      case '^': // '^' - exponentiation
        tokens[indexToken++] = initToken("^", POWER);
        break;
      case '!': // '!' - factorial
        tokens[indexToken++] = initToken("!", FACTORIAL);
        break;
      case '.': // '.' - unexpected as we handle '.' in numbers in the tokenizeNumber() function
        error("Error: Unexpected '.', please have digits before '.' (e.g., 0.1 instead of .1)", -1);
        error("       Also, numbers can only have one '.' (e.g., no 1.1.1)", current);
        break;
      default: {
        // Unknown characters are reported (shouldn't happen as it
        // happens already in checkExpressionValidity())
        char errorMessage[1024];
        sprintf(errorMessage, "Error: Unknown character: '%c'", c);
        error(errorMessage, current);
        break;
      }
    }
    current++;
    start = current; // end of token, the start of the next token must be the next character
  }
}

// Finds the number of tokens in a string expression
// Note that this function is very similar to the 'tokenize()' function,
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

      // Ignore whitespace afterwards
      while (current + 1 < strlen(exp) && (exp[current + 1] == ' ' || exp[current + 1] == '\t')) {
        current++;
      }
      // If we have an identifier after a number, then we will insert
      // a multiplication operator there, so make sure to increment
      // the token count.
      if (current + 1 < strlen(exp) && isAlpha(exp[current + 1])) {
        numTokens += 2;
      } else {
        numTokens++;
      }

      current++;
      start = current;
      continue;
    } else if (isAlpha(c)) {
      tokenizeAlpha();

      // Ignore whitespace afterwards
      while (current + 1 < strlen(exp) && (exp[current + 1] == ' ' || exp[current + 1] == '\t')) {
        current++;
      }

      // If we have a number after the identifier, then we will insert
      // a multiplication operator there, so make sure to increment
      // the token count.
      if (current + 1 < strlen(exp) && (isNumeric(exp[current + 1]) || isAlpha(exp[current + 1]))) {
        numTokens += 2;
      } else {
        numTokens++;
      }

      current++;
      start = current;
      continue;
    }
    switch (c) { // +1 for each of these characters
      // Notice the chaining of cases.
      // Each of these cases below should increment token count by 1.
      case '(': {
        int tempCurrent = current;
        // Ignore whitespace before
        while (exp[tempCurrent - 1] == ' ' || exp[tempCurrent - 1] == '\t') {
          tempCurrent--;
        }
        // If we have a number in front of the '(', or a factorial, or an identifier,
        // then we will insert a multiplication operator there,
        // so make sure to increment the token count.
        if (tempCurrent - 1 >= 0 &&
            (isNumeric(exp[tempCurrent - 1]) || exp[tempCurrent - 1] == '!' || isAlpha(exp[tempCurrent - 1]))) {
          numTokens += 2;
        } else {
          numTokens++;
        }
        break;
      }
      case ')':
        // Ignore whitespace afterwards
        while (current + 1 < strlen(exp) && (exp[current + 1] == ' ' || exp[current + 1] == '\t')) {
          current++;
        }
        // If we have a number after ')', then we will insert
        // a multiplication operator there, so make sure to increment
        // the token count.
        if (current + 1 < strlen(exp) && (isNumeric(exp[current + 1]) || isAlpha(exp[current + 1]))) {
          numTokens += 2;
        } else {
          numTokens++;
        }
        break;
      case '!':
        // Ignore whitespace afterwards
        while (current + 1 < strlen(exp) && (exp[current + 1] == ' ' || exp[current + 1] == '\t')) {
          current++;
        }
        // If we have a number after '!', then we will insert
        // a multiplication operator there, so make sure to increment
        // the token count.
        if (current + 1 < strlen(exp) && isNumeric(exp[current + 1])) {
          numTokens += 2;
        } else {
          numTokens++;
        }
        break;
      case '+':
      case '-':
      case '*':
      case '/':
      case '%':
      case '^':
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

// Tokenize a function/constant identifier
void tokenizeAlpha() {
  // Consume alphabetical characters
  // If we reach the end of the expression or the next character isn't alphabetical, stop
  while (current + 1 < strlen(userExp) && (isAlpha(userExp[current + 1]))) {
    current++;
  }

  // Note that no token is created here, it is created back in the function tokenize().
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
    switch (copyOfInput[i]) { // Remove non-parentheses characters
      case '(':
      case ')':
        continue;
      default: // Remove numbers as well
        copyOfInput[i] = ' ';
        break;
    }
  }

  // While there are still matching brackets '()'
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
          copyOfInput[i] = ' ';     // Remove '('
          copyOfInput[tempI] = ' '; // Remove ')'
        }
      }
    }
  }

  // If hadError isn't already true, set it to be true if the result
  // string is not just a bunch of spaces
  hadError = hadError || !isBlank(copyOfInput);

  // If there are still parentheses in the string, then
  // the parentheses are unmatched, an error is thrown.
  if (!isBlank(copyOfInput)) {
    error("Unmatched parentheses.", current);
  }
}

// Check if the expression contains only valid characters
void checkExpressionValidity() {
  for (int index = 0; index < findNumberOfTokens(userExp); index++) {
    const char *tempTokenValue = tokens[index].value;
    // Iterate through all the tokens and check if all identifiers are valid
    // Interpret the values of the identifiers and replace their token types
    switch (tokens[index].type) {
      case IDENTIFIER: // Matches an identifier
        tokenizeFunction(index, tempTokenValue);
        break;
      default: {
        break;
      }
    }
  }
}

void tokenizeFunction(int index, const char *tempTokenValue) {
  if (match(tempTokenValue, "pi")) {
    // Match 'pi', which represents π
    Token t = {"pi", MATH_PI, 25};
    tokens[index] = t;
  } else if (match(tempTokenValue, "exp")) {
    // Match 'exp', which matches Euler's constant
    Token t = {"exp", MATH_E, 25};
    tokens[index] = t;
  } else if (match(tempTokenValue, "rand")) {
    Token t = {"rand", RAND_NUM, 25};
    tokens[index] = t;
  } else {
    // Tokenize functions
    if (match(tempTokenValue, "sqrt")) tokens[index] = initToken("sqrt", SQRT);
    else if (match(tempTokenValue, "cbrt")) tokens[index] = initToken("cbrt", CBRT);
    else if (match(tempTokenValue, "log")) tokens[index] = initToken("log", LOG);
    else if (match(tempTokenValue, "ln")) tokens[index] = initToken("ln", LN);
    else if (match(tempTokenValue, "sin")) tokens[index] = initToken("sin", SIN);
    else if (match(tempTokenValue, "cos")) tokens[index] = initToken("cos", COS);
    else if (match(tempTokenValue, "tan")) tokens[index] = initToken("tan", TAN);
    else if (match(tempTokenValue, "asin")) tokens[index] = initToken("asin", ASIN);
    else if (match(tempTokenValue, "acos")) tokens[index] = initToken("acos", ACOS);
    else if (match(tempTokenValue, "atan")) tokens[index] = initToken("atan", ATAN);
    else if (match(tempTokenValue, "sinh")) tokens[index] = initToken("sinh", SINH);
    else if (match(tempTokenValue, "cosh")) tokens[index] = initToken("cosh", COSH);
    else if (match(tempTokenValue, "tanh")) tokens[index] = initToken("tanh", TANH);
    else if (match(tempTokenValue, "asinh")) tokens[index] = initToken("asinh", ASINH);
    else if (match(tempTokenValue, "acosh")) tokens[index] = initToken("acosh", ACOSH);
    else if (match(tempTokenValue, "atanh")) tokens[index] = initToken("atanh", ATANH);
    else if (match(tempTokenValue, "abs")) tokens[index] = initToken("abs", ABS);
    else if (match(tempTokenValue, "floor")) tokens[index] = initToken("floor", FLOOR);
    else if (match(tempTokenValue, "ceil")) tokens[index] = initToken("ceil", CEIL);
    else if (match(tempTokenValue, "round")) tokens[index] = initToken("round", ROUND);
    else if (match(tempTokenValue, "degtorad")) tokens[index] = initToken("degtorad", DEGTORAD);
    else if (match(tempTokenValue, "radtodeg")) tokens[index] = initToken("radtodeg", RADTODEG);
    else { // Some random word that isn't a reserved identifier
      hadError = true;
      char errorMessage[1024];
      sprintf(errorMessage, "Unexpected identifier '%s'.", tokens[index].value);

      inTokenizeStage = false;
      error(errorMessage, index + 1);
    }
    omitToken(index + 1);
  }
}

void omitToken(int index) {
  if (tokens[index].type == MULTIPLY) {
    tokens[index] = initToken("", PASS_TOKEN);
  }
}

bool match(const char *value, char *anotherValue) {
  return strcmp(value, anotherValue) == 0;
}

// Checks if character is digit from 0 to 9
bool isNumeric(char c) {
  return c >= '0' && c <= '9';
}

// Checks if character is an alphabet from a-z
bool isAlpha(char c) {
  return (tolower(c) >= 'a' && tolower(c) <= 'z');
}

#if defined(WIN32)
HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

// Print in blue/cyan
void blue() {
  SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_GREEN);
}

// Print in bold blue/cyan
void boldBlue() {
  SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
}

// Print in bold red
void boldRed() {
  SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_INTENSITY);
}

// Print in green
void green() {
  SetConsoleTextAttribute(console, FOREGROUND_GREEN);
}

// Print in purple
void purple() {
  SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_RED);
}
#else
// Print in blue/cyan
void blue() {
  printf("\033[0;36m");
}

// Print in bold blue/cyan
void boldBlue() {
  printf("\033[1;36m");
}

// Print in bold red
void boldRed() {
  printf("\033[1;31m");
}

// Print in green
void green() {
  printf("\033[0;32m");
}

// Print in purple
void purple() {
  printf("\033[0;35m");
}
#endif

// Play beep sound
void beep() {
  printf("\a");
}

// Print errors in bold red and set the hadError flag on.
// Additionally, mark where the error occurred in the expression.
void error(char *str, int index) {
  numErrors++;
  boldRed();
  if (numErrors == 6) {
    type("Too many errors identified, please fix the ones pointed out first.\n\n");
    return;
  } else if (numErrors > 6) {
    return;
  }
  hadError = true;
  type(str);
  type("\n");
  if (index >= 0) {
    type("    => ");
    char *point;
    int len = 0;
    int tempIndex = index;
    if (inTokenizeStage) {
      type(userExp);
      type("\n");

      len = (int) strlen(userExp);
    } else {
      tempIndex = 0;
      for (int i = 0; i < index - 1; i++) {
        tempIndex += (int) strlen(tokens[i].value);
      }

      for (int i = 0; i < findNumberOfTokens(userExp); i++) {
        type(tokens[i].value);
      }
      type("\n");

      for (int i = 0; i < findNumberOfTokens(userExp); i++) {
        len += (int) strlen(tokens[i].value);
      }
    }
    point = malloc(len);
    for (int indice = 0; indice < len; indice++) {
      if (indice == tempIndex) {
        point[indice] = '^';
      } else {
        point[indice] = ' ';
      }
    }

    type("       ");
    type(point);
    type("\n");
  }
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
    case MODULO:
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
  type(" - You can enter numbers (including decimals)\n");
  type(" - This calculator is accurate up to 9 decimal digits.\n");
  type(" - You can enter expressions that are ");
  boldBlue();
  type("at most 1024 characters long");
  blue();
  type(".\n");
  type(" - If the evaluated expression is:\n");
  type("\t - Greater than 1e16\n");
  type("\t - Less than -1e16\n");
  type("\t - Between -1e-16 and 1e-16\n");
  type("   The result will be expressed in scientific notation.\n");

  purple();
  type("\nSUPPORTED OPERATIONS\n");
  blue();
  type(" - Operations supported:\n");
  type("\t  Operation      Symbol\n");
  type("\t- Addition       [+]\n");
  type("\t- Subtraction    [-]\n");
  type("\t- Multiplication [*]\n");
  type("\t- Division       [/]\n");
  type("\t- Modulo         [%]\n");
  type("\t- Exponentiation [^]\n");
  type("\t- Factorials     [!]\n");
  type("\t- Parentheses    [()]\n");

  purple();
  type("\nSUPPORTED IDENTIFIERS\n");
  blue();
  type("Note: expressions like '2/3pi' will be evaluated as '(2 / 3) * pi' instead of '2 / (3 * pi)'\n");
  type(" - pi: 3.141...\n");
  type(" - exp: 2.718...\n");
  type(" - rand: generates a random number between 0 and 1\n");

  purple();
  type("\nSUPPORTED FUNCTIONS\n");
  blue();
  type("Hint: functions must be succeeded with parentheses '()'. \n");
  type(" - sqrt(arg): Performs square root on arg.\n");
  type(" - cbrt(arg): Performs cube root on arg.\n");
  type(" - log(arg): Performs logarithm (base 10) on arg.\n");
  type(" - ln(arg): Performs natural logarithm (base e) on arg.\n");
  type(" - sin(arg): Performs sine on arg (in degrees).\n");
  type(" - cos(arg): Performs cosine on arg (in degrees).\n");
  type(" - tan(arg): Performs tangent on arg (in degrees).\n");
  type(" - asin(arg): Performs arcsine on arg.\n");
  type(" - acos(arg): Performs arccosine on arg.\n");
  type(" - atan(arg): Performs arctangent on arg.\n");
  type(" - sinh(arg): Performs hyperbolic sine on arg.\n");
  type(" - cosh(arg): Performs hyperbolic cosine on arg.\n");
  type(" - tanh(arg): Performs hyperbolic tangent on arg.\n");
  type(" - asinh(arg): Performs hyperbolic arcsine on arg.\n");
  type(" - acosh(arg): Performs hyperbolic arccosine on arg.\n");
  type(" - atanh(arg): Performs hyperbolic arctangent on arg.\n");
  type(" - abs(arg): Performs absolute value on arg.\n");
  type(" - floor(arg): Performs rounding (down) on arg.\n");
  type(" - ceil(arg): Performs rounding (up) on arg.\n");
  type(" - round(arg): Performs rounding on arg.\n");
  type(" - degtorad(arg): Performs degree to radian conversion on arg.\n");
  type(" - radtodeg(arg): Performs radian to degree conversion on arg.\n");

  purple();
  type("\nOTHER COMMANDS\n");
  blue();
  type(" - You can type any message that contains the word 'help' to summon this help manual.\n");
  type(" - You can type any message that contains the word 'clear' to clear the console screen.\n");
  type(" - You can type any message that contains the word 'exit' to exit the program.\n");

  type("Hope this helped!\n\n");
}

// Typing function
void type(const char *str) {
  // Find the amount of milliseconds to delay printing each character by.
  // Note that strings will be typed out at different speeds depending on their length.
  // Each string will take ~300 milliseconds to type out
  int timeInMs = (int) (300.0l / strlen(str));

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
