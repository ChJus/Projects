import java.math.RoundingMode;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Scanner;

public final class PrattCalculator {
  public static int current = 0;
  public static int start = 0;
  public static int parseCurrent = 0;
  public static String exp;
  public static ArrayList<Token> tokens = new ArrayList<>();
  public static char[] validCharacters = new char[]{
    '(', ')',
    '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '.',
    '+', '-', '*', '/', '^'
  };
  public static Token token;

  public static void main(String[] args) {
    System.out.println("Calculator");
    System.out.println("=".repeat(50) + "\n");
    System.out.println("Supports [()], [0-9], [+-*/], and [^]");
    System.out.println("Type 'exit' to terminate the program.");
    System.out.println("Type 'help' to call for the help manual.");
    Scanner input = new Scanner(System.in);
    while (true) {
      System.out.print("Input expression: \n> ");
      exp = input.nextLine();
      if (exp.toLowerCase().contains("exit")) {
        System.out.println("Goodbye!");
        break;
      }
      if (exp.toLowerCase().contains("help")) {
        System.out.println("Help Manual: \n");
        System.out.println("Calculator");
        System.out.println("=".repeat(50));
        System.out.println("Supports [()], [0-9], [+-*/], and [^]");
        System.out.println("Type 'exit' to terminate the program.");
        System.out.println("Type 'help' to call for the help manual.");
        System.out.println("=".repeat(50) + "\n");
        continue;
      }
      if (!isValidExpression(exp)) {
        System.out.println("Error: Please enter a valid expression.\n");
        continue;
      }
      if (!hasMatchedBrackets(exp)) {
        System.out.println("Error: Expression has unmatched brackets.\n");
        continue;
      }

      tokens = tokenize(exp);

      current = 0;
      start = 0;

      if (tokens == null) {
        continue;
      }

      try {
        token = tokens.get(parseCurrent);
        double result = expression(0);
        DecimalFormat df = new DecimalFormat("#.##########");
        df.setRoundingMode(RoundingMode.HALF_UP);
        if (result == Double.POSITIVE_INFINITY ||
          result == Double.NEGATIVE_INFINITY) {
          System.out.println("Error: Operation too large.");
          System.out.println("       Note: may be because of division by 0.");
          System.out.println("Result: " + df.format(result));
        } else if (Double.isNaN(result)) {
          System.out.println("Error: divide by 0.");
          System.out.println("Result: NaN");
        } else {
          System.out.println(df.format(result));
        }
        System.out.println();
      } catch (Exception e) {
        e.printStackTrace();
        System.out.println("Error: Invalid expression.\n");
      }
      tokens.clear();
      token = null;
      parseCurrent = 0;
    }
  }

  public static double expression(int bindingPower) {
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

  public static double led(Token tempToken, double left) {
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
        return Math.pow(left, expression(30 - 1));
      default:
        throw new IllegalArgumentException("Unable to parse: '" + tempToken.value + "'.");
    }
  }

  public static double nud(Token tempToken) {
    switch (tempToken.type) {
      case NUMBER:
        return Double.parseDouble(tempToken.value);
      case MINUS:
        return -expression(25);
      case START_BRACKET:
        double val = expression(0);
        if (tokens.get(parseCurrent).type != Symbol.END_BRACKET) {
          throw new IllegalArgumentException("Expected ending bracket.");
        }
        token = advance();
        return val;
      default:
        throw new IllegalArgumentException("Unexpected token: '" + tempToken.value + "'.");
    }
  }

  public static Token advance() {
    if (parseCurrent + 1 < tokens.size()) {
      return tokens.get(++parseCurrent);
    } else {
      return new Token("", Symbol.END_OF_EXPRESSION);
    }
  }

  public static ArrayList<Token> tokenize(String exp) {
    start = 0;
    current = 0;
    ArrayList<Token> tokens = new ArrayList<>();
    while (current < exp.length()) {
      char c = exp.charAt(current);
      if (isNumeric(c)) {
        tokenizeNumber();

        tokens.add(new Token(exp.substring(start, current + 1), Symbol.NUMBER));
        current++;
        start = current;
        continue;
      }
      switch (c) {
        case ' ':
          break;
        case '(':
          tokens.add(new Token(c + "", Symbol.START_BRACKET));
          break;
        case ')':
          tokens.add(new Token(c + "", Symbol.END_BRACKET));
          break;
        case '+':
          tokens.add(new Token(c + "", Symbol.ADD));
          break;
        case '-':
          tokens.add(new Token(c + "", Symbol.MINUS));
          break;
        case '*':
          tokens.add(new Token(c + "", Symbol.MULTIPLY));
          break;
        case '/':
          tokens.add(new Token(c + "", Symbol.DIVIDE));
          break;
        case '^':
          tokens.add(new Token(c + "", Symbol.POWER));
          break;
        case '.':
          System.out.println("Error: Unexpected '.', please have digits before '.' (e.g., 0.1 instead of .1)");
          System.out.println("       Also, numbers can only have one '.' (e.g., no 1.1.1)\n");
          return null;
        default:
          System.out.println("Error: Unknown character: '" + c + "'\n");
          return null;
      }
      current++;
      start = current;
    }
    return tokens;
  }

  public static void tokenizeNumber() {
    while (current + 1 < exp.length() && (isNumeric(exp.charAt(current + 1)))) {
      current++;
    }

    if (current + 1 < exp.length() && exp.charAt(current + 1) == '.') {
      current++;
      while (current + 1 < exp.length() && isNumeric(exp.charAt(current + 1))) {
        current++;
      }
    }
  }

  public static boolean isNumeric(char c) {
    return c >= '0' && c <= '9';
  }

  public static boolean hasMatchedBrackets(String exp) {
    String ex = exp.replaceAll("[^()]", "");
    while (ex.length() > 2) {
      ex = ex.replaceAll("\\(\\)", "");
      if (!ex.contains("()") && ex.length() > 2) {
        return false;
      }
    }
    return ex.equals("()") || ex.equals("");
  }

  public static boolean isValidExpression(String exp) {
    boolean isOneOfValid = false;
    for (char c : exp.toCharArray()) {
      for (char validChar : validCharacters) {
        if (c == validChar) {
          isOneOfValid = true;
          break;
        }
      }
      if (!isOneOfValid) {
        return false;
      }
    }
    return true;
  }

  enum Symbol {
    NUMBER,
    ADD, MINUS,
    DIVIDE, MULTIPLY,
    POWER,
    START_BRACKET, END_BRACKET,
    END_OF_EXPRESSION
  }

  static class Token {
    public String value;
    public Symbol type;
    public int bindingPower = 0;

    public Token(String value, Symbol type) {
      this.value = value;
      this.type = type;

      switch (type) {
        case ADD:
        case MINUS:
          this.bindingPower = 10;
          break;
        case MULTIPLY:
        case DIVIDE:
          this.bindingPower = 20;
          break;
        case POWER:
          this.bindingPower = 30;
          break;
      }
    }
  }
}