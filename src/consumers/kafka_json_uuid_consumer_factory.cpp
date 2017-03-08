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

#include "../utils/kafka_utils.hpp"
#include "kafka_json_uuid_consumer_factory.hpp"

#include <iostream>

using namespace EventsCounter::Consumers;
using namespace EventsCounter::Configuration;
using namespace std;
using namespace RdKafka;

KafkaUUIDConsumerFactory::KafkaUUIDConsumerFactory(
    const std::string t_uuid_key, const std::vector<std::string> t_read_topics,
    kafka_conf_list t_consumer_rk_conf_v, kafka_conf_list t_consumer_rkt_conf_v)
    : uuid_key(t_uuid_key), read_topics(t_read_topics),
      consumer_rk_conf_v(t_consumer_rk_conf_v),
      consumer_rkt_conf_v(t_consumer_rkt_conf_v) {}

unique_ptr<KafkaJSONUUIDConsumer> KafkaUUIDConsumerFactory::create() {
  string errstr;

  unique_ptr<Conf> conf(Conf::create(Conf::CONF_GLOBAL));
  unique_ptr<Conf> tconf(Conf::create(Conf::CONF_TOPIC));

  Utils::rdkafka_set_conf_vector(this->consumer_rkt_conf_v, tconf, "topic");
  Utils::rdkafka_set_conf_vector(this->consumer_rk_conf_v, conf, "kafka");
  conf->set("default_topic_conf", tconf.get(), errstr);

  return unique_ptr<KafkaJSONUUIDConsumer>(
      new KafkaJSONUUIDConsumer(this->read_topics, this->uuid_key, conf.get()));
}
