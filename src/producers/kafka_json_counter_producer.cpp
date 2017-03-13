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

#include "../writters/json_counter_writter.hpp"
#include "kafka_json_counter_producer.hpp"

#include <chrono>
#include <iostream>

using namespace EventsCounter::Formatters;
using namespace EventsCounter::Producers;
using namespace std;
using namespace std::chrono;

ErrorCode KafkaJSONCounterProducer::produce(const Utils::UUIDBytes &counter,
                                            chrono::seconds) {
  unique_ptr<JSONKafkaMessage> message(
      new JSONKafkaMessage(counter.get_uuid()));
  JSONCounterWritter<>(counter, message->string_buffer);

  const RdKafka::ErrorCode produce_rc = this->kafka_producer->produce(
      this->kafka_topic.get(), RdKafka::Topic::PARTITION_UA,
      RdKafka::Producer::RK_MSG_COPY, message->payload(), message->len(), NULL,
      NULL);

  switch (produce_rc) {
  case RdKafka::ERR__QUEUE_FULL:
    return ERR_QUEUE_FULL;

  case RdKafka::ERR_MSG_SIZE_TOO_LARGE:
    cout << "Producing a too large message!";
    return ERR_MSG_TOO_LARGE;

  case RdKafka::ERR__UNKNOWN_PARTITION:
  case RdKafka::ERR__UNKNOWN_TOPIC:
  default:
    return NO_ERROR;
  };
}
