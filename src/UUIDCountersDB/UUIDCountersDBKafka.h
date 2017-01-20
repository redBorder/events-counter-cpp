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

#include "UUIDCountersDB.h"

#include <librdkafka/rdkafkacpp.h>

#include <memory>
#include <vector>

namespace EventsCounter {

class CounterToKafkaFormatter {
public:
	virtual ~CounterToKafkaFormatter() = default;
	virtual std::unique_ptr<RdKafka::Message>
	format(const std::string &uuid, const uint64_t bytes) = 0;

	static std::unique_ptr<CounterToKafkaFormatter> create();
};

}; // namespace EventsCounter
