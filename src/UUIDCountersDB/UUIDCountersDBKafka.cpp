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

#include "UUIDCountersDBKafka.h"

#include <rapidjson/encodings.h>
#include <rapidjson/writer.h>

#include <ctime>

using namespace EventsCounter;
using namespace std;

namespace {
template <typename OutputStream = rapidjson::StringBuffer,
	  typename SourceEncoding = rapidjson::UTF8<>,
	  typename TargetEncoding = rapidjson::UTF8<>,
	  typename StackAllocator = rapidjson::CrtAllocator,
	  unsigned writeFlags = rapidjson::kWriteDefaultFlags>
class CountersJSONWriter : public rapidjson::Writer<OutputStream,
						    SourceEncoding,
						    TargetEncoding,
						    StackAllocator,
						    writeFlags> {
public:
	typedef rapidjson::Writer<OutputStream,
				  SourceEncoding,
				  TargetEncoding,
				  StackAllocator>
			Base;
	typedef typename Base::Ch Ch;

	explicit CountersJSONWriter(
			const string &uuid,
			const uint64_t bytes,
			OutputStream &os,
			StackAllocator *allocator = nullptr,
			size_t levelDepth = Base::kDefaultLevelDepth)
	    : Base(os, allocator, levelDepth) {

		Base::StartObject();

		// JSON object members

		const vector<struct json_child<string>> strings {

			{"type", "data"}, {"unit", "bytes"},
					{"monitor", "uuid_received"},
					{"uuid", uuid},
		};
		const vector<struct json_child<uint64_t>> numbers {
			{"timestamp", current_unix_timestamp()},
					{"value", bytes},
		};

		dump_vars(strings);
		dump_vars(numbers);

		Base::EndObject();
	}

private:
	template <typename T> struct json_child {
		string key;
		T value;
	};

	static uint64_t current_unix_timestamp() {
		return std::time(nullptr);
	}

	bool add_value(const string &str) {
		return Base::String(str.c_str(), str.size());
	}

	bool add_value(const uint64_t val) {
		return Base::Uint64(val);
	}

	/// @todo check add value return type
	template <typename T> void dump_vars(T vars) {
		for (const auto &itr : vars) {
			add_value(itr.key);
			add_value(itr.value);
		}
	}
};

class RedBorderJSONKafkaMessage : public RdKafka::Message {
public:
	RedBorderJSONKafkaMessage(
			std::string t_kafka_key = std::string(),
			int32_t t_partition = RdKafka::Topic::PARTITION_UA,
			void *t_opaque = NULL,
			RdKafka::MessageTimestamp t_timestamp =
					RdKafka::MessageTimestamp())
	    : m_kafka_key(t_kafka_key), m_partition(t_partition),
	      m_opaque(t_opaque), m_timestamp(t_timestamp) {
	}

	RedBorderJSONKafkaMessage(RedBorderJSONKafkaMessage &) = delete;
	RedBorderJSONKafkaMessage(RedBorderJSONKafkaMessage &&) = delete;
	RedBorderJSONKafkaMessage &
	operator=(RedBorderJSONKafkaMessage &) = delete;
	RedBorderJSONKafkaMessage &
	operator=(RedBorderJSONKafkaMessage &&) = delete;

	/* Kafka message interface */
	virtual std::string errstr() const {
		return string();
	}

	virtual RdKafka::ErrorCode err() const {
		return RdKafka::ERR_NO_ERROR;
	}

	virtual RdKafka::Topic *topic() const {
		return NULL;
	}

	virtual std::string topic_name() const {
		return string();
	}

	virtual int32_t partition() const {
		return this->m_partition;
	}

	virtual void *payload() const {
		// Nobody should modify this ever anyway
		return const_cast<char *>(this->string_buffer.GetString());
	}

	virtual size_t len() const {
		return this->string_buffer.GetSize();
	}

	/** @returns Message key as string (if applicable) */
	virtual const std::string *key() const {
		return &this->m_kafka_key;
	}

	/** @returns Message key as void pointer  (if applicable) */
	virtual const void *key_pointer() const {
		return this->m_kafka_key.c_str();
	}

	/** @returns Message key's binary length (if applicable) */
	virtual size_t key_len() const {
		return this->m_kafka_key.size();
	}

	/** @returns Message or error offset (if applicable) */
	virtual int64_t offset() const {
		return 0;
	}

	/** @returns Message timestamp (if applicable) */
	virtual RdKafka::MessageTimestamp timestamp() const {
		return this->m_timestamp;
	}

	/** @returns The \p msg_opaque as provided to
	 * RdKafka::Producer::produce() */
	virtual void *msg_opaque() const {
		return this->m_opaque;
	}

	virtual ~RedBorderJSONKafkaMessage() = default;

	/* rapidjson interface */
	rapidjson::StringBuffer string_buffer;

	// need by Event Counter methods
	void set_kafka_key(string &&str) {
		this->m_kafka_key = str;
	}

private:
	std::string m_kafka_key;
	uint32_t m_partition;
	void *m_opaque;
	RdKafka::MessageTimestamp m_timestamp;
};

class RedborderCounterToKafkaFormatter : public CounterToKafkaFormatter {
public:
	virtual unique_ptr<RdKafka::Message>
	format(const string &uuid, const uint64_t bytes);
};

unique_ptr<RdKafka::Message>
RedborderCounterToKafkaFormatter::format(const string &uuid,
					 const uint64_t bytes) {

	unique_ptr<RedBorderJSONKafkaMessage> ret(
			new RedBorderJSONKafkaMessage(uuid));

	CountersJSONWriter<>(uuid, bytes, ret->string_buffer);

	return ret;
}

}; // anonymous namespace

unique_ptr<CounterToKafkaFormatter> CounterToKafkaFormatter::create() {
	return unique_ptr<CounterToKafkaFormatter>(
			new RedborderCounterToKafkaFormatter());
}
