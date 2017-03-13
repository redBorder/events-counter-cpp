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

#include "../config/config.hpp"
#include "kafka_json_counter_producer.hpp"

namespace EventsCounter {
namespace Producers {

class KafkaJSONCounterProducerFactory {
private:
  Configuration::uuid_counter_config_s uuid_counter_config;
  std::shared_ptr<KafkaJSONCounterProducer> producer;

public:
  KafkaJSONCounterProducerFactory(
      Configuration::uuid_counter_config_s t_uuid_counter_config);
  KafkaJSONCounterProducerFactory(KafkaJSONCounterProducerFactory &&) = delete;
  KafkaJSONCounterProducerFactory &
  operator=(KafkaJSONCounterProducerFactory &) = delete;
  KafkaJSONCounterProducerFactory &
  operator=(KafkaJSONCounterProducerFactory &&) = delete;

  std::shared_ptr<KafkaJSONCounterProducer> create();
};
};
};
