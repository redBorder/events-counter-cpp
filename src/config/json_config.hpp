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

#include "config.hpp"

namespace EventsCounter {
namespace Configuration {

////////////////
// Exceptions //
////////////////

class JSONParserException : public std::exception {
private:
  const std::string m_what;

public:
  JSONParserException(const char *t_what) : m_what(t_what) {}

  JSONParserException(const std::string &t_what)
      : JSONParserException(t_what.c_str()) {}

  virtual const char *what() const noexcept { return m_what.c_str(); }
};

class JSONUnexpectedTypeException : public JSONParserException {
public:
  /// Inherit constructor
  using JSONParserException::JSONParserException;
};

class JSONChildNotFoundException : public JSONParserException {
public:
  /// Inherit constructor
  using JSONParserException::JSONParserException;
};

/////////////
// Structs //
/////////////

/// Configuration for an UUID forwarder
struct forwarder_config {
  /// UUID consumer
  // std::unique_ptr<UUIDConsumerFactory> consumer_factory;

  /// UUID producer
  std::shared_ptr<Producers::KafkaJSONCounterProducer> producer;

  /// Time to produce
  std::chrono::seconds period, offset;
};

typedef std::vector<std::pair<std::string, std::string>> kafka_conf_list;

///////////
// Class //
///////////

class JsonConfig : public Config {
private:
  // Configuration for an UUID forwarder
  struct forwarder_config m_counters;
  std::vector<std::string> m_counters_uuid;

  std::vector<std::string> counter_read_topics;
  std::string counter_uuid_key;
  kafka_conf_list counter_consumer_rk_conf_v;
  kafka_conf_list counter_consumer_rkt_conf_v;

  void parse_uuid_consumer_configuration(
      const rapidjson::Value::ConstObject &counters_config,
      struct forwarder_config &fw_config);

public:
  JsonConfig(const std::string &text_config);

  std::vector<std::string> get_counter_read_topics() {
    return this->counter_read_topics;
  }

  std::string get_counter_uuid_key() { return this->counter_uuid_key; }

  kafka_conf_list get_counter_consumer_rk_conf_v() {
    return this->counter_consumer_rk_conf_v;
  }

  kafka_conf_list get_counter_consumer_rkt_conf_v() {
    return this->counter_consumer_rkt_conf_v;
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

  static std::string rapidjson_type_str(const rapidjson::Type type);

  template <typename T, rapidjson::Type expected_json_type, typename T_in>
  static const T
  get_object_child(const T_in &object, const std::string value,
                   const std::function<T(const rapidjson::Value &)> convert_cb,
                   const char *object_name = NULL);

  template <typename T>
  static rapidjson::Value::ConstObject
  get_object_object(const T &object, const std::string value,
                    const char *object_name = NULL);

  template <typename T>
  static rapidjson::Value::ConstArray
  get_object_array(const T &object, const std::string value,
                   const char *object_name = NULL);

  template <typename T>
  static std::string get_object_string(const T &object, const std::string value,
                                       const char *object_name = NULL);

  template <typename T>
  static int64_t get_object_int(const T &object, const std::string value,
                                const char *object_name = NULL);

  static std::vector<std::string>
  get_string_vector(const std::string &array_error_name,
                    const rapidjson::Value::ConstArray &topics);

  static void
  json_parse_kafka_props(const rapidjson::Value::ConstObject &kafka_props,
                         kafka_conf_list &rk_conf, kafka_conf_list &rkt_conf);

  static void parse_kafka_forwarder_properties(
      const rapidjson::Value::ConstObject &forwarder_config,
      kafka_conf_list &consumer_conf, kafka_conf_list &consumer_tconf,
      kafka_conf_list &producer_conf, kafka_conf_list &producer_tconf);
};
};
};
