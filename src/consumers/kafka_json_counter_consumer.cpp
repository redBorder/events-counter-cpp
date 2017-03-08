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

#include "../utils/json_zerocopy.hpp"
#include "../utils/kafka_utils.hpp"
#include "kafka_json_consumer_exceptions.hpp"

#include "kafka_json_counter_consumer.hpp"

#include <chrono>
#include <iostream>

using namespace EventsCounter::Consumers;
using namespace EventsCounter::Utils;
using namespace EventsCounter::Configuration;
using namespace std::chrono;
using namespace rapidjson;
using namespace RdKafka;
using namespace std;

KafkaJSONCounterConsumer::KafkaJSONCounterConsumer(
    uint64_t t_period, uint64_t t_offset, const string &read_topic,
    kafka_conf_list_t t_rk_conf_v, kafka_conf_list_t t_rkt_conf_v)
    : period(t_period), offset(t_offset) {
  string errstr;

  vector<string> topics;
  topics.push_back(read_topic);

  const unique_ptr<Conf> conf(Conf::create(Conf::CONF_GLOBAL));
  unique_ptr<Conf> tconf(Conf::create(Conf::CONF_TOPIC));

  rdkafka_set_conf_vector(t_rk_conf_v, conf, "kafka");
  rdkafka_set_conf_vector(t_rkt_conf_v, tconf, "topic");

  conf->set("default_topic_conf", tconf.get(), errstr);

  this->kafka_consumer =
      unique_ptr<KafkaConsumer>(KafkaConsumer::create(conf.get(), errstr));
  if (!this->kafka_consumer) {
    throw CreateConsumerException(errstr.c_str());
  }

  RdKafka::ErrorCode err = this->kafka_consumer->subscribe(topics);
  if (err) {
    throw SubscribeException(errstr.c_str());
  }
}

bool KafkaJSONCounterConsumer::check_timestamp(UUIDBytes &uuid_bytes,
                                               uint64_t period,
                                               uint64_t offset) {
  system_clock::time_point tp = system_clock::now();
  system_clock::duration dtn = tp.time_since_epoch();

  uint64_t interval_start;
  uint64_t now =
      dtn.count() * system_clock::period::num / system_clock::period::den;
  uint64_t period_start = (now - now % period);

  if (now > period_start + offset) {
    interval_start = period_start + offset;
  } else {
    interval_start = period_start - period + offset;
  }

  if (uuid_bytes.get_timestamp() < interval_start) {
    return false;
  }

  return true;
}

UUIDBytes KafkaJSONCounterConsumer::consume(uint32_t timeout) const {
  unique_ptr<Message> message(this->kafka_consumer->consume(timeout));

  const int err = message->err();
  if (err != ERR_NO_ERROR) {
    return Utils::UUIDBytes();
  }

  UUIDBytes uuid_bytes(get_message_uuid_bytes(message));
  if (!check_timestamp(uuid_bytes, this->period, this->offset)) {
    return Utils::UUIDBytes();
  };

  return uuid_bytes;
}

UUIDBytes KafkaJSONCounterConsumer::get_message_uuid_bytes(
    unique_ptr<Message> &message) const {
  Utils::JSON json(static_cast<char *>(message->payload()), message->len());

  if (json.HasParseError() || !json.IsObject()) {
    cerr << "Invalid JSON message" << endl;
    return Utils::UUIDBytes();
  }

  if (!json.HasMember("uuid") || !json["uuid"].IsString()) {
    cerr << "Unknown JSON format (missing uuid)" << endl;
    return Utils::UUIDBytes();
  }
  if (!json.HasMember("value") || !json["value"].IsNumber()) {
    cerr << "Unknown JSON format (missing value)" << endl;
    return Utils::UUIDBytes();
  }
  if (!json.HasMember("timestamp") || !json["timestamp"].IsNumber()) {
    cerr << "Unknown JSON format (missing timestamp)" << endl;
    return Utils::UUIDBytes();
  }

  Value &uuid = json["uuid"];
  Value &bytes = json["value"];
  Value &timestamp = json["timestamp"];
  string uuid_str = uuid.GetString();
  uint64_t bytes_number = bytes.GetUint64();
  uint64_t timestamp_number = timestamp.GetUint64();

  return Utils::UUIDBytes(uuid_str, bytes_number, timestamp_number);
}
