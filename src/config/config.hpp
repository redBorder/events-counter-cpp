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

#include "../producers/kafka_json_counter_producer.hpp"
#include "../uuid_consumer/uuid_consumer.hpp"
#include "../uuid_counters_db/uuid_counters_db.hpp"

#include <rapidjson/document.h>

#include <chrono>
#include <iostream> // @TODO delete
#include <memory>
#include <vector>

namespace EventsCounter {
namespace Configuration {

// TODO this should be splitted in conf and JSON parser that produces a conf
// object
class Config {
public:
  virtual ~Config() {}
  virtual std::unique_ptr<UUIDConsumer::UUIDConsumer>
  get_counters_consumer() = 0;

  virtual std::shared_ptr<Producers::KafkaJSONCounterProducer>
  get_counters_producer() = 0;

  /// Get counters interval period
  virtual std::chrono::seconds get_counters_timer_period() = 0;

  /// Get counters interval offset to launch
  virtual std::chrono::seconds get_counters_timer_offset() = 0;

  virtual const std::vector<std::string> &counters_uuids() = 0;

protected:
  Config() {}
};

class JsonConfig : public Config {
public:
  class JSONParserException : public std::exception {
  private:
    const std::string m_what;

  public:
    JSONParserException(const char *t_what) : m_what(t_what) {}

    JSONParserException(const std::string &t_what)
        : JSONParserException(t_what.c_str()) {}

    virtual const char *what() const noexcept { return m_what.c_str(); }
  };

  class UUIDConsumerFactory {
  public:
    virtual std::unique_ptr<UUIDConsumer::UUIDConsumer> create() = 0;
    virtual ~UUIDConsumerFactory() {}
  };

  static JsonConfig *json_parse(const std::string &json_text);
  virtual std::unique_ptr<UUIDConsumer::UUIDConsumer> get_counters_consumer() {
    return this->m_counters.consumer_factory->create();
  }

  virtual std::shared_ptr<Producers::KafkaJSONCounterProducer>
  get_counters_producer() {
    return this->m_counters.producer;
  }

  virtual std::chrono::seconds get_counters_timer_period() {
    return this->m_counters.period;
  }
  /// Get counters interval offset to launch
  virtual std::chrono::seconds get_counters_timer_offset() {
    return this->m_counters.offset;
  }

  virtual const std::vector<std::string> &counters_uuids() {
    return m_counters_uuid;
  };

private:
  JsonConfig() {}

  /// Configuration for an UUID forwarder
  struct forwarder_config {

    /// UUID consumer
    std::unique_ptr<UUIDConsumerFactory> consumer_factory;

    /// UUID producer
    std::shared_ptr<Producers::KafkaJSONCounterProducer> producer;

    /// Time to produce
    std::chrono::seconds period, offset;
  } m_counters;
  static void parse_counter_consumer_configuration(
      const rapidjson::Value::ConstObject &counters_config,
      struct JsonConfig::forwarder_config &fw_config);
  std::vector<std::string> m_counters_uuid;
};
};
};
