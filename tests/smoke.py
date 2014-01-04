from base import HydraTestCase


class HydraSmokeTestCase(HydraTestCase):
    def test_running(self):
        self.assertTrue(True, 'True is not true')
        self.assertTrue(self.daemon_process.pid > 0, 'Daemon is not running')
