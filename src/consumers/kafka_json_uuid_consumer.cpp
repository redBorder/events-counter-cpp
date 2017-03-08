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

#include <iostream>
#include <memory>

#include "../utils/json_zerocopy.hpp"
#include "kafka_json_uuid_consumer.hpp"

using namespace std;
using namespace EventsCounter;
using namespace EventsCounter::Consumers;
using namespace rapidjson;
using namespace RdKafka;

/// TODO improve error messages
Utils::UUIDBytes KafkaJSONUUIDConsumer::get_message_uuid_bytes(
    const std::string &t_json_uuid_key, const RdKafka::Message *message) const {
  Utils::JSON json(static_cast<char *>(message->payload()), message->len());

  if (json.HasParseError()) {
    cerr << "Couldn't parse message JSON" << endl;
    return Utils::UUIDBytes();
  }

  if (!json.IsObject() || !json.HasMember(t_json_uuid_key.c_str()) ||
      !json[t_json_uuid_key.c_str()].IsString()) {
    return Utils::UUIDBytes();
  }

  Value &uuid = json[t_json_uuid_key.c_str()];
  string uuid_str = uuid.GetString();

  return Utils::UUIDBytes(uuid_str, message->len());
}

Utils::UUIDBytes KafkaJSONUUIDConsumer::consume(uint32_t timeout) const {
  unique_ptr<Message> message(this->kafka_consumer->consume(timeout));

  const int err = message->err();
  switch (err) {
  case ERR_NO_ERROR:
    return get_message_uuid_bytes(this->json_uuid_key, message.get());

  case ERR__TIMED_OUT:
  case ERR__PARTITION_EOF:
  default:
    break;
  }

  return Utils::UUIDBytes();
}

KafkaJSONUUIDConsumer::KafkaJSONUUIDConsumer(const vector<string> &topics,
                                             const string t_json_uuid_key,
                                             RdKafka::Conf *conf)
    : json_uuid_key(t_json_uuid_key) {
  std::string errstr;
  this->kafka_consumer =
      unique_ptr<KafkaConsumer>(KafkaConsumer::create(conf, errstr));
  if (!this->kafka_consumer) {
    throw CreateConsumerException(errstr.c_str());
  }

  ErrorCode err = this->kafka_consumer->subscribe(topics);
  if (err) {
    throw SubscribeException(errstr.c_str());
  }
}

KafkaJSONUUIDConsumer::~KafkaJSONUUIDConsumer() {
  this->kafka_consumer->close();
}
