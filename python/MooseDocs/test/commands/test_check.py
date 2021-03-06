#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import os
import unittest
import mock
import types
import io
import mooseutils
import moosesqa
from MooseDocs.commands import check

class TestCheckScript(unittest.TestCase):
    def testCheck(self, *args):
        cmd = ['python', 'moosedocs.py', 'check', '--config', 'sqa_test_reports.yml']
        cwd = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..', 'test', 'doc'))
        out = mooseutils.check_output(cmd, cwd=cwd)
        self.assertIn('moose_test', out)
        self.assertIn('OK', out)


class TestCheck(unittest.TestCase):
    def setUp(self):
        # Change to the test/doc directory
        self._working_dir = os.getcwd()
        moose_test_doc_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..', '..', '..', 'test', 'doc'))
        os.chdir(moose_test_doc_dir)

    def tearDown(self):
        # Restore the working directory
        os.chdir(self._working_dir)

    @mock.patch('moosesqa.SQAMooseAppReport._writeFile')
    def testCheck(self, writeFile):

        # Store the filenames to be created
        filenames = list()
        writeFile.side_effect = lambda fn, *args: filenames.append(fn)

        # Create command-line arguments in
        opt = types.SimpleNamespace(config='sqa_test_reports.yml', reports=['app'], dump=None,
                                    app_reports=None, req_reports=None,
                                    generate=['MooseTestApp'], show_warnings=False)

        # --generate
        status = check.main(opt)
        self.assertEqual(status, 0)
        self.assertTrue(len(filenames) > 0)

        # --dump
        opt.dump = ['moose_test']
        opt.generate = None
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertIn('/Kernels/Diffusion', stdout.getvalue())
        self.assertEqual(status, 0)

        # --reports
        opt.reports = ['app']
        opt.config = 'sqa_reports.yml'
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertNotIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertNotIn('REQUIREMENT REPORT(S)', stdout.getvalue())

        opt.reports = ['doc', 'req']
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertNotIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertIn('REQUIREMENT REPORT(S)', stdout.getvalue())

        # --app-reports
        opt.reports = ['app']
        opt.app_reports = ['not-a-value']
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertNotIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertNotIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertNotIn('REQUIREMENT REPORT(S)', stdout.getvalue())

        opt.app_reports = ['framework']
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertNotIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertNotIn('REQUIREMENT REPORT(S)', stdout.getvalue())

        # --req-reports
        opt.reports = ['req']
        opt.req_reports = ['not-a-value']
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertNotIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertNotIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertNotIn('REQUIREMENT REPORT(S)', stdout.getvalue())

        opt.req_reports = ['moose_test']
        with mock.patch('sys.stdout', new=io.StringIO()) as stdout:
            status = check.main(opt)
        self.assertNotIn('MOOSEAPP REPORT(S)', stdout.getvalue())
        self.assertNotIn('DOCUMENT REPORT(S)', stdout.getvalue())
        self.assertIn('REQUIREMENT REPORT(S)', stdout.getvalue())


if __name__ == '__main__':
    unittest.main(verbosity=2)
