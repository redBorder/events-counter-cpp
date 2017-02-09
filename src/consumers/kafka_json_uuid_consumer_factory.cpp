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

#include "kafka_json_uuid_consumer_factory.hpp"

#include <iostream>

using namespace EventsCounter::Consumers;
using namespace std;
using namespace RdKafka;

/////////////
// Helpers //
/////////////

static void
rdkafka_set_conf_vector(const vector<pair<string, string>> &conf_parameters,
                        Conf *conf, const string &err_conf_type) {
  string errstr;
  for (const auto &itr : conf_parameters) {
    const Conf::ConfResult rc = conf->set(itr.first, itr.second, errstr);
    switch (rc) {
    case Conf::CONF_UNKNOWN:
      cerr << "Unknown " << err_conf_type << " property " << itr.first << ": "
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

//////////////////////////////
// KafkaUUIDConsumerFactory //
//////////////////////////////

KafkaUUIDConsumerFactory::KafkaUUIDConsumerFactory(
    vector<string> t_read_topics, string t_json_uuid_key,
    Configuration::kafka_conf_list t_kafka_consumer_conf,
    Configuration::kafka_conf_list t_kafka_consumer_tconf)
    : m_read_topics(t_read_topics), m_json_uuid_key(t_json_uuid_key),
      m_kafka_consumer_conf(t_kafka_consumer_conf),
      m_kafka_consumer_tconf(t_kafka_consumer_tconf) {}

unique_ptr<KafkaJSONUUIDConsumer> KafkaUUIDConsumerFactory::create() {
  string errstr;

  unique_ptr<Conf> conf(Conf::create(Conf::CONF_GLOBAL)),
      tconf(Conf::create(Conf::CONF_TOPIC));

  rdkafka_set_conf_vector(this->m_kafka_consumer_tconf, tconf.get(), "topic");
  rdkafka_set_conf_vector(this->m_kafka_consumer_conf, conf.get(), "kafka");
  conf->set("default_topic_conf", tconf.get(), errstr);

  return unique_ptr<KafkaJSONUUIDConsumer>(new KafkaJSONUUIDConsumer(
      this->m_read_topics, m_json_uuid_key, conf.get()));
}
