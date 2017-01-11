#pragma once

#include "UUIDBytes.h"

#include <librdkafka/rdkafkacpp.h>
#include <rapidjson/document.h>

namespace EventsCounter {

class UUIDConsumerKafka {
private:
	RdKafka::KafkaConsumer *kafka_consumer;

public:
	/**
	 *
	 */
	UUIDConsumerKafka(const char *brokers,
			  const char *group_id,
			  const char *topic);

	/**
	 *
	 */
	UUIDConsumerKafka(RdKafka::Conf *t_kafka_consumer_conf,
			  RdKafka::Conf *t_kafka_topic_conf);

	/**
	 *
	 */
	UUIDConsumerKafka(RdKafka::KafkaConsumer *t_kafka_consumer)
	    : kafka_consumer(t_kafka_consumer) {
	}

	/**
	 *
	 */
	~UUIDConsumerKafka();

	/**
	 *
	 */
	UUIDConsumerKafka &operator=(const UUIDConsumerKafka &) = delete;

	/**
	 * [consume description]
	 * @param  timeout [description]
	 * @return         [description]
	 */
	UUIDBytes consume(uint32_t timeout) const;
};
};
