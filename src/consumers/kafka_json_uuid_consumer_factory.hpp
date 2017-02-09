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

#include "../config/json_config.hpp"
#include "kafka_json_uuid_consumer.hpp"

namespace EventsCounter {
namespace Consumers {

class KafkaUUIDConsumerFactory {
private:
  std::vector<std::string> m_read_topics;
  std::string m_json_uuid_key;
  Configuration::kafka_conf_list m_kafka_consumer_conf;
  Configuration::kafka_conf_list m_kafka_consumer_tconf;

public:
  KafkaUUIDConsumerFactory(
      std::vector<std::string> t_read_topics, std::string t_json_uuid_key,
      Configuration::kafka_conf_list t_kafka_consumer_conf,
      Configuration::kafka_conf_list t_kafka_consumer_tconf);
  KafkaUUIDConsumerFactory(KafkaUUIDConsumerFactory &&) = delete;
  KafkaUUIDConsumerFactory &operator=(KafkaUUIDConsumerFactory &) = delete;
  KafkaUUIDConsumerFactory &operator=(KafkaUUIDConsumerFactory &&) = delete;

  std::unique_ptr<Consumers::KafkaJSONUUIDConsumer> create();
};
};
};
