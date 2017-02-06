/*
  Copyright (C) 2017 Eneo Tecnologia S.L.
  Authors: Diego Fernandez <bigomby@gmail.com>
     Eugenio Perez <eupm90@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "TestUtils.hpp"

#include "../src/uuid_counters_monitor/kafka_leadership_monitor.hpp"

#include <gtest/gtest.h>
#include <librdkafka/rdkafkacpp.h>

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>

namespace {

using namespace std;
using namespace EventsCounter::UUIDCountersMonitor;
using namespace EventsCounter::TestUtils;

class LeadershipMonitorTest : public ::testing::Test {
protected:
  static char *rand_tmpl(char *tmpl) {
    int fd = mkstemp(tmpl);
    close(fd);
    remove(tmpl);

    return tmpl;
  }
};

TEST_F(LeadershipMonitorTest, leadership) {
  uint8_t retries = 30;
  chrono::milliseconds retry_timeout_ms(1000);

  char tmpl[] = "XXXXXX";
  string rand_str(rand_tmpl(tmpl));
  string topic("topic_" + rand_str);
  string group("group_" + rand_str);

  // Produce a dummy message to force Kafka to create a partition
  UUIDProduce("uuid", "dummy", topic);

  vector<unique_ptr<KafkaLeadershipMonitor>> instances;
  for (uint32_t i = 0; i < 5; i++) {
    unique_ptr<KafkaLeadershipMonitor> instance(
        new KafkaLeadershipMonitor("kafka:9092", topic, group));
    instances.push_back(move(instance));
  }

  while (instances.size() > 0 || retries-- > 0) {
    for (auto &instance : instances) {
      if (instance->check_leadership(retry_timeout_ms)) {
        instances.erase(remove(instances.begin(), instances.end(), instance),
                        instances.end());
        break;
      }
    }
  }

  EXPECT_EQ(instances.size(), 0);
}
} // anonymous namespace

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  const int rc = RUN_ALL_TESTS();

  for (int i = 0; i < 10; ++i) {
    static const int timeout_ms = 100;
    const int destroy_rc = RdKafka::wait_destroyed(timeout_ms);
    if (0 == destroy_rc) {
      break;
    }
  }

  return rc;
}
