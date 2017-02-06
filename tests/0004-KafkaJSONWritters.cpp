// Copyright (C) 2017 Eneo Tecnologia S.L.
//
// Authors:
//   Diego Fernandez <bigomby@gmail.com>
//   Eugenio Perez <eupm90@gmail.com>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "TestUtils.hpp"

#include "../src/utils/uuid_bytes.hpp"
#include "../src/uuid_counters_db/uuid_counters_db.hpp"
// #include "../src/writters/json_counter_writter.hpp"
#include "../src/writters/json_monitor_writter.hpp"

#include <gtest/gtest.h>
#include <rapidjson/document.h>

using namespace std;
using namespace EventsCounter;
using namespace EventsCounter::Formatters;
using namespace EventsCounter::TestUtils;

namespace {

class KafkaJSONFormattersTest : public ::testing::Test {};

TEST_F(KafkaJSONFormattersTest, produce_monitor_message) {
  char tmpl[] = "test_XXXXXX";
  char *tmp_str = rand_tmpl(tmpl);

  string brokers("kafka:9092");
  string topic_str(string("topic_") + tmp_str);
  string group_str(string("group_") + tmp_str);

  unique_ptr<RdKafka::KafkaConsumer> consumer(
      bootstrap_test_kafka_consumer(brokers, group_str, topic_str));
  unique_ptr<RdKafka::Producer> producer(
      bootstrap_test_kafka_producer(brokers));
  unique_ptr<RdKafka::Topic> topic(
      bootstrap_test_kafka_topic(producer, topic_str));

  Utils::UUIDBytes uuid_bytes("00000000-0000-0000-C000-000000000000", 42);

  unique_ptr<JSONKafkaMessage> message(
      new JSONKafkaMessage(uuid_bytes.get_uuid()));
  JSONMonitorWritter<>(uuid_bytes, message->string_buffer);

  producer->produce(topic.get(), 0, RdKafka::Producer::RK_MSG_COPY,
                    message->payload(), message->len(), message->key(),
                    message->key_len(), NULL);

  while (true) {
    unique_ptr<RdKafka::Message> msg(consumer->consume(1000));
    if (msg->err() == RdKafka::ErrorCode::ERR_NO_ERROR) {
      rapidjson::Document json;
      const char *payload = static_cast<const char *>(msg->payload());
      json.Parse(payload);

      rapidjson::Value &uuid = json["uuid"];
      EXPECT_TRUE(uuid.IsString());
      EXPECT_STREQ("00000000-0000-0000-C000-000000000000", uuid.GetString());

      rapidjson::Value &value = json["value"];
      EXPECT_TRUE(value.IsUint64());
      EXPECT_EQ(42, value.GetUint64());

      break;
    }
  }

  consumer->close();
  wait_kafka_destroyed();
}

// TEST_F(KafkaJSONFormattersTest, produce_counter_message) {
//   char tmpl[] = "test_XXXXXX";
//   char *tmp_str = rand_tmpl(tmpl);
//
//   string brokers("kafka:9092");
//   string topic_str(string("topic_") + tmp_str);
//   string group_str(string("group_") + tmp_str);
//
//   unique_ptr<RdKafka::KafkaConsumer> consumer(
//       bootstrap_test_kafka_consumer(brokers, group_str, topic_str));
//   unique_ptr<RdKafka::Producer> producer(
//       bootstrap_test_kafka_producer(brokers));
//   unique_ptr<RdKafka::Topic> topic(
//       bootstrap_test_kafka_topic(producer, topic_str));
//
//   Utils::UUIDBytes uuid_bytes("00000000-0000-0000-C000-000000000000", 42);
//
//   unique_ptr<JSONKafkaMessage> message(
//       new JSONKafkaMessage(uuid_bytes.get_uuid()));
//   JSONCounterWritter<>(uuid_bytes, message->string_buffer);
//
//   producer->produce(topic.get(), 0, RdKafka::Producer::RK_MSG_COPY,
//                     message->payload(), message->len(), message->key(),
//                     message->key_len(), NULL);
//
//   while (true) {
//     unique_ptr<RdKafka::Message> msg(consumer->consume(1000));
//     if (msg->err() == RdKafka::ErrorCode::ERR_NO_ERROR) {
//       rapidjson::Document json;
//       const char *payload = static_cast<const char *>(msg->payload());
//       json.Parse(payload);
//
//       rapidjson::Value &uuid = json["uuid"];
//       EXPECT_TRUE(uuid.IsString());
//       EXPECT_STREQ("00000000-0000-0000-C000-000000000000", uuid.GetString());
//
//       rapidjson::Value &value = json["value"];
//       EXPECT_TRUE(value.IsUint64());
//       EXPECT_EQ(42, value.GetUint64());
//
//       break;
//     }
//   }
//
//   consumer->close();
//   wait_kafka_destroyed();
// }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
