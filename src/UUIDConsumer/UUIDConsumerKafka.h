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

#pragma once

#include "UUIDConsumer.h"
#include "Utils/UUIDBytes.h"

#include "UUIDConsumer.h"

#include <librdkafka/rdkafkacpp.h>

#include <memory>

namespace EventsCounter {

/**
 * Class for consume and parse JSON messages from Kafka
 */
class UUIDConsumerKafka : public UUIDConsumer {
private:
	std::unique_ptr<RdKafka::KafkaConsumer> kafka_consumer;
	/// UUID field in kafka JSON message
	std::string json_uuid_key;

public:
	UUIDConsumerKafka(const std::vector<std::string> &topics,
			  const std::string json_uuid_key,
			  RdKafka::Conf *t_kafka_consumer_conf);

	~UUIDConsumerKafka();

	UUIDConsumerKafka(UUIDConsumerKafka &&) = delete;
	UUIDConsumerKafka &operator=(const UUIDConsumerKafka &) = delete;
	UUIDConsumerKafka &operator=(const UUIDConsumerKafka &&) = delete;

	/**
	 * Consume a JSON message from Kafka and returns the "sensor_uuid" value
	 * of the message and the number of bytes.
	 *
	 * @param  timeout Max time to wait for a message in milliseconds.
	 * @return         Pair with UUID and number of bytes of the message.
	 */
	UUIDBytes consume(uint32_t timeout_ms) const;
};
};
