#!/usr/bin/env python3

""" The lpbroutonlab unittests """

import subprocess
from subprocess import TimeoutExpired
import os
from os.path import join
import unittest
from datetime import datetime
from abc import ABCMeta, abstractmethod

from .colored_testrunner import ColoredTextTestRunner


TIMEOUT = 15  # sec


class BehrensConverterTestcase(unittest.TestCase):
    """ Testcase of the lpbroutonlab lib """

    def __init__(self, cli, *args, **kwargs):
        """ Initialize lpbroutonlab testcase """

        self.argrunner = None
        self.cli = cli

        self.tick = None
        self.post_message = None
        unittest.TestCase.__init__(self, *args, **kwargs)

    def runTest(self):  # pylint: disable=invalid-name
        """ Run the lpbroutonlab testcase """

    def setUp(self):
        """ Setup the lpbroutonlab testcase """
        self.tick = datetime.now()

    def tearDown(self):
        """ tear down the lpbroutonlab testcase """
        self.tick = (datetime.now() - self.tick).total_seconds()


class TestFunctionality(BehrensConverterTestcase, metaclass=ABCMeta):  # pylint: disable=too-many-public-methods
    """" Test the command line interface """

    # @abstractmethod
    # def runner(self, argv):
    #     """ Run test """

    def shortDescription(self):
        """ Print description of native testing """
        return "Run " + self._testMethodName + ": "  

    def test_angles(self):
        from .angles import to_polar 
        pc = 2
        prc = 3
        x1 = [165, 26]
        x2 = [59, 123]
        x3 = [-58,123]
        x4 = [-132,52]
        x5 = [-132,-69]
        x6 = [-58,-134]
        x7 = [71,-154]
        x8 = [153, -85]
        i1 = [0, 0]
        i2 = [37, 26]

        def add_data(x1, x2, i, cat, angle):
            inv_cat = prc if cat == pc else pc
            return [
                x1 + x2 + i + [cat, angle],
                x1 + x2 + i + [inv_cat, 360 - angle]
            ]

        data = [
            add_data(x1, x2, i1, prc, 55.42),
            add_data(x1, x4, i1, prc, 149.54),
            add_data(x1, x3, i1, prc, 106.29),
            add_data(x1, x6, i1, pc, 122.36),
            add_data(x1, x8, i1, pc, 38.01), 
            add_data(x2, x5, i1, prc, 143.22),
            add_data(x3, x4, i1, prc, 43.25),
            add_data(x3, x5, i1, prc, 92.35),
            add_data(x3, x8, i1, pc, 144.30),
            add_data(x5, x6, i1, prc, 39.0),
            add_data(x5, x7, i1, prc, 87.15),
            add_data(x6, x7, i1, prc, 48.16),
            add_data(x7, x8, i1, prc, 36.19),
            add_data(x2, x4, i2, prc, 94.03),
        ]

        def check_case(i_case, x1, y1, x2, y2, i, j, gval, phi_): 
            (phi, ((r1, phi1), (r2, phi2))) = to_polar(x1, y1, x2, y2, i, j, gval, None)
            if round(phi, 2) != round(phi_,2):
                raise Exception("i:{}, phi: {} {}, r1: {}, r2: {}, phi1: {}, phi2: {}".format(
                    i_case, round(phi, 2), round(phi_, 2), r1, r2, phi1, phi2))
            
        errors = 0
        for i, d_c in enumerate(data):
            for d in d_c:
                try:
                    check_case(i, *d)
                except Exception as exc:
                    errors += 1
                    print("Error on data: ", str(exc))
        if errors:
            raise Exception("Errors: ", str(errors))


def run_tests(cli, tests):
    """ Run the tests """

    testcases = []
    for test in tests:
        testcases.append(TestFunctionality(cli, test))

    # for test in tests:
    #     testcases.append(TestCLIProcess(None, test))


    suite = unittest.TestSuite(tests=testcases)
    runner = ColoredTextTestRunner()
    runner.run(suite)


def test_all(cli):
    """ Run authentication tests """

    tests = [
        'test_angles'
    ]
    run_tests(cli, tests)


