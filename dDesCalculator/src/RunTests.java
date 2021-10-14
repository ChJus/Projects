import java.io.IOException;
import java.math.RoundingMode;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.text.DecimalFormat;

public class RunTests {
  public static void main(String[] args) throws IOException {
    String[] tests = readFile("/Users/JC/IdeaProjects/dDesCalculator/src/tests").trim().split("\n");
    String[] expected = readFile("/Users/JC/IdeaProjects/dDesCalculator/src/expectedResults").trim().split("\n");
    if (tests.length != expected.length) {
      System.out.println("Error: test and expected results files are of different length.");
      return;
    }
    int correct = 0;
    for (int i = 0; i < tests.length; i++) {
      PrattCalculator.exp = tests[i];
      PrattCalculator.tokens = PrattCalculator.tokenize(tests[i]);

      PrattCalculator.current = 0;
      PrattCalculator.start = 0;

      if (PrattCalculator.tokens == null) {
        continue;
      }

      double result = 0;
      try {
        PrattCalculator.token = PrattCalculator.tokens.get(PrattCalculator.parseCurrent);
        result = PrattCalculator.expression(0);
        DecimalFormat df = new DecimalFormat("#.##########");
        df.setRoundingMode(RoundingMode.HALF_UP);
      } catch (Exception e) {
        e.printStackTrace();
        System.out.println("Error: Invalid expression.\n");
      }
      PrattCalculator.tokens.clear();
      PrattCalculator.token = null;
      PrattCalculator.parseCurrent = 0;

      if (String.valueOf(result).contains(expected[i]) && PrattCalculator.hasMatchedBrackets(expected[i]) && PrattCalculator.isValidExpression(expected[i])) {
        System.out.printf("Passed Test %d. Got %.10f which matches %.10f.\n", i + 1, result, Double.parseDouble(expected[i]));
        correct++;
      } else {
        System.out.printf("Failed Test %d. Expected %.10f but got %.10f.\n", i + 1, Double.parseDouble(expected[i]), result);
      }
    }
    System.out.println("\nPassed " + correct + " of " + tests.length + " tests.");
  }

  public static String readFile(String filepath) throws IOException {
    byte[] bytes = Files.readAllBytes(Paths.get(filepath));
    return new String(bytes, Charset.defaultCharset());
  }
}

