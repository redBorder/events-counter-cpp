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

#include "../src/config/json_config.hpp"
#include "../src/consumers/kafka_json_uuid_consumer.hpp"
#include "../src/consumers/kafka_json_uuid_consumer_factory.hpp"
#include "../src/uuid_counter/uuid_counter.hpp"

#include <gtest/gtest.h>
#include <librdkafka/rdkafkacpp.h>

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>

namespace {

using namespace std;
using namespace EventsCounter::Consumers;
using namespace EventsCounter::UUIDCountersDB;
using namespace EventsCounter::TestUtils;
using namespace EventsCounter::Configuration;
using namespace EventsCounter::UUIDCounter;

class UUIDConsumerTest : public ::testing::Test {
protected:
  static string vector_to_json(const vector<string> &svector) {
    stringstream ret;
    for (auto i = svector.cbegin(); i != svector.cend(); i++) {
      if (i != svector.cbegin()) {
        ret << ',';
      }
      ret << "\"" << *i << "\"";
    }

    return ret.str();
  }

  static string test_config(const vector<string> &read_topics,
                            const string &read_group_id,
                            const string &write_topic,
                            const string &json_uuid_key,
                            const vector<string> &uuids) {
    const string read_topics_s = vector_to_json(read_topics);
    const string uuids_s = vector_to_json(uuids);

    stringstream ss;
    // clang-format off
		ss << '{' <<
                    "\"counters_config\":{" <<
                        "\"timer_seconds\": {" <<
                            "\"period\":5," <<
                            "\"offset\":4" <<
                        "}," <<
                        "\"json_read_uuid_key\":\"" << json_uuid_key << "\"," <<
                        "\"read_topics\":[" << read_topics_s << "]," <<
                        "\"write_topic\":\"" << write_topic << "\"," <<
                        "\"rdkafka\": {" <<
                            "\"read\":{" <<
                                "\"group.id\":\"" << read_group_id << "\"," <<
                                "\"metadata.broker.list\":\"kafka:9092\","
                                "\"topic.auto.offset.reset\":\"smallest\""
                            "}" <<
                        "}" <<
                    "}, \"uuids\": [" << uuids_s << ']' <<
               '}';
    // clang-format on

    return ss.str();
  }

  static map<string, uint64_t> uuid_vector_to_map(const vector<string> uuids) {
    map<string, uint64_t> ret;
    for (const auto &uuid : uuids) {
      ret[uuid] = 0;
    }
    return ret;
  }

public:
  static void counter_test() {
    static const string json_uuid_key = "sensor_uuid";
    static const string uuid = "123456";
    static const string invalid_uuid = uuid + "7";
    const string read_topic = random_topic();
    const string group_id = string("group_") + read_topic;
    const string write_topic = random_topic();

    unique_ptr<RdKafka::Conf> conf =
        create_test_kafka_consumer_config("kafka:9092", group_id);
    unique_ptr<JsonConfig> config(new JsonConfig(
        test_config(vector<string>{read_topic}, group_id, write_topic,
                    json_uuid_key, vector<string>{uuid})));

    unique_ptr<KafkaUUIDConsumerFactory> consumer_factory(
        new KafkaUUIDConsumerFactory(config->get_uuid_counter_config()));

    unique_ptr<KafkaJSONUUIDConsumer> uuid_consumer =
        consumer_factory->create();
    UUIDCountersDB::counters_t aux_counters =
        uuid_vector_to_map(config->counters_uuids());
    UUIDCounter counter(move(uuid_consumer), UUIDCountersDB(aux_counters));

    // Invalid UUID, should ignore
    UUIDProduce(json_uuid_key, invalid_uuid, read_topic);
    // Valid UUID, should accept
    UUIDProduce(json_uuid_key, uuid, read_topic);
    while (true) {
      counter.swap_counters(aux_counters);
      if (aux_counters[uuid] != 0) {
        break;
      }

      sleep(1); // @TODO C++ version?
    }

    // Check that any map contains invalid uuid
    ASSERT_EQ(aux_counters.find(invalid_uuid), aux_counters.end());
    counter.swap_counters(aux_counters);
    ASSERT_EQ(aux_counters.find(invalid_uuid), aux_counters.end());
    counter.swap_counters(aux_counters);

    // And bytes have been incremented for valid one
    ASSERT_NE(aux_counters[uuid], 0);
  }
};

TEST_F(UUIDConsumerTest, consumer_uuid) { EXPECT_NO_THROW(counter_test()); }

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
