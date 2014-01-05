from base import HydraTestCase, daemon_version
import time


class HydraStatsMethodTestCase(HydraTestCase):
    def test_stats_uptime(self):
        response = self.makeRequest('stats', {})
        self.assertTrue(response['data']['uptime'] > 0, "Uptime exists")

        time.sleep(0.5)

        response = self.makeRequest('stats', {})
        self.assertTrue(response['data']['uptime'] > 0.5, "Uptime increases")

    def test_stats_rusage(self):
        response = self.makeRequest('stats', {})
        self.assertTrue(response['data']['rusage']['rss'] > 0, "RSS exists")
        self.assertTrue(response['data']['rusage']['ru_maxrss'] > 0, "Rusage max RSS exists")
        self.assertTrue(response['data']['rusage']['ru_stime'] > 0, "Rusage stime exists")
        self.assertTrue(response['data']['rusage']['ru_utime'] > 0, "Rusage utime exists")

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
