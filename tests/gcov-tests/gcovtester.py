#!/usr/bin/env python
import sys
import os
import argparse
import subprocess
import glob

# Class for gcov, lcov testing
class GcovTester:

    # Inner class for colors
    class _colors:
        okgreen = '\033[92m'
        fail = '\033[91m'
        okblue = '\033[94m'
        bold = '\033[1m'
        endc = '\033[0m'

    # Getting the arguments
    def get_arguments(self):
        parser = argparse.ArgumentParser()
        parser.add_argument("--build-coverage",
        help = "builds JerryScript with the compile flags needed for coverage measurement", action = 'store_true')
        parser.add_argument("--test-file", help = "the location of the test to run")
        parser.add_argument("--run-tests", help = "run all tests for JerryScript", action='store_true')
        parser.add_argument("--src", help = "the path of the file to be coverage tested", type = str, required = True)
        parser.add_argument("--jerry-location", help = "the location of the JerryScript project folder",
        required = True)
        parser.add_argument("--browser", help = "the name of the browser for opening  the output", type = str)
        parser.add_argument("-q", help = "Does not print any output", action='store_true')

        # If no arguments given print help and exit
        if len(sys.argv) <= 1:
            parser.print_help()
            sys.exit(1)

        return parser.parse_args()

    # Printing debug messages
    def debug(self, message, options):
        if (not options.q):
            print(message)

    # Clean building JerryScript with compile flags needed for coverage analysis if needed
    def build(self, options):
        if (options.build_coverage):
            self.build_py = os.path.join(options.jerry_location, "tools/build.py")  # The absolute path of engine location
            self.build_command = "python %s --clean --compile-flag=-fprofile-arcs " % (self.build_py)
            self.build_command +=  "--compile-flag=-ftest-coverage --compile-flag=--coverage --compile-flag=-ggdb3 --debug" # The build command
            self.process = subprocess.Popen(self.build_command.split(' ')) # Executing the command
            self.process.communicate() # Waiting for process to terminate

    # Run test suites
    def run_test_suites(self, options):

        self.bash_command = "%s --jerry-test-suite --test262 --unittests --jerry-tests --buildoptions=--compile-flag=-ftest-coverage," % (os.path.join(options.jerry_location, "tools", "run-tests.py" ))
        self.bash_command += "--compile-flag=--coverage,--compile-flag=-ggdb3,--debug"
        self.process = subprocess.Popen(self.bash_command.split()) # Executing the command
        self.process.communicate() # Waiting for process to terminate

    # Runs .js file with the JerryScript engine
    def run_js_in_jerry(self, options):
        # The absolute path of test file
        self.test_file = os.path.abspath(options.test_file)
        # The absolute path of the engine
        self.jerry_engine = os.path.abspath(os.path.join(options.jerry_location, "build/bin/jerry"))
        # Running the .js file with the engine
        self.process = subprocess.Popen([self.jerry_engine, self.test_file], stderr = open(os.devnull, 'w'))
        self.process.communicate() # Waiting for process to terminate
        # Return code of the .js file colorised
        self.debug("%s%s\nTest\t\t\t\t\t\t\t\t\tExit code\n%s" % (self._colors.bold, self._colors.okblue, self._colors.endc),
        options)
        if (not self.process.returncode):
            self.debug("%s%s   \t%d%s" % (self._colors.okgreen, self.test_file,
            self.process.returncode, self._colors.endc), options)
        else:
            self.debug("%s%s   \t%d%s" % (self._colors.fail, self.test_file,
            self.process.returncode, self._colors.endc), options)

    def one_file_or_suite(self, options):
        # If the test file is a python file we want to execute a suite
        if (options.run_tests):
            self.run_test_suites(options)
        # Else if it is a JavaScript file, we want to execute that one file in JerryScript
        else:
            self.run_js_in_jerry(options)

    # Captures the information about code coverage
    def capture(self, options):
        self.debug("%s%s\nCapturing coverage information\n%s" % (self._colors.bold, self._colors.okblue,
        self._colors.endc), options)
        self.bash_command = "lcov --rc lcov_branch_coverage=1 --capture --directory . --output-file coverage.info" # Capture command
        if options.q: # No output showed (optional)
            self.bash_command += " --quiet"
        self.process = subprocess.Popen(self.bash_command.split()) # Executing the command
        self.process.communicate() # Waiting for process to terminate

    # Generating the html file
    def generate_html(self, options):
        self.debug("%s%s\nGenerating HTML\n%s" % (self._colors.bold, self._colors.okblue, self._colors.endc), options)
        self.out_dir = os.path.abspath("./out") # The absolute path of the output directory
        self.src_file = os.path.abspath(options.src) # The absolute path of the src_file we want to check
        self.html_page = os.path.join(self.out_dir, os.path.relpath(self.src_file, options.jerry_location))
        self.html_page = self.html_page + ".gcov.html" # The address of the html page of the src_file
        self.bash_command = "genhtml coverage.info --output-directory out"
        self.bash_command += " --legend -s --branch-coverage --prefix %s" % (options.jerry_location) # The generating command
        if options.q: # No output showed (optional)
            self.bash_command += " --quiet"
        self.process = subprocess.Popen(self.bash_command.split()) # Executing the command
        self.process.communicate() # Waiting for process to terminate
        self.debug("%s%s\nDone\n%s" % (self._colors.bold, self._colors.okblue, self._colors.endc), options)

    # Opens the html on the page of the src_file if needed
    def open_html(self, options):
        if (options.browser): # Opening results in a choosen browser (optional)
            # The command to open the page with a choosen browser
            self.bash_command = "%s %s" % (options.browser, self.html_page)
            self.process = subprocess.Popen(self.bash_command.split()) # Executing the command
            self.process.communicate() # Waiting for process to terminate

    # Build (optional), run .js file with the engine, create coverage tests, and show results
    def run_gcov_test(self, options):
        # Building JerryScript if needed
        self.build(options)
        # Run .js file or the suite with the engine
        self.one_file_or_suite(options)
        # Captures the information about code coverage
        self.capture(options)
        # Generating the html file
        self.generate_html(options)
        # Opens the html on the page of the src_file if needed
        self.open_html(options)


def main():
    gcov = GcovTester()
    gcov.run_gcov_test(gcov.get_arguments())

if __name__ == '__main__':
    main()
