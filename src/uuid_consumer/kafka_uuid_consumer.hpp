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

#include "../utils/uuid_bytes.hpp"
#include "uuid_consumer.hpp"

#include "uuid_consumer.hpp"

#include <librdkafka/rdkafkacpp.h>

#include <memory>

namespace EventsCounter {
namespace UUIDConsumer {

/**
 * Class for consume and parse JSON messages from Kafka
 */
class KafkaUUIDConsumer : public UUIDConsumer {
private:
  std::unique_ptr<RdKafka::KafkaConsumer> kafka_consumer;
  /// UUID field in kafka JSON message
  std::string json_uuid_key;

protected:
  virtual Utils::UUIDBytes
  get_message_uuid_bytes(const std::string &json_uuid_key,
                         const RdKafka::Message *message) const = 0;

public:
  KafkaUUIDConsumer(const std::vector<std::string> &topics,
                    const std::string json_uuid_key,
                    RdKafka::Conf *t_kafka_consumer_conf);

  ~KafkaUUIDConsumer();

  KafkaUUIDConsumer(KafkaUUIDConsumer &&) = delete;
  KafkaUUIDConsumer &operator=(const KafkaUUIDConsumer &) = delete;
  KafkaUUIDConsumer &operator=(const KafkaUUIDConsumer &&) = delete;

  /**
   * Consume a JSON message from Kafka and returns the "sensor_uuid" value
   * of the message and the number of bytes.
   *
   * @param  timeout Max time to wait for a message in milliseconds.
   * @return         Pair with UUID and number of bytes of the message.
   */
  Utils::UUIDBytes consume(uint32_t timeout_ms) const;
};

/// Transforms counter kafka input message in UUIDBytes format
class CounterUUIDJSONKafkaConsumer : public KafkaUUIDConsumer {
public:
  using KafkaUUIDConsumer::KafkaUUIDConsumer;

private:
  virtual Utils::UUIDBytes
  get_message_uuid_bytes(const std::string &json_uuid_key,
                         const RdKafka::Message *message) const;
};
};
};
