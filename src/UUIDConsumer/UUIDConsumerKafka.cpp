#include <iostream>
#include <memory>

#include "UUIDConsumer.h"
#include "UUIDConsumerKafka.h"

using namespace rapidjson;
using namespace std;
using namespace EventsCounter;

UUIDBytes UUIDConsumerKafka::consume(uint32_t timeout) const {
	unique_ptr<RdKafka::Message> message(
			this->kafka_consumer->consume(timeout));

	switch (message->err()) {
	case RdKafka::ERR__TIMED_OUT:
		break;

	case RdKafka::ERR__PARTITION_EOF:
		break;

	case RdKafka::ERR_NO_ERROR: {
		const char *buf = (const char *)message->payload();
		Document json;
		json.Parse(buf);

		if (!json.IsObject() || !json.HasMember("sensor_uuid") ||
		    !json["sensor_uuid"].IsString()) {
			break;
		}

		Value &uuid = json["sensor_uuid"];
		std::string uuid_str = uuid.GetString();

		return UUIDBytes(uuid_str, message->len());
	}

	default:
		break;
	}

	return UUIDBytes();
}

UUIDConsumerKafka::UUIDConsumerKafka(const char *brokers,
				     const char *group_id,
				     const char *topic) {
	std::string errstr;
	std::vector<std::string> topics;

	topics.push_back(topic);

	unique_ptr<RdKafka::Conf> conf(
			RdKafka::Conf::create(RdKafka::Conf::CONF_GLOBAL));
	unique_ptr<RdKafka::Conf> tconf(
			RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

	if (conf->set("group.id", group_id, errstr) != RdKafka::Conf::CONF_OK) {
		throw "Error setting group id";
	}

	if (conf->set("metadata.broker.list", brokers, errstr) !=
	    RdKafka::Conf::CONF_OK) {
		throw "Error setting broker list";
	}

	if (tconf->set("offset.store.method", "broker", errstr) !=
	    RdKafka::Conf::CONF_OK) {
		throw "Error setting group id";
	}

	if (tconf->set("auto.offset.reset", "smallest", errstr) !=
	    RdKafka::Conf::CONF_OK) {
		throw "Error setting offset";
	}

	if (conf->set("default_topic_conf", tconf.get(), errstr) !=
	    RdKafka::Conf::CONF_OK) {
		throw "Error setting topic conf";
	}

	this->kafka_consumer =
			RdKafka::KafkaConsumer::create(conf.get(), errstr);
	if (!this->kafka_consumer) {
		throw "Failed to create consumer";
	}

	RdKafka::ErrorCode err = this->kafka_consumer->subscribe(topics);
	if (err) {
		throw "Failed to subscribe to topic";
	}
}

UUIDConsumerKafka::~UUIDConsumerKafka() {
	this->kafka_consumer->close();
	delete this->kafka_consumer;
}
