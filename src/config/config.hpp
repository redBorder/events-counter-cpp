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

#include "../consumers/kafka_json_uuid_consumer.hpp"
#include "../producers/kafka_json_counter_producer.hpp"
#include "../uuid_counters_db/uuid_counters_db.hpp"

#include <rapidjson/document.h>

#include <chrono>
#include <memory>
#include <vector>

namespace EventsCounter {
namespace Configuration {

typedef std::vector<std::pair<std::string, std::string>> kafka_conf_list;

struct kafka_config_s {
  kafka_conf_list consumer_rk_conf_v;
  kafka_conf_list consumer_rkt_conf_v;

  kafka_conf_list producer_rk_conf_v;
  kafka_conf_list producer_rkt_conf_v;
};

struct uuid_counter_config_s {
  kafka_config_s kafka_config;
  std::vector<std::string> read_topics;
  std::string write_topic;
  std::string uuid_key;
  std::chrono::seconds update_period;
};

struct counters_monitor_config_s {
  kafka_config_s kafka_config;
  std::string read_topic;
  std::string write_topic;
  std::chrono::seconds period;
  std::chrono::seconds offset;
  UUIDCountersDB::UUIDCountersDB::counters_t limits;
};

class Config {
public:
  virtual ~Config() = default;

  virtual const std::vector<std::string> &counters_uuids() = 0;
  virtual struct uuid_counter_config_s get_uuid_counter_config() = 0;
  virtual struct counters_monitor_config_s get_monitor_config() = 0;
};
};
};
