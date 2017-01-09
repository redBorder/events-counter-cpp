#include "UUIDConsumer.hpp"
#include "UUIDConsumerKafka.hpp"

using namespace rapidjson;

std::string UUIDConsumerKafka::consume() {

  RdKafka::Message *message = this->kafka_consumer->consume(1000);

  switch (message->err()) {
  case RdKafka::ERR__TIMED_OUT:
    break;

  case RdKafka::ERR__PARTITION_EOF:
    break;

  case RdKafka::ERR_NO_ERROR: {
    const char *buf = (const char *)message->payload();
    Document json;
    json.Parse(buf);

    if (!json.IsObject() || !json.HasMember("uuid") ||
        !json["uuid"].IsString()) {
      break;
    }

    Value &uuid = json["uuid"];
    std::string uuid_str = uuid.GetString();

    return uuid_str;
  }

  default:
    break;
  }

  return NULL;
}
