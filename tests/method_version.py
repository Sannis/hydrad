from base import HydraTestCase, daemon_version, uv_version


class HydraVersionMethodTestCase(HydraTestCase):
    def test_version(self):
        response = self.makeRequest('version', {})
        self.assertEqual(response['data'], {'version': daemon_version})

        # Check versions
        self.assertEqual(response['data']['version'], daemon_version)
        self.assertEqual(response['data']['deps']['uv_version'], uv_version)
