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

#include "UUIDKafkaTopicProducer.h"

#include <chrono>
#include <iostream>

using namespace EventsCounter;
using namespace std;
using namespace std::chrono;

UUIDProducer::ErrorCode
UUIDKafkaTopicProducer::produce(const Utils::UUIDBytes &counter,
				chrono::seconds timestamp) {
	/// @TODO extract JSON formatting
	const string s = string("{\"monitor\":\"sensor_received_bytes\"") +
			 ",\"uuid\":\"" + counter.get_uuid() + "\",\"value\":" +
			 to_string(counter.get_bytes()) + ",\"timestamp\":" +
			 to_string(timestamp.count()) + '}';

	const RdKafka::ErrorCode produce_rc = this->kafka_producer->produce(
			this->kafka_topic.get(),
			RdKafka::Topic::PARTITION_UA,
			RdKafka::Producer::RK_MSG_COPY /* Copy payload */,
			const_cast<char *>(s.c_str()),
			s.size(),
			NULL,
			NULL);

	switch (produce_rc) {
	case RdKafka::ERR__QUEUE_FULL:
		return ERR_QUEUE_FULL;
	case RdKafka::ERR_MSG_SIZE_TOO_LARGE:
		cout << "Producing a too large message!";
		return ERR_MSG_TOO_LARGE;
	case RdKafka::ERR__UNKNOWN_PARTITION:
	case RdKafka::ERR__UNKNOWN_TOPIC:
	/// @TODO launch exceptions?
	default:
		return NO_ERROR;
	};
}
