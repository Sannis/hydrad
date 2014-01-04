from base import HydraTestCase, daemon_version


class HydraStatsMethodTestCase(HydraTestCase):
    def test_stats_requests_total_count(self):
        self.makeRequest('version', {})
        self.makeRequest('version', {})
        self.makeRequest('version', {})

        response = self.makeRequest('stats', {})
        self.assertEqual(response['data']['requests']['total_count'], 4)

        response = self.makeRequest('stats', {})
        self.assertEqual(response['data']['requests']['total_count'], 5)

    def test_stats_requests_error_count(self):
        self.makeRequest('version', {})
        self.makeRequest('version', {})
        self.makeRequest('ver sion', {})

        response = self.makeRequest('stats', {})
        self.assertEqual(response['data']['requests']['total_count'], 4)
        self.assertEqual(response['data']['requests']['error_count'], 1)

        response = self.makeRequest('sta ts', {})

        response = self.makeRequest('stats', {})
        self.assertEqual(response['data']['requests']['total_count'], 6)
        self.assertEqual(response['data']['requests']['error_count'], 2)
