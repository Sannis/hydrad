import unittest
import os
import subprocess
import json
import socket
import time


daemon_relative_filename = '/../hydrad'
daemon_version = '0.0.1-dev'
daemon_port = 8888


class HydraTestCase(unittest.TestCase):
    def __init__(self,x):
        unittest.TestCase.__init__(self)
        self.daemon_filename = os.path.abspath(os.path.dirname(__file__) + '/' + daemon_relative_filename)

    def setUp(self):
        self.daemon_process = subprocess.Popen(args = self.daemon_filename, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
        time.sleep(0.1)

    def tearDown(self):
        self.daemon_process.terminate()

    def makeRequest(self, method, params):
        # Create socket
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect(('127.0.0.1', daemon_port))

        # Send request
        sock.sendall("%s %s" % (method, json.dumps(params)))
        sock.shutdown(socket.SHUT_WR)

        # Read response
        response = ''
        while True:
            data = sock.recv(1024)
            if len(data) > 0:
                response = response + data
            else:
                break
        sock.close()

        return {'data': json.loads(response)}


class HydraSmokeTestCase(HydraTestCase):
    def runTest(self):
        self.assertTrue(True, 'True is not true')
        self.assertTrue(self.daemon_process.pid > 0, 'Daemon is not running')


class HydraVersionMethodTestCase(HydraTestCase):
    def runTest(self):
        response = self.makeRequest('version', {})
        self.assertEqual(response['data'], {'version': daemon_version})


if __name__ == "__main__":
    unittest.main()
