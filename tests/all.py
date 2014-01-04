import unittest
import os
import subprocess

daemon_relative_filename = '/../hydrad'

class HydraTestCase(unittest.TestCase):
    def __init__(self,x):
        unittest.TestCase.__init__(self)
        self.daemon_filename = os.path.abspath(os.path.dirname(__file__) + '/' + daemon_relative_filename)

    def setUp(self):
        self.daemon_process = subprocess.Popen(self.daemon_filename)

    def tearDown(self):
        self.daemon_process.terminate()

class HydraSmokeTestCase(HydraTestCase):
    def runTest(self):
        self.assertTrue(True, 'True is not true')
        self.assertTrue(self.daemon_process.pid > 0, 'Daemon is not running')

if __name__ == "__main__":
    unittest.main()
