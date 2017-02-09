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

#include "config.hpp"
#include "default_kafka_delivery_cb.hpp"
#include "json_config.hpp"

#include <iostream>
#include <rapidjson/document.h>

using namespace std;
using namespace EventsCounter::Configuration;
using namespace rapidjson;
using namespace RdKafka;

// TODO manage reload
JsonConfig::JsonConfig(const std::string &text_config) {
  Document d;
  d.Parse(text_config.c_str());

  if (d.HasParseError()) {
    // FIXME make this exception work
    // throw JSONParserException(GetParseError_En(d.GetParseError()));
  }

  if (!d.IsObject()) {
    throw JSONUnexpectedTypeException("root is not an object");
  }

  {
    // Allowed uuids
    const Value::ConstArray json_uuids = get_object_array(d, "uuids");
    this->m_counters_uuid = get_string_vector("uuids", json_uuids);
  }

  // Parse UUID Counter config
  const Value::ConstObject counters_config =
      get_object_object(d.GetObject(), "counters_config");
  parse_uuid_counter_timer(counters_config);
  this->uuid_counter_config.uuid_key = parse_uuid_key(counters_config);
  this->uuid_counter_config.read_topics =
      parse_read_topics(counters_config, "counters_config");
  this->uuid_counter_config.write_topic =
      parse_write_topic(counters_config, "counters_config");
  parse_rdkafka_configuration(counters_config,
                              this->uuid_counter_config.kafka_config);

  // // Parse Monitor config
  // const Value::ConstObject monitor_config =
  //     get_object_object(d.GetObject(), "monitor_config");
  // parse_rdkafka_configuration(monitor_config, this->monitor_kafka_config);
  // parse_uuid_consumer_configuration(counters_config, this->m_counters);
}

void JsonConfig::parse_rdkafka_configuration(
    const Value::ConstObject &json_config,
    struct kafka_config_s &kafka_config) {

  const Value::ConstObject &consumer_rdkafka_config =
      get_object_object(json_config, "rdkafka");

  parse_kafka_properties(
      consumer_rdkafka_config, kafka_config.consumer_rk_conf_v,
      kafka_config.consumer_rkt_conf_v, kafka_config.producer_rk_conf_v,
      kafka_config.producer_rkt_conf_v);
}

vector<string>
JsonConfig::parse_read_topics(const Value::ConstObject &json_config,
                              const string &object_name) {

  const Value::ConstArray json_read_topics =
      get_object_array(json_config, "read_topics", object_name.c_str());

  return get_string_vector("read_topics", json_read_topics);
}

string JsonConfig::parse_write_topic(const Value::ConstObject &json_config,
                                     const string &object_name) {
  return get_object_string(json_config, "write_topic", object_name.c_str());
}

string JsonConfig::parse_uuid_key(const Value::ConstObject &json_config) {
  return get_object_string(json_config, "json_read_uuid_key");
}

void JsonConfig::parse_uuid_counter_timer(
    const Value::ConstObject &json_config) {
  const Value::ConstObject timer =
      get_object_object(json_config, "timer_seconds", "counters_config");
  this->uuid_counter_config.period =
      chrono::seconds(get_object_int(timer, "period", "timer_seconds"));
  this->uuid_counter_config.offset =
      chrono::seconds(get_object_int(timer, "offset", "timer_seconds"));
}

///////////////////////////
// Helper static methods //
///////////////////////////

string JsonConfig::rapidjson_type_str(const rapidjson::Type type) {
  switch (type) {
  case kNullType:
    return "null";
  case kFalseType:
  case kTrueType:
    return "boolean";
  case kObjectType:
    return "object";
  case kArrayType:
    return "array";
  case kStringType:
    return "string";
  case kNumberType:
    return "number";
  default:
    return "Unknown";
  };
}

/// Get value or throws exception
/// TODO convert_cb should be a template parameter, or accept a method of
/// T_object
template <typename T, rapidjson::Type expected_json_type, typename T_in>
const T
JsonConfig::get_object_child(const T_in &object, const string value,
                             const std::function<T(const Value &)> convert_cb,
                             const char *object_name) {
  const Value::ConstMemberIterator ret_itr = object.FindMember(value.c_str());
  if (ret_itr == object.MemberEnd()) {
    throw JSONChildNotFoundException(
        string("object") + (object_name ? (string(" ") + object_name) : "") +
        " has no property " + value);
  }

  const Value &ret = ret_itr->value;
  const rapidjson::Type value_type = ret.GetType();
  if (expected_json_type != value_type) {
    throw JSONUnexpectedTypeException((object_name ? (string("object"
                                                             " ") +
                                                      object_name)
                                                   : string("")) +
                                      string("child ") + value + " is not " +
                                      rapidjson_type_str(expected_json_type) +
                                      " but " + rapidjson_type_str(value_type));
  }

  return convert_cb(ret);
}

// Need this template for rapidjson::Value and rapidjson::Value::ConstIterator
template <typename T>
Value::ConstObject JsonConfig::get_object_object(const T &object,
                                                 const string value,
                                                 const char *object_name) {
  return get_object_child<Value::ConstObject, kObjectType, T>(
      object, value, [](const Value &v) { return v.GetObject(); }, object_name);
}

template <typename T>
Value::ConstArray JsonConfig::get_object_array(const T &object,
                                               const string value,
                                               const char *object_name) {
  return get_object_child<Value::ConstArray, kArrayType, T>(
      object, value, [](const Value &v) { return v.GetArray(); }, object_name);
}

template <typename T>
std::string JsonConfig::get_object_string(const T &object, const string value,
                                          const char *object_name) {
  return get_object_child<const char *, kStringType, T>(
      object, value, [](const Value &v) { return v.GetString(); }, object_name);
}

template <typename T>
int64_t JsonConfig::get_object_int(const T &object, const string value,
                                   const char *object_name) {
  return get_object_child<int64_t, kNumberType, T>(
      object, value, [](const Value &v) { return v.GetInt64(); }, object_name);
}

vector<string> JsonConfig::get_string_vector(const string &array_error_name,
                                             const Value::ConstArray &topics) {
  vector<string> ret;
  for (const auto &itr : topics) {
    if (!itr.IsString()) {
      cerr << "One element of " << array_error_name << " is not a string";
    }

    ret.push_back(itr.GetString());
  }

  return ret;
}

/// Parse a kafka forwarder module properties
void JsonConfig::parse_kafka_properties(const Value::ConstObject &kafka_config,
                                        kafka_conf_list &consumer_conf,
                                        kafka_conf_list &consumer_tconf,
                                        kafka_conf_list &producer_conf,
                                        kafka_conf_list &producer_tconf) {

  struct to_parse {
    const char *key;
    kafka_conf_list &rk_conf;
    kafka_conf_list &rkt_conf;
  };

  static const std::vector<struct to_parse> to_parse{
      {"read", consumer_conf, consumer_tconf},
      {"write", producer_conf, producer_tconf},
  };

  for (const auto &parse : to_parse) {
    try {
      const Value::ConstObject read =
          JsonConfig::get_object_object(kafka_config, parse.key);
      json_parse_kafka_props(read, parse.rk_conf, parse.rkt_conf);
    } catch (const JSONChildNotFoundException &e) {
      // Do nothing
    }
  }
}

void JsonConfig::json_parse_kafka_props(const Value::ConstObject &kafka_props,
                                        kafka_conf_list &rk_conf,
                                        kafka_conf_list &rkt_conf) {
  /// TODO if(!object)
  for (const auto &property : kafka_props) {
    if (!property.value.IsString()) {
      cerr << "One kafka property is not a "
              "string"
           << endl;
      continue;
    }

    std::string errstr;
    static const char *rkt_prop = "topic.";
    const std::string key = property.name.GetString();
    /// TODO if !string
    const std::string value = property.value.GetString();
    /// TODO if !string

    const bool is_rkt_prop = 0 == key.compare(0, strlen(rkt_prop), rkt_prop);

    auto &conf = is_rkt_prop ? rkt_conf : rk_conf;
    conf.push_back(pair<string, string>(key, value));
  }
};
