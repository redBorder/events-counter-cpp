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

#include "json_consumer.hpp"

#include <librdkafka/rdkafkacpp.h>

#include <memory>

namespace EventsCounter {
namespace Consumers {

class KafkaJSONCounterConsumer : public JSONConsumer {
private:
  std::unique_ptr<RdKafka::KafkaConsumer> kafka_consumer;

public:
  KafkaJSONCounterConsumer(const std::string &topic, RdKafka::Conf *conf);
  ~KafkaJSONCounterConsumer();
  KafkaJSONCounterConsumer(KafkaJSONCounterConsumer &&) = delete;
  KafkaJSONCounterConsumer &
  operator=(const KafkaJSONCounterConsumer &) = delete;
  KafkaJSONCounterConsumer &
  operator=(const KafkaJSONCounterConsumer &&) = delete;

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
