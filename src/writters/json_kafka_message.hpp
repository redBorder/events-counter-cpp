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

#include <librdkafka/rdkafkacpp.h>
#include <rapidjson/stringbuffer.h>

namespace EventsCounter {
namespace Formatters {

template <typename T> struct json_child {
  std::string key;
  T value;
};

class JSONKafkaMessage : public RdKafka::Message {
private:
  std::string m_kafka_key;
  uint32_t m_partition;
  void *m_opaque;
  RdKafka::MessageTimestamp m_timestamp;

public:
  JSONKafkaMessage(
      std::string t_kafka_key = std::string(),
      int32_t t_partition = RdKafka::Topic::PARTITION_UA, void *t_opaque = NULL,
      RdKafka::MessageTimestamp t_timestamp = RdKafka::MessageTimestamp());
  JSONKafkaMessage(JSONKafkaMessage &) = delete;
  JSONKafkaMessage(JSONKafkaMessage &&) = delete;
  JSONKafkaMessage &operator=(JSONKafkaMessage &) = delete;
  JSONKafkaMessage &operator=(JSONKafkaMessage &&) = delete;
  virtual ~JSONKafkaMessage() = default;

  /**
   * Needed by Event Counter methods
   */
  void set_kafka_key(std::string &&str) { this->m_kafka_key = str; }

  /////////////////////////
  // RapidJson Interface //
  /////////////////////////

  rapidjson::StringBuffer string_buffer;

  ////////////////////////////////
  // RdKafka::Message Interface //
  ////////////////////////////////

  virtual std::string errstr() const { return std::string(); }
  virtual RdKafka::ErrorCode err() const { return RdKafka::ERR_NO_ERROR; }
  virtual RdKafka::Topic *topic() const { return NULL; }
  virtual std::string topic_name() const { return std::string(); }
  virtual int32_t partition() const { return this->m_partition; }
  // NOTE Nobody should modify this ever anyway
  virtual void *payload() const {
    return const_cast<char *>(this->string_buffer.GetString());
  }
  virtual size_t len() const { return this->string_buffer.GetSize(); }
  virtual const std::string *key() const { return &this->m_kafka_key; }
  virtual const void *key_pointer() const { return this->m_kafka_key.c_str(); }
  virtual size_t key_len() const { return this->m_kafka_key.size(); }
  virtual int64_t offset() const { return 0; }
  virtual RdKafka::MessageTimestamp timestamp() const {
    return this->m_timestamp;
  }
  virtual void *msg_opaque() const { return this->m_opaque; }
};
};
};
