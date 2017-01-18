#include "../src/UUIDConsumer/UUIDConsumerKafka.h"
#include "../src/UUIDCountersDB/UUIDCountersDB.h"

#include <gtest/gtest.h>

#include <cstdlib>
#include <memory>
#include <string>

namespace {

using namespace std;
using namespace EventsCounter;

class UUIDConsumerTest : public ::testing::Test {
protected:
	static char *rand_tmpl(char *tmpl) {
		int fd = mkstemp(tmpl);
		close(fd);
		remove(tmpl);

		return tmpl;
	}

	static void UUIDProduce(const string &uuid, const string &topic_str) {
		const string message =
				string("{\"sensor_uuid\": \"") + uuid + "\"}";
		string errstr;

		unique_ptr<RdKafka::Conf> conf(RdKafka::Conf::create(
				RdKafka::Conf::CONF_GLOBAL));
		unique_ptr<RdKafka::Conf> tconf(RdKafka::Conf::create(
				RdKafka::Conf::CONF_TOPIC));

		conf->set("metadata.broker.list", "kafka:9092", errstr);

		unique_ptr<RdKafka::Producer> producer(
				RdKafka::Producer::create(conf.get(), errstr));
		ASSERT_TRUE(NULL != producer.get());

		unique_ptr<RdKafka::Topic> topic(
				RdKafka::Topic::create(producer.get(),
						       topic_str,
						       tconf.get(),
						       errstr));
		ASSERT_TRUE(NULL != topic.get());

		producer->produce(topic.get(),
				  0,
				  RdKafka::Producer::RK_MSG_COPY,
				  const_cast<char *>(message.c_str()),
				  message.size(),
				  NULL,
				  NULL);

		while (producer->outq_len() > 0) {
			producer->poll(100);
		}
	}

	static shared_ptr<RdKafka::Conf>
	create_test_kafka_config(const std::string &brokers,
				 const std::string &group_id) {
		string errstr;
		unique_ptr<RdKafka::Conf> topic_conf(RdKafka::Conf::create(
				RdKafka::Conf::CONF_TOPIC));
		shared_ptr<RdKafka::Conf> conf(RdKafka::Conf::create(
				RdKafka::Conf::CONF_GLOBAL));

		struct property {
			RdKafka::Conf *conf;
			std::string key, val;
		};

		const array<struct property, 3> properties{{
				{topic_conf.get(),
				 "auto.offset.reset",
				 "smallest"},
				{conf.get(), "metadata.broker.list", brokers},
				{conf.get(), "group.id", group_id},
		}};

		for (const struct property &property : properties) {
			const RdKafka::Conf::ConfResult rc = property.conf->set(
					property.key, property.val, errstr);

			EXPECT_EQ(rc, RdKafka::Conf::CONF_OK);
		}

		const RdKafka::Conf::ConfResult set_default_topic_conf_rc =
				conf->set("default_topic_conf",
					  topic_conf.get(),
					  errstr);
		EXPECT_EQ(set_default_topic_conf_rc, RdKafka::Conf::CONF_OK);
		return conf;
	}
};

TEST_F(UUIDConsumerTest, consumer_uuid) {
	EXPECT_NO_THROW({
		char tmp_topic_tmpl[] = "rb_flow_XXXXXX";
		rand_tmpl(tmp_topic_tmpl);
		const string topic_str = tmp_topic_tmpl;
		const string group_id = string("group_") + tmp_topic_tmpl;

		shared_ptr<RdKafka::Conf> kafka_conf(create_test_kafka_config(
				"kafka:9092", group_id));
		UUIDConsumerKafka consumer(vector<string>{topic_str},
					   kafka_conf.get());

		UUIDProduce("123456", topic_str);
		while (true) {
			const UUIDBytes data = consumer.consume(100);
			if (data.empty()) {
				continue;
			}

			EXPECT_EQ("123456", data.get_uuid());
			EXPECT_EQ(25, data.get_bytes());
			break;
		}

	});
}
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);

	const int rc = RUN_ALL_TESTS();
	for (int i = 0; i < 10; ++i) {
		static const int timeout_ms = 100;
		const int destroy_rc = RdKafka::wait_destroyed(timeout_ms);
		if (0 == destroy_rc) {
			break;
		}
	}

	return rc;
}
