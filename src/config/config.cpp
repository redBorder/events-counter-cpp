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

#include "../monitor_producer/kafka_monitor_producer.hpp"
#include "../uuid_consumer/kafka_uuid_consumer.hpp"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <iostream>

using namespace EventsCounter;
using namespace EventsCounter::MonitorProducer;
using namespace EventsCounter::Configuration;
using namespace rapidjson;
using namespace std;

namespace {

static void
rdkafka_set_conf_vector(const vector<pair<string, string>> &conf_parameters,
                        RdKafka::Conf *conf, const string &err_conf_type) {
  string errstr;
  for (const auto &itr : conf_parameters) {
    const RdKafka::Conf::ConfResult rc =
        conf->set(itr.first, itr.second, errstr);
    switch (rc) {
    case RdKafka::Conf::CONF_UNKNOWN:
      cerr << "Unknown " << err_conf_type << " property " << itr.first << ": "
           << errstr << endl;
      continue;
    case RdKafka::Conf::CONF_INVALID:
      cerr << "Unknown " << err_conf_type << " property value " << itr.second
           << " for key " << itr.first << ": " << errstr << endl;
      continue;
    case RdKafka::Conf::CONF_OK:
    default:
      break;
    };
  }
}

class KafkaUUIDConsumerFactory : public JsonConfig::UUIDConsumerFactory {
public:
  typedef vector<pair<string, string>> kafka_conf_list;
  KafkaUUIDConsumerFactory(vector<string> t_read_topics, string t_json_uuid_key,
                           kafka_conf_list t_kafka_consumer_conf,
                           kafka_conf_list t_kafka_consumer_tconf)
      : m_read_topics(t_read_topics), m_json_uuid_key(t_json_uuid_key),
        m_kafka_consumer_conf(t_kafka_consumer_conf),
        m_kafka_consumer_tconf(t_kafka_consumer_tconf) {}

  KafkaUUIDConsumerFactory(KafkaUUIDConsumerFactory &&) = delete;
  KafkaUUIDConsumerFactory &operator=(KafkaUUIDConsumerFactory &) = delete;
  KafkaUUIDConsumerFactory &operator=(KafkaUUIDConsumerFactory &&) = delete;
  ~KafkaUUIDConsumerFactory() {}

  std::unique_ptr<UUIDConsumer::UUIDConsumer> create() {
    string errstr;
    unique_ptr<RdKafka::Conf> conf(
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)),
        tconf(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

    rdkafka_set_conf_vector(this->m_kafka_consumer_tconf, tconf.get(), "topic");
    rdkafka_set_conf_vector(this->m_kafka_consumer_conf, conf.get(), "kafka");
    conf->set("default_topic_conf", tconf.get(), errstr);

    return std::unique_ptr<UUIDConsumer::UUIDConsumer>(
        new UUIDConsumer::CounterUUIDJSONKafkaConsumer(
            this->m_read_topics, m_json_uuid_key, conf.get()));
  }

private:
  vector<string> m_read_topics;
  string m_json_uuid_key;
  kafka_conf_list m_kafka_consumer_conf;
  kafka_conf_list m_kafka_consumer_tconf;
};

static string rapidjson_type_str(const rapidjson::Type type) {
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

class JSONUnexpectedTypeException : public JsonConfig::JSONParserException {
public:
  /// Inherit constructor
  using JSONParserException::JSONParserException;
};

class JSONChildNotFoundException : public JsonConfig::JSONParserException {
public:
  /// Inherit constructor
  using JSONParserException::JSONParserException;
};

namespace JSON {

/// Get value or throws exception
/// @TODO convert_cb should be a template parameter, or accept a method of
/// T_object
template <typename T, rapidjson::Type expected_json_type, typename T_in>
static const T
get_object_child(const T_in &object, const string value,
                 const std::function<T(const Value &)> convert_cb,
                 const char *object_name = NULL) {
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
static Value::ConstObject get_object_object(const T &object, const string value,
                                            const char *object_name = NULL) {
  return get_object_child<Value::ConstObject, kObjectType, T>(
      object, value, [](const Value &v) { return v.GetObject(); }, object_name);
}

template <typename T>
static Value::ConstArray get_object_array(const T &object, const string value,
                                          const char *object_name = NULL) {
  return get_object_child<Value::ConstArray, kArrayType, T>(
      object, value, [](const Value &v) { return v.GetArray(); }, object_name);
}

template <typename T>
static std::string get_object_string(const T &object, const string value,
                                     const char *object_name = NULL) {
  return get_object_child<const char *, kStringType, T>(
      object, value, [](const Value &v) { return v.GetString(); }, object_name);
}

template <typename T>
static int64_t get_object_int(const T &object, const string value,
                              const char *object_name = NULL) {
  return get_object_child<int64_t, kNumberType, T>(
      object, value, [](const Value &v) { return v.GetInt64(); }, object_name);
}

static vector<string> get_string_vector(const string &array_error_name,
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

}; // JSON namespace

static void
json_parse_kafka_props(const Value::ConstObject &kafka_props,
                       KafkaUUIDConsumerFactory::kafka_conf_list &rk_conf,
                       KafkaUUIDConsumerFactory::kafka_conf_list &rkt_conf) {

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

/// Parse a kafka forwarder module properties
static void parse_kafka_forwarder_properties(
    const Value::ConstObject &forwarder_config,
    KafkaUUIDConsumerFactory::kafka_conf_list &consumer_conf,
    KafkaUUIDConsumerFactory::kafka_conf_list &consumer_tconf,
    KafkaUUIDConsumerFactory::kafka_conf_list &producer_conf,
    KafkaUUIDConsumerFactory::kafka_conf_list &producer_tconf) {
  struct to_parse {
    const char *key;
    KafkaUUIDConsumerFactory::kafka_conf_list &rk_conf, &rkt_conf;
  };

  static const std::vector<struct to_parse> to_parse{
      {"read", consumer_conf, consumer_tconf},
      {"write", producer_conf, producer_tconf},
  };

  for (const auto &parse : to_parse) {
    try {
      const Value::ConstObject read =
          JSON::get_object_object(forwarder_config, parse.key);
      json_parse_kafka_props(read, parse.rk_conf, parse.rkt_conf);
    } catch (const JSONChildNotFoundException &e) {
      // Do nothing
    }
  }
}

class EventCounterCb : public RdKafka::DeliveryReportCb {
public:
  virtual void dr_cb(RdKafka::Message &message) {
    if (message.err() != RdKafka::ERR_NO_ERROR) {
      cerr << "Couldn't deliver [";
      cerr.write(static_cast<char *>(message.payload()), message.len())
          << "] to topic [" << message.topic_name() << ':'
          << message.partition() << "]. Error: " << message.errstr() << endl;
    }
  }
};

static EventCounterCb events_counter_cb;

} // anonymous namespace

/// @todo manage reload
JsonConfig *JsonConfig::json_parse(const std::string &text_config) {
  unique_ptr<JsonConfig> ret(new JsonConfig());

  Document d;
  d.Parse(text_config.c_str());

  if (d.HasParseError()) {
    throw JSONParserException(GetParseError_En(d.GetParseError()));
  }

  if (!d.IsObject()) {
    throw JSONUnexpectedTypeException("root is not an object");
  }

  {
    // Allowed uuids
    const Value::ConstArray json_uuids = JSON::get_object_array(d, "uuids");
    ret->m_counters_uuid = JSON::get_string_vector("uuids", json_uuids);
  }

  const Value::ConstObject counters_config =
      JSON::get_object_object(d.GetObject(), "counters_config");

  parse_counter_consumer_configuration(counters_config, ret->m_counters);

  return ret.release();
}

void JsonConfig::parse_counter_consumer_configuration(
    const Value::ConstObject &json_config,
    struct JsonConfig::forwarder_config &fw_config) {
  KafkaUUIDConsumerFactory::kafka_conf_list counter_consumer_rk_conf_v,
      counter_consumer_rkt_conf_v, counter_producer_rk_conf_v,
      counter_producer_rkt_conf_v;

  {
    // Parse consumer & producer properties
    const Value::ConstObject &counter_rdkafka_config =
        JSON::get_object_object(json_config, "rdkafka");

    parse_kafka_forwarder_properties(
        counter_rdkafka_config, counter_consumer_rk_conf_v,
        counter_consumer_rkt_conf_v, counter_producer_rk_conf_v,
        counter_producer_rkt_conf_v);
  }

  {
    // Consumer
    const Value::ConstArray json_read_topics =
        JSON::get_object_array(json_config, "read_topics", "counters_config");

    const string counter_uuid_key =
        JSON::get_object_string(json_config, "json_read_uuid_key");

    vector<string> counter_read_topics =
        JSON::get_string_vector("read_topics", json_read_topics);

    fw_config.consumer_factory = unique_ptr<JsonConfig::UUIDConsumerFactory>(
        new KafkaUUIDConsumerFactory(counter_read_topics, counter_uuid_key,
                                     counter_consumer_rk_conf_v,
                                     counter_consumer_rkt_conf_v));
  }

  {
    // Timer for produce messages
    const Value::ConstObject timer = JSON::get_object_object(
        json_config, "timer_seconds", "counters_config");
    fw_config.period =
        chrono::seconds(JSON::get_object_int(timer, "period", "timer_seconds"));

    fw_config.offset =
        chrono::seconds(JSON::get_object_int(timer, "offset", "timer_seconds"));
  }

  {
    string errstr;
    const string counter_write_topic =
        JSON::get_object_string(json_config, "write_topic", "counters_config");

    /// Producer config
    unique_ptr<RdKafka::Conf> conf(
        RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL)),
        tconf(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

    rdkafka_set_conf_vector(counter_producer_rk_conf_v, conf.get(), "kafka");
    rdkafka_set_conf_vector(counter_producer_rkt_conf_v, tconf.get(), "topic");

    conf->set("dr_cb", &events_counter_cb, errstr);

    std::unique_ptr<RdKafka::Producer> rk(
        RdKafka::Producer::create(conf.get(), errstr));
    std::unique_ptr<RdKafka::Topic> rkt(RdKafka::Topic::create(
        rk.get(), counter_write_topic, tconf.get(), errstr));
    fw_config.producer =
        make_shared<KafkaMonitorProducer>(rk.release(), rkt.release());
  }
}
