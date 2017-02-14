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

#include "../config/config.hpp"
#include "../utils/uuid_bytes.hpp"
#include "events_consumer.hpp"

#include <librdkafka/rdkafkacpp.h>

#include <memory>

namespace EventsCounter {
namespace Consumers {

class KafkaJSONCounterConsumer : public EventsConsumer {
private:
  std::unique_ptr<RdKafka::KafkaConsumer> kafka_consumer;
  uint64_t period;
  uint64_t offset;

  Utils::UUIDBytes
  get_message_uuid_bytes(std::unique_ptr<RdKafka::Message> &message) const;

public:
  KafkaJSONCounterConsumer(
      struct Configuration::counters_monitor_config_s &config);
  ~KafkaJSONCounterConsumer() {}
  KafkaJSONCounterConsumer(KafkaJSONCounterConsumer &&) = delete;
  KafkaJSONCounterConsumer &
  operator=(const KafkaJSONCounterConsumer &) = delete;
  KafkaJSONCounterConsumer &
  operator=(const KafkaJSONCounterConsumer &&) = delete;

  /**
   * Check if a timestamp belongs to the current interval. Used to discard old
   * counters.
   */
  static bool check_timestamp(Utils::UUIDBytes &uuid_bytes, uint64_t period,
                              uint64_t offset);
  /**
   * Consume a JSON message from Kafka and returns the "uuid" and "value"
   * fields.
   *
   * @param  timeout Max time to wait for a message in milliseconds.
   * @return         Pair with UUID and number of bytes of the message.
   */
  Utils::UUIDBytes consume(uint32_t timeout_ms) const;
};
};
};
