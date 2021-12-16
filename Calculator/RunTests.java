import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.nio.file.Files;
import java.nio.file.Paths;

/*
 * This program is designed to run tests on my calculator.c program through the use of
 * process execution (similar to using terminal) to run the binary executable present
 * in the project directory. I tweaked my C program to be compatible with both
 * CLI arguments and runs without arguments. If an argument is given in the command line,
 * the program executes the expression and terminates. Otherwise, the program
 * enters a loop that asks for expressions and evaluates them.
 *
 * Tests are stored in a single file, where each line represents a test. There is a separate file
 * that stores the expected results from the tests, and this program simply compares the results
 * to see if a test passes. The number of tests passed is printed at the end.
 */
public class RunTests {
  public static void main(String[] args) throws IOException {
    // Support running this program with CL arguments. The program accepts an optional
    // argument that represents the test number to start from.
    int start = 0;
    if (args.length > 0) { // Parse the test number and store it as an index in start
      start = Integer.parseInt(args[0]) - 1;
    }

    // Read file of tests and split by delimiter '\n'
    String[] tests = readFile("/Users/JC/CLionProjects/Calculator/tests.txt").trim().split("\n");

    // Read file of expected results and split by delimiter '\n'
    String[] expected = readFile("/Users/JC/CLionProjects/Calculator/expectedResults.txt").trim().split("\n");

    // Report error if the number of tests and expected results aren't the same.
    if (tests.length != expected.length) {
      System.out.println("Error: test and expected results files are of different length.");
      return;
    }

    // Run tests
    int correct = 0;
    for (int i = start; i < tests.length; i++) {
      // Execute terminal-like command
      Process pb = Runtime.getRuntime().exec(new String[]{"/Users/JC/CLionProjects/Calculator/cmake-build-debug/Calculator", tests[i]});

      // Read the output of the program
      BufferedReader reader = new BufferedReader(new InputStreamReader(pb.getInputStream()));

      // Append each line of the output of the program
      StringBuilder result = new StringBuilder();
      String line;
      while ((line = reader.readLine()) != null) {
        result.append(line).append("\n");
      }

      // Replace text color-changing commands.
      result = new StringBuilder(result.toString().trim().replaceAll("\\[0;36m", "").replaceAll("\\[1;31m", ""));

      // Print whether test passed
      if (result.toString().contains(expected[i])) {
        System.out.printf("Passed Test %d.\n", i + 1);
        correct++;
      } else { // If test failed, print out the expected and actual result
        System.err.printf("Failed Test %d. Got '%s' but expected '%s'.\n", i + 1, result, expected[i]);
      }
    }
    // Print out the total number of tests passed.
    System.out.println("\nPassed " + correct + " of " + (tests.length - start) + " tests.");
  }

  // Read entire file into string and return string.
  public static String readFile(String filepath) throws IOException {
    byte[] bytes = Files.readAllBytes(Paths.get(filepath));
    return new String(bytes, Charset.defaultCharset());
  }
}
