/*
  Copyright (C) 2017 Eneo Tecnologia S.L.
  Authors: Diego Fernandez <bigomby@gmail.com>
     Eugenio Perez <eupm90@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "LeadershipMonitorKafka.h"

#include <iostream>

using namespace EventsCounter;
using namespace RdKafka;
using namespace std;

LeadershipMonitorKafka::LeadershipMonitorKafka(const string &brokers,
					       const string &topic,
					       const string &group) {
	vector<string> topics;
	topics.push_back(topic);
	string errstr;
	Conf::ConfResult rc;

	unique_ptr<Conf> conf(Conf::create(Conf::CONF_GLOBAL));
	unique_ptr<Conf> tconf(Conf::create(Conf::CONF_TOPIC));

	rc = conf->set("rebalance_cb", this, errstr);
	if (rc != Conf::CONF_OK) {
		throw SetRebalanceCBException(errstr);
	}
	rc = conf->set("group.id", group, errstr);
	if (rc != Conf::CONF_OK) {
		throw SetGroupIDException(errstr);
	}
	rc = conf->set("metadata.broker.list", brokers, errstr);
	if (rc != Conf::CONF_OK) {
		throw SetBrokersException(errstr);
	}
	rc = conf->set("default_topic_conf", tconf.get(), errstr);
	if (rc != Conf::CONF_OK) {
		throw SetRebalanceCBException(errstr);
	}

	this->kafka_consumer.reset(KafkaConsumer::create(conf.get(), errstr));
	if (!this->kafka_consumer) {
		throw CreateConsumerException("Failed to create consumer");
	}

	this->kafka_consumer->subscribe(topics);
	if (!this->kafka_consumer) {
		throw SubscribeException("Failed to create subscribe");
	}
}

LeadershipMonitorKafka::~LeadershipMonitorKafka() {
	this->kafka_consumer->close();
}

bool LeadershipMonitorKafka::check_leadership(
		chrono::milliseconds timeout) const {
	do {
		unique_ptr<Message> message(
				this->kafka_consumer->consume(timeout.count()));
		if (message->err() == ERR__TIMED_OUT) {
			break;
		}

		timeout = chrono::milliseconds(0);
	} while (true);

	return this->is_leader;
}

void LeadershipMonitorKafka::rebalance_cb(
		KafkaConsumer *,
		ErrorCode err,
		vector<TopicPartition *> &partitions) {

	this->is_leader =
			err == ERR__ASSIGN_PARTITIONS && partitions.size() > 0;
}
