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

#include "UUIDBytes.h"

#include <librdkafka/rdkafkacpp.h>

namespace EventsCounter {

class UUIDConsumerKafka {
private:
	RdKafka::KafkaConsumer *kafka_consumer;

public:
	/**
	 *
	 */
	UUIDConsumerKafka(const char *brokers,
			  const char *group_id,
			  const std::vector<std::string> &topic);

	/**
	 *
	 */
	UUIDConsumerKafka(const std::vector<std::string> &topics,
			  RdKafka::Conf *t_kafka_consumer_conf);

	/**
	 *
	 */
	~UUIDConsumerKafka();

	/**
	 *
	 */
	UUIDConsumerKafka &operator=(const UUIDConsumerKafka &) = delete;

	/**
	 * [consume description]
	 * @param  timeout [description]
	 * @return         [description]
	 */
	UUIDBytes consume(uint32_t timeout) const;
};
};
