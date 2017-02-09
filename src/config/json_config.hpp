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

///////////
// Class //
///////////

class JsonConfig : public Config {

public:
  JsonConfig(const std::string &text_config);

  ///////////////////////////
  // Config implementation //
  ///////////////////////////

  struct uuid_counter_config_s get_uuid_counter_config() {
    return this->uuid_counter_config;
  }

  struct counters_monitor_config_s get_monitor_config() {
    return this->counters_monitor_config;
  }

  virtual const std::vector<std::string> &counters_uuids() {
    return m_counters_uuid;
  };

private:
  std::vector<std::string> m_counters_uuid;
  struct uuid_counter_config_s uuid_counter_config;
  struct counters_monitor_config_s counters_monitor_config;

  /////////////
  // Helpers //
  /////////////

  void parse_uuid_consumer_configuration(
      const rapidjson::Value::ConstObject &json_config,
      struct kafka_config_s &kafka_config);

  void
  parse_rdkafka_configuration(const rapidjson::Value::ConstObject &json_config,
                              struct kafka_config_s &kafka_config);

  std::vector<std::string>
  parse_read_topics(const rapidjson::Value::ConstObject &json_config,
                    const std::string &object_name);

  std::string parse_uuid_key(const rapidjson::Value::ConstObject &json_config);

  void
  parse_uuid_counter_timer(const rapidjson::Value::ConstObject &json_config);

  std::string
  parse_write_topic(const rapidjson::Value::ConstObject &json_config,
                    const std::string &object_name);

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

  static void parse_kafka_properties(
      const rapidjson::Value::ConstObject &kafka_config,
      kafka_conf_list &consumer_conf, kafka_conf_list &consumer_tconf,
      kafka_conf_list &producer_conf, kafka_conf_list &producer_tconf);
};
};
};
