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
#include "kafka_json_uuid_consumer.hpp"

namespace EventsCounter {
namespace Consumers {

typedef std::vector<std::pair<std::string, std::string>> kafka_conf_list;

class KafkaUUIDConsumerFactory {
private:
  std::string uuid_key;
  std::vector<std::string> read_topics;
  kafka_conf_list consumer_rk_conf_v;
  kafka_conf_list consumer_rkt_conf_v;

public:
  KafkaUUIDConsumerFactory(const std::string t_uuid_key,
                           const std::vector<std::string> t_read_topics,
                           kafka_conf_list t_consumer_rk_conf_v,
                           kafka_conf_list t_consumer_rkt_conf_v);
  KafkaUUIDConsumerFactory(KafkaUUIDConsumerFactory &&) = delete;
  KafkaUUIDConsumerFactory &operator=(KafkaUUIDConsumerFactory &) = delete;
  KafkaUUIDConsumerFactory &operator=(KafkaUUIDConsumerFactory &&) = delete;

  std::unique_ptr<Consumers::KafkaJSONUUIDConsumer> create();
};
};
};
