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

UUIDBytes UUIDConsumerKafka::consume(uint32_t timeout) const {
	unique_ptr<Message> message(this->kafka_consumer->consume(timeout));

	const int err = message->err();
	switch (err) {
	case ERR_NO_ERROR: {
		JSON json(static_cast<char *>(const_cast<void *>(
					  message->payload())),
			  message->len());

		if (json.HasParseError()) {
			cerr << "Couldn't parse message JSON" << endl;
			break;
		}

		if (!json.IsObject() || !json.HasMember("sensor_uuid") ||
		    !json["sensor_uuid"].IsString()) {
			break;
		}

		Value &uuid = json["sensor_uuid"];
		string uuid_str = uuid.GetString();

		return UUIDBytes(uuid_str, message->len());
	}

	case ERR__TIMED_OUT:
	case ERR__PARTITION_EOF:
	default:
		break;
	}

	return UUIDBytes();
}

/// Creates a new rkt conf with specified broker + group id
static unique_ptr<Conf> new_rk_conf(const char *brokers, const char *group_id) {
	std::string errstr;
	unique_ptr<Conf> conf(Conf::create(Conf::CONF_GLOBAL));

	if (conf->set("group.id", group_id, errstr) != Conf::CONF_OK) {
		throw "Error setting group id";
	}

	if (conf->set("metadata.broker.list", brokers, errstr) !=
	    Conf::CONF_OK) {
		throw "Error setting broker list";
	}

	return conf;
}

UUIDConsumerKafka::UUIDConsumerKafka(const char *brokers,
				     const char *group_id,
				     const vector<string> &topics)
    : UUIDConsumerKafka(topics, new_rk_conf(brokers, group_id).get()) {
}

UUIDConsumerKafka::UUIDConsumerKafka(const vector<string> &topics, Conf *conf) {
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
