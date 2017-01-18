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

#include "Config.h"

#include "UUIDConsumer/UUIDConsumerKafka.h"

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>

#include <iostream>

using namespace EventsCounter;
using namespace EventsCounter::Configuration;
using namespace rapidjson;
using namespace std;

namespace {

static void
rdkafka_set_conf_vector(const vector<pair<string, string>> &conf_parameters,
			RdKafka::Conf *conf,
			const string &err_conf_type) {
	string errstr;
	for (const auto &itr : conf_parameters) {
		const RdKafka::Conf::ConfResult rc =
				conf->set(itr.first, itr.second, errstr);
		switch (rc) {
		case RdKafka::Conf::CONF_UNKNOWN:
			cerr << "Unknown " << err_conf_type << " property "
			     << itr.first << ": " << errstr << endl;
			continue;
		case RdKafka::Conf::CONF_INVALID:
			cerr << "Unknown " << err_conf_type
			     << " property value " << itr.second << " for key "
			     << itr.first << ": " << errstr << endl;
			continue;
		case RdKafka::Conf::CONF_OK:
		default:
			break;
		};
	}
}

class KafkaUUIDConsumerFactory : public Config::UUIDConsumerFactory {
public:
	typedef vector<pair<string, string>> kafka_conf_list;
	KafkaUUIDConsumerFactory(vector<string> t_read_topics,
				 kafka_conf_list t_kafka_consumer_conf,
				 kafka_conf_list t_kafka_consumer_tconf)
	    : m_read_topics(t_read_topics),
	      m_kafka_consumer_conf(t_kafka_consumer_conf),
	      m_kafka_consumer_tconf(t_kafka_consumer_tconf) {
	}

	KafkaUUIDConsumerFactory(KafkaUUIDConsumerFactory &&) = delete;
	KafkaUUIDConsumerFactory &
	operator=(KafkaUUIDConsumerFactory &) = delete;
	KafkaUUIDConsumerFactory &
	operator=(KafkaUUIDConsumerFactory &&) = delete;
	~KafkaUUIDConsumerFactory() {
	}

	UUIDConsumer *create() {
		string errstr;
		unique_ptr<RdKafka::Conf> conf(RdKafka::Conf::create(
				RdKafka::Conf::CONF_GLOBAL)),
				tconf(RdKafka::Conf::create(
						RdKafka::Conf::CONF_TOPIC));

		rdkafka_set_conf_vector(this->m_kafka_consumer_tconf,
					tconf.get(),
					"topic");
		rdkafka_set_conf_vector(this->m_kafka_consumer_conf,
					conf.get(),
					"kafka");
		conf->set("default_topic_conf", tconf.get(), errstr);

		return new UUIDConsumerKafka(this->m_read_topics, conf.get());
	}

private:
	vector<string> m_read_topics;
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

/// Get value or throws exception
/// @TODO convert_cb should be a template parameter, or accept a method of
/// T_object
template <typename T, rapidjson::Type expected_json_type, typename T_object>
static const T get_config(const T_object &object,
			  const string value,
			  const std::function<T(const Value &)> convert_cb,
			  const string *object_name = NULL) {
	const Value::ConstMemberIterator ret_itr =
			object.FindMember(value.c_str());
	if (ret_itr == object.MemberEnd()) {
		throw JSONChildNotFoundException(
				string("object") +
				(object_name ? (string(" ") +
						object_name->c_str())
					     : "") +
				" has no property " + value);
	}

	const Value &ret = ret_itr->value;
	const rapidjson::Type value_type = ret.GetType();
	if (expected_json_type != value_type) {
		throw JSONUnexpectedTypeException(
				(object_name ? (string("object"
						       " ") +
						object_name->c_str())
					     : string("")) +
				string("child ") + value + " is not " +
				rapidjson_type_str(expected_json_type) +
				" but " + rapidjson_type_str(value_type));
	}

	return convert_cb(ret);
}

static vector<string> json_get_string_vector(const string &array_error_name,
					     const Value::ConstArray &topics) {
	vector<string> ret(topics.Size());
	for (const auto &itr : topics) {
		if (!itr.IsString()) {
			cerr << "One element of " << array_error_name
			     << " is not a string";
		}
		ret.push_back(itr.GetString());
	}

	return ret;
}

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

		const bool is_rkt_prop =
				0 == key.compare(0, strlen(rkt_prop), rkt_prop);

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
			const Value::ConstObject read = get_config<
					Value::ConstObject,
					kObjectType>(
					forwarder_config,
					parse.key,
					[](const Value &v) {
						return v.GetObject();
					});
			json_parse_kafka_props(
					read, parse.rk_conf, parse.rkt_conf);
		} catch (const JSONChildNotFoundException &e) {
			// Do nothing
		}
	}
}

} // unnamed namespace

/// @todo manage reload
JsonConfig *JsonConfig::json_parse(const std::string &text_config) {
	unique_ptr<JsonConfig> ret(new JsonConfig());

	Document d;
	d.Parse(text_config.c_str());
	KafkaUUIDConsumerFactory::kafka_conf_list counter_consumer_rk_conf_v,
			counter_consumer_rkt_conf_v, counter_producer_rk_conf_v,
			counter_producer_rkt_conf_v;

	if (d.HasParseError()) {
		throw JSONParserException(GetParseError_En(d.GetParseError()));
	}

	if (!d.IsObject()) {
		throw JSONUnexpectedTypeException("root is not an object");
	}

	const Value::ConstObject counters_config =
			get_config<Value::ConstObject, kObjectType>(
					d.GetObject(),
					"counters_config",
					[](const Value &v) {
						return v.GetObject();
					});

	const Value::ConstArray json_read_topics =
			get_config<Value::ConstArray, kArrayType>(
					counters_config,
					"read_topics",
					[](const Value &v) {
						return v.GetArray();
					});

	vector<string> counter_read_topics =
			json_get_string_vector("read_topics", json_read_topics);

	const Value::ConstObject &counter_rdkafka_config =
			get_config<Value::ConstObject, kObjectType>(
					counters_config,
					"rdkafka",
					[](const Value &v) {
						return v.GetObject();
					});

	// TODO if !exist counters_config
	parse_kafka_forwarder_properties(counter_rdkafka_config,
					 counter_consumer_rk_conf_v,
					 counter_consumer_rkt_conf_v,
					 counter_producer_rk_conf_v,
					 counter_producer_rkt_conf_v);

	ret->m_counters_uuid_consumer_factory = unique_ptr<
			KafkaUUIDConsumerFactory>(new KafkaUUIDConsumerFactory(
			counter_read_topics,
			counter_consumer_rk_conf_v,
			counter_consumer_rkt_conf_v));

	return ret.release();
}
