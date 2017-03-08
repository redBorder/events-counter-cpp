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

#include "../src/config/config.hpp"
#include "../src/consumers/events_consumer.hpp"
#include "../src/consumers/kafka_json_counter_consumer.hpp"
#include "../src/consumers/kafka_json_uuid_consumer_factory.hpp"
#include "../src/producers/kafka_monitor_producer.hpp"
#include "../src/utils/uuid_bytes.hpp"
#include "../src/uuid_counters_db/uuid_counters_db.hpp"
#include "../src/uuid_counters_monitor/uuid_counters_monitor.hpp"
#include "../src/writters/json_counter_writter.hpp"
#include "TestUtils.hpp"

#include <gtest/gtest.h>

using namespace std;
using namespace EventsCounter::TestUtils;
using namespace EventsCounter::UUIDCountersDB;
using namespace EventsCounter::UUIDCountersMonitor;
using namespace EventsCounter::Consumers;
using namespace EventsCounter::Producers;
using namespace EventsCounter::Utils;
using namespace EventsCounter::Formatters;
using namespace EventsCounter::Configuration;

namespace {

class UUIDCountersMonitorTest : public ::testing::Test {};

TEST_F(UUIDCountersMonitorTest, produce_monitor_message) {
  char tmpl[] = "test_XXXXXX";
  char *tmp_str = rand_tmpl(tmpl);

  string brokers("kafka:9092");
  string counter_topic(string("counters_topic_") + tmp_str);
  string monitor_topic(string("monitor_topic_") + tmp_str);
  string group_id(string("group_test_") + tmp_str);

  /////////////////////////////////////////////
  // TEST Monitor Consumer and UUID Producer //
  /////////////////////////////////////////////

  // Counter producer
  unique_ptr<RdKafka::Producer> test_counter_producer(
      bootstrap_test_kafka_producer(brokers));
  unique_ptr<RdKafka::Topic> test_counter_topic_producer(
      bootstrap_test_kafka_topic(test_counter_producer, counter_topic));

  // Create TEST Kafka monitor consumer
  // unique_ptr<RdKafka::KafkaConsumer> test_monitor_consumer(
  //     bootstrap_test_kafka_consumer(brokers, group_id, counter_topic));

  ///////////////////////////////////////////
  // Counter consumer and Monitor Producer //
  ///////////////////////////////////////////

  // Create Kafka counter consumer
  kafka_conf_list_t rkt_conf = {{"auto.offset.reset", "smallest"}};
  kafka_conf_list_t rk_conf = {{"metadata.broker.list", "kafka:9092"},
                               {"group.id", group_id}};
  unique_ptr<EventsConsumer> counter_consumer(
      new KafkaJSONCounterConsumer(100, 0, counter_topic, rk_conf, rkt_conf));

  // Create Kafka monitor producer
  // struct counters_monitor_config_s counters_monitor_config = {};
  // unique_ptr<MonitorProducer> monitor_producer(
  //     new KafkaMonitorProducer(counters_monitor_config));

  ///////////////////////////
  // UUID Counters Monitor //
  ///////////////////////////

  // Create an UUIDCounter
  const UUIDCountersDB::counters_t boostrap_map = {
      {"a", 0}, {"b", 0}, {"c", 0}};
  const UUIDCountersDB::counters_t limits = {
      {"a", 100}, {"b", 150}, {"c", 200}};

  // Create an UUIDCountersMonitor
  UUIDCountersDB monitor_db(boostrap_map);
  UUIDCountersMonitor monitor(monitor_db, limits);

  // Produce test counter
  UUIDBytes uuid_bytes("a", 42);
  unique_ptr<JSONKafkaMessage> message(
      new JSONKafkaMessage(uuid_bytes.get_uuid()));
  JSONCounterWritter<>(uuid_bytes, message->string_buffer);
  test_counter_producer->produce(test_counter_topic_producer.get(), 0,
                                 RdKafka::Producer::RK_MSG_COPY,
                                 message->payload(), message->len(),
                                 message->key(), message->key_len(), NULL);

  // Check counters
  UUIDBytes data;
  while (true) {
    data = counter_consumer->consume(100);
    if (!data.empty()) {
      break;
    }
  }

  // if (monitor.check(data.get_uuid(), data.get_bytes())) {
  //   monitor_producer->produce(data);
  // }
}
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
