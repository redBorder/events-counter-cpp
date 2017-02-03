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

#include "leadership_monitor.hpp"

#include <librdkafka/rdkafkacpp.h>

#include <exception>
#include <memory>

namespace EventsCounter {
namespace UUIDCountersMonitor {

class KafkaLeadershipMonitorException : public std::exception {
private:
  const std::string errstr;

protected:
  KafkaLeadershipMonitorException(std::string t_errstr) : errstr(t_errstr) {}

public:
  virtual const char *what() const throw() {
    return ("KafkaLeadershipMonitorException: " + errstr).c_str();
  }
};

class SetRebalanceCBException : public KafkaLeadershipMonitorException {
public:
  SetRebalanceCBException(std::string t_errstr)
      : KafkaLeadershipMonitorException(t_errstr) {}
};

class SetGroupIDException : public KafkaLeadershipMonitorException {
public:
  SetGroupIDException(std::string t_errstr)
      : KafkaLeadershipMonitorException(t_errstr) {}
};

class SetBrokersException : public KafkaLeadershipMonitorException {
public:
  SetBrokersException(std::string t_errstr)
      : KafkaLeadershipMonitorException(t_errstr) {}
};

class CreateConsumerException : public KafkaLeadershipMonitorException {
public:
  CreateConsumerException(std::string t_errstr)
      : KafkaLeadershipMonitorException(t_errstr) {}
};

class SetDefaultTopicConfException : public KafkaLeadershipMonitorException {
public:
  SetDefaultTopicConfException(std::string t_errstr)
      : KafkaLeadershipMonitorException(t_errstr) {}
};

class SubscribeException : public KafkaLeadershipMonitorException {
public:
  SubscribeException(std::string t_errstr)
      : KafkaLeadershipMonitorException(t_errstr) {}
};

/**
 * Class used to select a leader among other instances based on the ownership
 * of a Kafka partition. The consumer that owns the single partitions of a topic
 * is considered the leader of the group.
 */
class KafkaLeadershipMonitor : public LeadershipMonitor,
                               public RdKafka::RebalanceCb {
private:
  bool is_leader = false;
  std::unique_ptr<RdKafka::KafkaConsumer> kafka_consumer;

  /**
   * Callback triggered when a rebalance occurs. In this particular case
   * is used to select a leadar among other instances. The topic MUST have
   * only one partition.
   *
   * The consumer that owns the single partitions is considered the leader
   * of the group.
   *
   * @param consumer   Consumer than handles the rebalance (unused)
   * @param err        Information about the rebalance result
   * @param partitions Partitions that should be assigned to the consumer
   */
  void rebalance_cb(RdKafka::KafkaConsumer *consumer, RdKafka::ErrorCode err,
                    std::vector<RdKafka::TopicPartition *> &partitions);

public:
  KafkaLeadershipMonitor(const std::string &brokers, const std::string &topic,
                         const std::string &group);
  ~KafkaLeadershipMonitor();

  /**
   * Returns if the instance is the leader or not.
   *
   * @param  timeout_ms Max time to wait for a rebalance.
   * @return            True if leadership has been acquired. Flase in
   *                    other case
   */
  bool check_leadership(std::chrono::milliseconds timeout_ms) const;
};
};
};
