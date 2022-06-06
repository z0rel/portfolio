#!/usr/bin/env python3
# coding=utf8

""" Colored output for testrunner """

import unittest
import time


_COLOR = {'green': "\x1b[32;01m",
          'red': "\x1b[31;01m",
          'reset': "\x1b[0m"
          }


def red_str(text):
    """Return red text."""
    return _COLOR['red'] + text + _COLOR['reset']


def green_str(text):
    """Return green text."""
    return _COLOR['green'] + text + _COLOR['reset']


class _ColoredTextTestResult(unittest._TextTestResult):  # pylint: disable=protected-access
    """Colored version."""
    def user_post_message(self, test):
        """ Print user postmessage """
        if test.post_message is not None:
            self.stream.writeln(test.post_message)

    def addSuccess(self, test):
        """ Color success output """
        unittest.TestResult.addSuccess(self, test)
        self.stream.writeln("{0}    on {1:8.4f} sec: {2}".format(
            green_str("Ok"), test.tick, test.shortDescription()))
        self.user_post_message(test)

    def addError(self, test, err):
        """ Color error output """
        unittest.TestResult.addError(self, test, err)
        self.stream.writeln("{0} on {1:8.4f} sec: {2}".format(
            red_str("ERROR"), test.tick, test.shortDescription()))
        self.user_post_message(test)

    def addFailure(self, test, err):
        """ Color failure output """
        unittest.TestResult.addFailure(self, test, err)
        self.stream.writeln("{0}  on {1:8.4f} sec: {2}".format(
            red_str("FAIL"), test.tick, test.shortDescription()))
        self.user_post_message(test)

    def printErrorList(self, flavour, errors):
        """ Color error list """
        for test, err in errors:
            self.stream.writeln(self.separator1)
            descr = test.shortDescription()
            if not descr:
                self.stream.writeln("{0}: {1}".format(red_str(flavour), self.getDescription(test)))
            else:
                self.stream.writeln("{0}: {1}".format(red_str(flavour), descr))
            self.stream.writeln(self.separator2)
            self.stream.writeln("%s" % err)


class ColoredTextTestRunner(unittest.TextTestRunner):  # pylint: disable=too-few-public-methods
    """Override to be color powered."""
    def _makeResult(self):
        " make colored result "
        return _ColoredTextTestResult(self.stream, self.descriptions, self.verbosity)

    def run(self, test):
        "Run the given test case or test suite"
        result = self._makeResult()
        start_time = time.time()
        test(result)
        stop_time = time.time()
        time_taken = float(stop_time - start_time)
        result.printErrors()
        self.stream.writeln(result.separator2)
        run = result.testsRun
        self.stream.writeln("Ran %d test%s in %.3fs" %
                            (run, run != 1 and "s" or "", time_taken))

        self.stream.writeln()
        if not result.wasSuccessful():
            self.stream.write(red_str("FAILED") + " (")
            failed, errors = map(len, (result.failures, result.errors))
            if failed:
                self.stream.write("failures=%d" % failed)
            if errors:
                if failed:
                    self.stream.write(", ")
                self.stream.write("errors=%d" % errors)
            self.stream.writeln(")")
        else:
            self.stream.writeln(green_str("OK"))
        return result
