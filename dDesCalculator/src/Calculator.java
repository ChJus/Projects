import java.math.RoundingMode;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Scanner;

public final class Calculator {
  static int current = 0;
  static int start = 0;
  static String exp;
  static ArrayList<Token> tokens = new ArrayList<>();
  static Symbol[][] levels = {
    {Symbol.POWER, Symbol.POWER},
    {Symbol.MULTIPLY, Symbol.DIVIDE},
    {Symbol.ADD, Symbol.MINUS}
  };
  static char[] validCharacters = new char[]{
    '(', ')',
    '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '.',
    '+', '-', '*', '/', '^'
  };

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
        Token result = parse(tokens);
        if (result == null) {
          continue;
        }
        DecimalFormat df = new DecimalFormat("#.##########");
        df.setRoundingMode(RoundingMode.HALF_UP);
        double doubleResult = Double.parseDouble(result.value);
        if (doubleResult == Double.POSITIVE_INFINITY ||
          doubleResult == Double.NEGATIVE_INFINITY) {
          System.out.println("Error: Operation too large.");
          System.out.println("       Note: may be because of division by 0.");
          System.out.println("Result: " + df.format(doubleResult));
        } else if (Double.isNaN(doubleResult)) {
          System.out.println("Error: divide by 0.");
          System.out.println("Result: NaN");
        } else {
          System.out.println(df.format(doubleResult));
        }
        System.out.println();
      } catch (Exception e) {
        System.out.println("Error: Invalid expression.\n");
      }
    }
  }

  private static Token parse(ArrayList<Token> tokens) {
    current = 0;
    start = 0;
    while (tokens.size() != 1) {
      if (tokens.size() == 2 &&
        tokens.get(0).type == Symbol.START_BRACKET &&
        tokens.get(1).type == Symbol.END_BRACKET) {
        return null;
      }
      int deepestBracket = -1;
      int deepestBracketEnd = -1;
      for (int i = 0; i < tokens.size(); i++) {
        if (i + 2 < tokens.size() &&
          tokens.get(i).type == Symbol.START_BRACKET &&
          tokens.get(i + 1).type == Symbol.NUMBER &&
          tokens.get(i + 2).type == Symbol.END_BRACKET) {
          tokens.remove(i);
          tokens.remove(i + 1);
          break;
        } else if (tokens.get(i).type == Symbol.START_BRACKET) {
          deepestBracket = i;
        }
      }

      if (deepestBracket != -1) {
        for (int i = deepestBracket; i < tokens.size(); i++) {
          if (tokens.get(i).type == Symbol.END_BRACKET && deepestBracketEnd == -1) {
            deepestBracketEnd = i;
          }
        }
      }

      if (deepestBracket != -1 && deepestBracketEnd != -1) {
        evaluate(deepestBracket, deepestBracketEnd);
      } else {
        evaluate(0, tokens.size());
      }
    }
    return tokens.get(0);
  }

  private static void evaluate(int curr, int end) {
    if (end == curr + 1 &&
      tokens.get(curr).type == Symbol.START_BRACKET &&
      tokens.get(curr + 1).type == Symbol.END_BRACKET) {
      tokens.remove(curr);
      tokens.remove(curr);
      return;
    }
    for (Symbol[] level : levels) {
      int current = curr;
      while (current < end) {
        if (tokens.size() == 1 || current >= tokens.size()) {
          return;
        }
        if (tokens.get(current).type == level[0] ||
          tokens.get(current).type == level[1]) {
          tokens.add(current - 1, new Token(
            String.valueOf(
              performOperation(
                tokens.get(current).type, Double.parseDouble(
                  tokens.remove(current - 1).value
                ), Double.parseDouble(
                  tokens.remove(current).value
                )
              )
            ), Symbol.NUMBER));
          tokens.remove(current);
          return;
        }
        current++;
      }
    }
  }

  private static double performOperation(Symbol operation, double val1, double val2) {
    switch (operation) {
      case ADD:
        return val1 + val2;
      case MINUS:
        return val1 - val2;
      case MULTIPLY:
        return val1 * val2;
      case DIVIDE:
        return val1 / val2;
      case POWER:
        return Math.pow(val1, val2);
    }
    return 0;
  }

  private static ArrayList<Token> tokenize(String exp) {
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

  private static void tokenizeNumber() {
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

  private static boolean isNumeric(char c) {
    return c >= '0' && c <= '9';
  }

  private static boolean hasMatchedBrackets(String exp) {
    String ex = exp.replaceAll("[^()]", "");
    while (ex.length() > 2) {
      ex = ex.replaceAll("\\(\\)", "");
      if (!ex.contains("()")) {
        return false;
      }
    }
    return ex.equals("()") || ex.equals("");
  }

  private static boolean isValidExpression(String exp) {
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
    START_BRACKET, END_BRACKET
  }

  static class Token {
    public String value;
    public Symbol type;

    public Token(String value, Symbol type) {
      this.value = value;
      this.type = type;
    }

    @Override
    public String toString() {
      return "Token {" +
        "value = '" + value + '\'' +
        ", type = " + type +
        '}';
    }
  }
}