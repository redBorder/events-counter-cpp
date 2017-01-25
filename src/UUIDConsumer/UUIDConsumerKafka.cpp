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

#include <iostream>
#include <memory>

#include "../Utils/JSON.h"
#include "UUIDConsumer.h"
#include "UUIDConsumerKafka.h"

using namespace std;
using namespace EventsCounter;
using namespace rapidjson;
using namespace RdKafka;

/// @TODO improve error messages
UUIDBytes CounterUUIDJSONKafkaConsumer::get_message_uuid_bytes(
		const std::string &t_json_uuid_key,
		const RdKafka::Message *message) const {
	JSON json(static_cast<char *>(const_cast<void *>(message->payload())),
		  message->len());

	if (json.HasParseError()) {
		cerr << "Couldn't parse message JSON" << endl;
		return UUIDBytes();
	}

	if (!json.IsObject() || !json.HasMember(t_json_uuid_key.c_str()) ||
	    !json[t_json_uuid_key.c_str()].IsString()) {
		return UUIDBytes();
	}

	Value &uuid = json[t_json_uuid_key.c_str()];
	string uuid_str = uuid.GetString();

	return UUIDBytes(uuid_str, message->len());
}

UUIDBytes UUIDConsumerKafka::consume(uint32_t timeout) const {
	unique_ptr<Message> message(this->kafka_consumer->consume(timeout));

	const int err = message->err();
	switch (err) {
	case ERR_NO_ERROR:
		return get_message_uuid_bytes(this->json_uuid_key,
					      message.get());

	case ERR__TIMED_OUT:
	case ERR__PARTITION_EOF:
	default:
		break;
	}

	return UUIDBytes();
}

UUIDConsumerKafka::UUIDConsumerKafka(const vector<string> &topics,
				     const string t_json_uuid_key,
				     RdKafka::Conf *conf)
    : json_uuid_key(t_json_uuid_key) {
	std::string errstr;
	this->kafka_consumer = unique_ptr<KafkaConsumer>(
			KafkaConsumer::create(conf, errstr));
	if (!this->kafka_consumer) {
		throw "Failed to create consumer";
	}

	ErrorCode err = this->kafka_consumer->subscribe(topics);
	if (err) {
		throw "Failed to subscribe to topic";
	}
}

UUIDConsumerKafka::~UUIDConsumerKafka() {
	this->kafka_consumer->close();
}
