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
#include "monitor_producer.hpp"

#include <librdkafka/rdkafkacpp.h>

#include <memory>

namespace EventsCounter {
namespace Producers {

class KafkaMonitorProducer : public MonitorProducer {
private:
  std::unique_ptr<RdKafka::Producer> kafka_producer;
  std::unique_ptr<RdKafka::Topic> kafka_topic;

public:
  KafkaMonitorProducer(struct Configuration::counters_monitor_config_s &config);
  KafkaMonitorProducer(KafkaMonitorProducer &) = delete;
  KafkaMonitorProducer(KafkaMonitorProducer &&) = delete;
  KafkaMonitorProducer &operator=(KafkaMonitorProducer &) = delete;
  KafkaMonitorProducer &operator=(KafkaMonitorProducer &&) = delete;
  ~KafkaMonitorProducer() {
    while (this->kafka_producer->outq_len() > 0) {
      this->kafka_producer->poll(100);
    }
  }

  ErrorCode produce(const Utils::UUIDBytes &counter);
  void do_idle_tasks(
      std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
    kafka_producer->poll(timeout.count());
  }
};
};
};
