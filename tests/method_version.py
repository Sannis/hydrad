from base import HydraTestCase, daemon_version


class HydraVersionMethodTestCase(HydraTestCase):
    def test_version(self):
        response = self.makeRequest('version', {})
        self.assertEqual(response['data'], {'version': daemon_version})
