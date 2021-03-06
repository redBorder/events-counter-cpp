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

#include <gtest/gtest.h>

using namespace EventsCounter::TestUtils;
using namespace std;

std::unique_ptr<RdKafka::Conf>
EventsCounter::TestUtils::create_test_kafka_consumer_config(
    const std::string &brokers, const std::string &group_id) {
  string errstr;
  unique_ptr<RdKafka::Conf> topic_conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));
  unique_ptr<RdKafka::Conf> conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

  struct property {
    RdKafka::Conf *conf;
    std::string key, val;
  };

  const array<struct property, 3> properties{{
      {topic_conf.get(), "auto.offset.reset", "smallest"},
      {conf.get(), "metadata.broker.list", brokers},
      {conf.get(), "group.id", group_id},
  }};

  for (const struct property &property : properties) {
    const RdKafka::Conf::ConfResult rc =
        property.conf->set(property.key, property.val, errstr);

    EXPECT_EQ(rc, RdKafka::Conf::CONF_OK);
  }

  const RdKafka::Conf::ConfResult set_default_topic_conf_rc =
      conf->set("default_topic_conf", topic_conf.get(), errstr);
  EXPECT_EQ(set_default_topic_conf_rc, RdKafka::Conf::CONF_OK);
  return conf;
}

/// Produce a single kafka message
void EventsCounter::TestUtils::UUIDProduce(const std::string &uuid_key,
                                           const std::string &uuid,
                                           const std::string &topic_str) {
  const string message = string("{\"") + uuid_key + "\": \"" + uuid + "\"}";
  string errstr;

  unique_ptr<RdKafka::Conf> conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
  unique_ptr<RdKafka::Conf> tconf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

  conf->set("metadata.broker.list", "kafka:9092", errstr);

  unique_ptr<RdKafka::Producer> producer(
      RdKafka::Producer::create(conf.get(), errstr));
  ASSERT_TRUE(NULL != producer.get());

  unique_ptr<RdKafka::Topic> topic(
      RdKafka::Topic::create(producer.get(), topic_str, tconf.get(), errstr));
  ASSERT_TRUE(NULL != topic.get());

  producer->produce(topic.get(), 0, RdKafka::Producer::RK_MSG_COPY,
                    const_cast<char *>(message.c_str()), message.size(), NULL,
                    NULL);

  while (producer->outq_len() > 0) {
    producer->poll(100);
  }
}

std::unique_ptr<RdKafka::KafkaConsumer>
EventsCounter::TestUtils::bootstrap_test_kafka_consumer(
    const std::string &brokers, const std::string &group_id,
    const std::string &topic) {
  string errstr;
  vector<string> topics;
  topics.push_back(topic);

  unique_ptr<RdKafka::Conf> topic_conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));
  unique_ptr<RdKafka::Conf> conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

  EXPECT_EQ(RdKafka::ErrorCode::ERR_NO_ERROR,
            topic_conf->set("auto.offset.reset", "smallest", errstr));
  EXPECT_EQ(RdKafka::ErrorCode::ERR_NO_ERROR,
            conf->set("metadata.broker.list", brokers, errstr));
  EXPECT_EQ(RdKafka::ErrorCode::ERR_NO_ERROR,
            conf->set("group.id", group_id, errstr));
  EXPECT_EQ(RdKafka::ErrorCode::ERR_NO_ERROR,
            conf->set("default_topic_conf", topic_conf.get(), errstr));

  unique_ptr<RdKafka::KafkaConsumer> consumer(
      RdKafka::KafkaConsumer::create(conf.get(), errstr));
  EXPECT_TRUE(consumer.get() != nullptr);

  consumer->subscribe(topics);

  return consumer;
}

std::unique_ptr<RdKafka::Producer>
EventsCounter::TestUtils::bootstrap_test_kafka_producer(
    const std::string &brokers) {
  string errstr;

  unique_ptr<RdKafka::Conf> conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
  unique_ptr<RdKafka::Conf> topic_conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

  EXPECT_EQ(RdKafka::ErrorCode::ERR_NO_ERROR,
            conf->set("metadata.broker.list", brokers, errstr));
  EXPECT_EQ(RdKafka::ErrorCode::ERR_NO_ERROR,
            topic_conf->set("auto.offset.reset", "smallest", errstr));

  unique_ptr<RdKafka::Producer> producer(
      RdKafka::Producer::create(conf.get(), errstr));
  EXPECT_TRUE(producer.get() != nullptr);

  return producer;
}

std::unique_ptr<RdKafka::Topic>
EventsCounter::TestUtils::bootstrap_test_kafka_topic(
    unique_ptr<RdKafka::Producer> &producer, const std::string &topic_str) {
  string errstr;
  vector<string> topics;
  topics.push_back(topic_str);

  unique_ptr<RdKafka::Conf> topic_conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

  EXPECT_EQ(RdKafka::ErrorCode::ERR_NO_ERROR,
            topic_conf->set("auto.offset.reset", "smallest", errstr));

  unique_ptr<RdKafka::Topic> topic_handler(RdKafka::Topic::create(
      producer.get(), topic_str, topic_conf.get(), errstr));
  EXPECT_TRUE(topic_handler.get() != nullptr);

  return topic_handler;
}
