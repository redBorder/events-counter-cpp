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
#include "kafka_json_counter_producer_factory.hpp"

#include <iostream>

using namespace EventsCounter::Producers;
using namespace EventsCounter::Configuration;
using namespace RdKafka;
using namespace std;

static void
rdkafka_set_conf_vector(const vector<pair<string, string>> &conf_parameters,
                        Conf *conf, const string &err_conf_type) {
  string errstr;
  for (const auto &itr : conf_parameters) {
    const Conf::ConfResult rc = conf->set(itr.first, itr.second, errstr);
    switch (rc) {
    case Conf::CONF_UNKNOWN:
      cerr << "Unknown " << err_conf_type << " property " << itr.first << ":"
           << errstr << endl;
      continue;

    case Conf::CONF_INVALID:
      cerr << "Unknown " << err_conf_type << " property value " << itr.second
           << " for key " << itr.first << ": " << errstr << endl;
      continue;

    case Conf::CONF_OK:
    default:
      break;
    };
  }
}

KafkaJSONCounterProducerFactory::KafkaJSONCounterProducerFactory(
    uuid_counter_config_s t_uuid_counter_config)
    : uuid_counter_config(t_uuid_counter_config) {
  string errstr;

  unique_ptr<RdKafka::Conf> conf(
      RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)),
      tconf(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

  rdkafka_set_conf_vector(
      this->uuid_counter_config.kafka_config.consumer_rk_conf_v, conf.get(),
      "kafka");
  rdkafka_set_conf_vector(
      this->uuid_counter_config.kafka_config.consumer_rkt_conf_v, tconf.get(),
      "topic");

  // TODO
  // conf->set("dr_cb", &events_counter_cb, errstr);

  std::unique_ptr<RdKafka::Producer> rk(
      RdKafka::Producer::create(conf.get(), errstr));
  std::unique_ptr<RdKafka::Topic> rkt(RdKafka::Topic::create(
      rk.get(), this->uuid_counter_config.write_topic, tconf.get(), errstr));

  this->producer =
      make_shared<KafkaJSONCounterProducer>(rk.release(), rkt.release());
}

shared_ptr<KafkaJSONCounterProducer> KafkaJSONCounterProducerFactory::create() {
  return this->producer;
}
