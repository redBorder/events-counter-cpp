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

#include "UUIDConsumer.h"
#include "UUIDConsumerKafka.h"

using namespace rapidjson;
using namespace std;
using namespace EventsCounter;

/// Wrapper class to give rapidjson zero-copy DOM parsing from a kafka message
template <typename Encoding = UTF8<>> class SizedBufferStream {
public:
	typedef typename Encoding::Ch Ch; //!< Character type of the stream.

	/// Default constructor
	SizedBufferStream(Ch *t_msg, size_t t_msg_size)
	    : payload(t_msg), size(t_msg_size), read_pos(0), write_pos(0) {
	}

	/// Read the current character from stream without moving the read
	/// cursor.
	Ch Peek() const {
		return this->read_pos < this->size
				       ? this->payload[this->read_pos]
				       : '\0';
	}

	/// Read the current character from stream and moving the read cursor to
	/// next character.
	Ch Take() {
		return this->read_pos < this->size
				       ? this->payload[this->read_pos++]
				       : '\0';
	}

	/// Get the current read cursor.
	/// @return Number of characters read from start.

	size_t Tell() {
		return this->read_pos;
	}

	/// Write a character
	void Put(Ch c) {
		if (this->write_pos < this->size) {
			this->payload[this->write_pos++] = c;
		}
	}

	/// Begin writing operation at the current read pointer.
	/// @return The begin writer pointer.
	Ch *PutBegin() {
		return &this->payload[this->write_pos = this->read_pos];
	}
	/// End the writing operation.
	/// @param begin The begin write pointer returned by PutBegin().
	/// @return Number of characters written.
	size_t PutEnd(Ch *begin) {
		return write_pos - static_cast<size_t>(begin - this->payload);
	}

private:
	Ch *payload;
	size_t size, read_pos, write_pos;
};

UUIDBytes UUIDConsumerKafka::consume(uint32_t timeout) const {
	unique_ptr<RdKafka::Message> message(
			this->kafka_consumer->consume(timeout));

	const int err = message->err();
	switch (err) {
	case RdKafka::ERR_NO_ERROR: {
		SizedBufferStream<> is(static_cast<char *>(const_cast<void *>(
						       message->payload())),
				       message->len());
		Document json;
		json.ParseStream<kParseDefaultFlags | kParseInsituFlag>(is);

		if (json.HasParseError()) {
			cerr << "Couldn't parse message JSON" << endl;
			break;
		}

		if (!json.IsObject() || !json.HasMember("sensor_uuid") ||
		    !json["sensor_uuid"].IsString()) {
			break;
		}

		Value &uuid = json["sensor_uuid"];
		std::string uuid_str = uuid.GetString();

		return UUIDBytes(uuid_str, message->len());
	}

	case RdKafka::ERR__TIMED_OUT:
	case RdKafka::ERR__PARTITION_EOF:
	default:
		break;
	}

	return UUIDBytes();
}

/// Creates a new rkt conf with specified broker + group id
static unique_ptr<RdKafka::Conf>
new_rk_conf(const char *brokers, const char *group_id) {
	std::string errstr;
	unique_ptr<RdKafka::Conf> conf(
			RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));

	if (conf->set("group.id", group_id, errstr) != RdKafka::Conf::CONF_OK) {
		throw "Error setting group id";
	}

	if (conf->set("metadata.broker.list", brokers, errstr) !=
	    RdKafka::Conf::CONF_OK) {
		throw "Error setting broker list";
	}

	return conf;
}

UUIDConsumerKafka::UUIDConsumerKafka(const char *brokers,
				     const char *group_id,
				     const vector<string> &topics)
    : UUIDConsumerKafka(topics, new_rk_conf(brokers, group_id).get()) {
}

UUIDConsumerKafka::UUIDConsumerKafka(const vector<string> &topics,
				     RdKafka::Conf *conf) {
	std::string errstr;
	this->kafka_consumer = RdKafka::KafkaConsumer::create(conf, errstr);
	if (!this->kafka_consumer) {
		throw "Failed to create consumer";
	}

	RdKafka::ErrorCode err = this->kafka_consumer->subscribe(topics);
	if (err) {
		throw "Failed to subscribe to topic";
	}
}

UUIDConsumerKafka::~UUIDConsumerKafka() {
	this->kafka_consumer->close();
	delete this->kafka_consumer;
}
