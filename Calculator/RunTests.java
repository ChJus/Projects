import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;

public class RunTests {
  public static void main(String[] args) throws IOException {
    String[] tests = readFile("/Users/JC/CLionProjects/Calculator/tests.txt").trim().split("\n");
    String[] expected = readFile("/Users/JC/CLionProjects/Calculator/expectedResults.txt").trim().split("\n");
    if (tests.length != expected.length) {
      System.out.println("Error: test and expected results files are of different length.");
      return;
    }
    int correct = 0;
    for (int i = 0; i < tests.length; i++) {
      Process pb = Runtime.getRuntime().exec(new String[]{"/Users/JC/CLionProjects/Calculator/cmake-build-debug-coverage/Calculator", tests[i]});

      BufferedReader reader = new BufferedReader(new InputStreamReader(pb.getInputStream()));

      StringBuilder result = new StringBuilder();
      String line;
      while ((line = reader.readLine()) != null) {
        result.append(line).append("\n");
      }
      result = new StringBuilder(result.toString().trim().substring(6));
      if (result.toString().contains(expected[i])) {
        System.out.printf("Passed Test %d. Got '%s' which matches '%f'.\n", i + 1, result, Double.parseDouble(expected[i]));
        correct++;
      } else {
        System.err.printf("Failed Test %d. Got '%s' but expected '%f'.\n", i + 1, result, Double.parseDouble(expected[i]));
      }
    }
    System.out.println("\nPassed " + correct + " of " + tests.length + " tests.");
  }

  public static String readFile(String filepath) throws IOException {
    byte[] bytes = Files.readAllBytes(Paths.get(filepath));
    return new String(bytes, Charset.defaultCharset());
  }
}

