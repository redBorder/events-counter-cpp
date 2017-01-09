#include "../src/UUIDConsumer/UUIDConsumerKafka.h"
#include "../src/UUIDCountersDB/UUIDCountersDB.h"

#include <gtest/gtest.h>

#include <string>

namespace {

using namespace std;
using namespace EventsCounter;

static char *rand_tmpl() {
	char *tmpl = strdup("rb_flow_XXXXXX");
	int fd = mkstemp(tmpl);
	close(fd);
	remove(tmpl);

	return tmpl;
}

class UUIDConsumerTest : public ::testing::Test {
protected:
	void UUIDProduce(const char *uuid, const char *topic_str) {
		char *message = (char *)calloc(128, sizeof(char));
		sprintf(message, "{\"sensor_uuid\": \"%s\"}", uuid);

		string errstr;
		RdKafka::Conf *conf = RdKafka::Conf::create(
				RdKafka::Conf::CONF_GLOBAL);
		RdKafka::Conf *tconf = RdKafka::Conf::create(
				RdKafka::Conf::CONF_TOPIC);

		conf->set("metadata.broker.list", "localhost:9092", errstr);

		RdKafka::Producer *producer =
				RdKafka::Producer::create(conf, errstr);
		if (!producer) {
			std::cerr << "Failed to create producer: " << errstr
				  << std::endl;
		}

		RdKafka::Topic *topic = RdKafka::Topic::create(
				producer, topic_str, tconf, errstr);
		if (!topic) {
			std::cerr << "Failed to create topic: " << errstr
				  << std::endl;
		}

		producer->produce(topic,
				  0,
				  RdKafka::Producer::RK_MSG_COPY,
				  message,
				  strlen(message),
				  NULL,
				  NULL);
		free(message);
	}
};

TEST_F(UUIDConsumerTest, consumer_uuid) {
	EXPECT_NO_THROW({
		UUIDBytes data;
		char *topic_str = rand_tmpl();
		char *group_id = rand_tmpl();

		unique_ptr<UUIDConsumerKafka> consumer(new UUIDConsumerKafka(
				"localhost:9092", group_id, topic_str));

		UUIDProduce("123456", topic_str);
		while (true) {
			data = consumer->consume(100);
			if (!data.empty()) {
				break;
			}
		}

		free(group_id);
		free(topic_str);
		EXPECT_EQ("123456", data.get_uuid());
		EXPECT_EQ(25, data.get_bytes());
	});
}
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
