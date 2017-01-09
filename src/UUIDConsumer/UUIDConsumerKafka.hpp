#include <stdint.h>

#include <librdkafka/rdkafkacpp.h>
#include <rapidjson/document.h>

class UUIDConsumerKafka {
private:
  RdKafka::KafkaConsumer *kafka_consumer;

public:
  UUIDConsumerKafka(RdKafka::KafkaConsumer *_kafka_consumer)
      : kafka_consumer(_kafka_consumer) {}
  ~UUIDConsumerKafka();
  std::string consume();
};
