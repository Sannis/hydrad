import unittest
import os
import subprocess
import json
import socket
import time


daemon_relative_filename = '/../src/hydrad'
daemon_version = '0.0.1-dev'
daemon_port = 8888
uv_version = '1.8.0'


daemon_absolute_filename = os.path.abspath(os.path.dirname(__file__) + '/' + daemon_relative_filename)


class HydraTestCase(unittest.TestCase):
    def setUp(self):
        self.daemon_process = subprocess.Popen(args = daemon_absolute_filename, stdout = subprocess.PIPE, stderr = subprocess.PIPE)
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
