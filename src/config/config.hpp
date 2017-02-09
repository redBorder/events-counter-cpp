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

#pragma once

#include "../consumers/kafka_json_uuid_consumer.hpp"
#include "../producers/kafka_json_counter_producer.hpp"
#include "../uuid_counters_db/uuid_counters_db.hpp"

#include <rapidjson/document.h>

#include <chrono>
#include <memory>
#include <vector>

namespace EventsCounter {
namespace Configuration {

class Config {
public:
  virtual ~Config() = default;

  // virtual std::unique_ptr<Consumers::KafkaJSONUUIDConsumer>
  // get_counters_consumer() = 0;

  virtual std::shared_ptr<Producers::KafkaJSONCounterProducer>
  get_counters_producer() = 0;

  /// Get counters interval period
  virtual std::chrono::seconds get_counters_timer_period() = 0;

  /// Get counters interval offset to launch
  virtual std::chrono::seconds get_counters_timer_offset() = 0;

  virtual const std::vector<std::string> &counters_uuids() = 0;
};
};
};
