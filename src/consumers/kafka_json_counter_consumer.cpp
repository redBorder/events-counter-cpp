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
#include "kafka_json_consumer_exceptions.hpp"

#include "kafka_json_counter_consumer.hpp"

#include <iostream>

using namespace EventsCounter::Consumers;
using namespace EventsCounter::Utils;
using namespace rapidjson;
using namespace RdKafka;
using namespace std;

KafkaJSONCounterConsumer::KafkaJSONCounterConsumer(const string &topic,
                                                   RdKafka::Conf *conf) {
  string errstr;
  vector<string> topics;
  topics.push_back(topic);

  this->kafka_consumer =
      unique_ptr<KafkaConsumer>(KafkaConsumer::create(conf, errstr));
  if (!this->kafka_consumer) {
    throw CreateConsumerException(errstr);
  }

  ErrorCode err = this->kafka_consumer->subscribe(topics);
  if (err) {
    throw SubscribeException(errstr);
  }
}

UUIDBytes KafkaJSONCounterConsumer::consume(uint32_t timeout) const {
  unique_ptr<Message> message(this->kafka_consumer->consume(timeout));

  const int err = message->err();
  switch (err) {
  case ERR_NO_ERROR:
    return get_message_uuid_bytes(message);

  case ERR__TIMED_OUT:
  case ERR__PARTITION_EOF:
  default:
    break;
  }

  return Utils::UUIDBytes();
}

UUIDBytes KafkaJSONCounterConsumer::get_message_uuid_bytes(
    unique_ptr<Message> &message) const {
  Utils::JSON json(static_cast<char *>(message->payload()), message->len());

  if (json.HasParseError() || !json.IsObject()) {
    cerr << "Invalid JSON message" << endl;
    return Utils::UUIDBytes();
  }

  if (!json.HasMember("uuid") || !json["uuid"].IsString()) {
    return Utils::UUIDBytes();
  }
  if (!json.HasMember("bytes") || !json["bytes"].IsNumber()) {
    return Utils::UUIDBytes();
  }

  Value &uuid = json["uuid"];
  Value &bytes = json["bytes"];
  string uuid_str = uuid.GetString();
  uint64_t bytes_number = bytes.GetUint64();

  return Utils::UUIDBytes(uuid_str, bytes_number);
}
