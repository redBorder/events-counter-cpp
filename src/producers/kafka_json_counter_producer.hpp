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
#include "monitor_producer.hpp"

#include <librdkafka/rdkafkacpp.h>

#include <memory>

namespace EventsCounter {
namespace Producers {

class KafkaJSONCounterProducer : public MonitorProducer {
public:
  KafkaJSONCounterProducer(RdKafka::Producer *t_kafka_producer,
                           RdKafka::Topic *t_kafka_topic)
      : kafka_producer(t_kafka_producer), kafka_topic(t_kafka_topic) {}

  ~KafkaJSONCounterProducer() {
    while (this->kafka_producer->outq_len() > 0) {
      this->kafka_producer->poll(100);
    }
  }

  ErrorCode produce(const Utils::UUIDBytes &counter,
                    const std::chrono::seconds duration);

  KafkaJSONCounterProducer(KafkaJSONCounterProducer &) = delete;
  KafkaJSONCounterProducer(KafkaJSONCounterProducer &&) = delete;
  KafkaJSONCounterProducer &operator=(KafkaJSONCounterProducer &) = delete;
  KafkaJSONCounterProducer &operator=(KafkaJSONCounterProducer &&) = delete;

  void do_idle_tasks(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
    kafka_producer->poll(timeout.count());
  }

private:
  std::unique_ptr<RdKafka::Producer> kafka_producer;
  std::unique_ptr<RdKafka::Topic> kafka_topic;
};
};
};
