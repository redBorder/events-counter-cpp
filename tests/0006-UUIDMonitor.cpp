#include "../src/uuid_counters_monitor/kafka_leadership_monitor.hpp"
#include "../src/uuid_counters_monitor/uuid_counters_monitor.hpp"

#include <cstdlib>
#include <memory>
#include <string>

#include <gtest/gtest.h>
#include <librdkafka/rdkafkacpp.h>

namespace {

using namespace std;
using namespace RdKafka;
using namespace EventsCounter::UUIDCountersDB;
using namespace EventsCounter::UUIDCountersMonitor;

class UUIDCountersMonitorTest : public ::testing::Test {
protected:
  static char *rand_tmpl(char *tmpl) {
    int fd = mkstemp(tmpl);
    close(fd);
    remove(tmpl);

    return tmpl;
  }

  static unique_ptr<Conf> create_kafka_consumer_conf(const string &brokers,
                                                     const string &group_id) {
    string errstr;

    unique_ptr<Conf> conf(Conf::create(Conf::CONF_GLOBAL));

    if (conf->set("group.id", group_id, errstr) != Conf::CONF_OK) {
      throw "Error setting group id";
    }

    if (conf->set("metadata.broker.list", brokers, errstr) != Conf::CONF_OK) {
      throw "Error setting broker list";
    }

    return conf;
  }

  static KafkaConsumer *create_kafka_consumer(Conf *conf, const string &topic) {
    string errstr;

    vector<string> topics;
    topics.push_back(topic);

    unique_ptr<Conf> tconf(RdKafka::Conf::create(RdKafka::Conf::CONF_TOPIC));

    conf->set("default_topic_conf", tconf.get(), errstr);

    KafkaConsumer *consumer = KafkaConsumer::create(conf, errstr);
    if (!consumer) {
      throw "Failed to create consumer";
    }

    TopicPartition *partition_zero = TopicPartition::create("test", 0);
    vector<TopicPartition *> new_partitions;
    new_partitions.push_back(partition_zero);
    consumer->assign(new_partitions);

    return consumer;
  }
};

TEST_F(UUIDCountersMonitorTest, check_leadership) {
  UUIDCountersDB::counters_t boostrap_map = {
      {"a", 50}, {"b", 100}, {"c", 150}, {"d", 200}};
  unique_ptr<UUIDCountersDB> db(new UUIDCountersDB(boostrap_map));
  //
  // unique_ptr<Conf> conf(
  // 		create_kafka_consumer_conf("kafka:9092", "tests"));
  // unique_ptr<KafkaConsumer> kafka_consumer(
  // 		create_kafka_consumer(conf.release(), "test"));

  unique_ptr<KafkaLeadershipMonitor> leadership_monitor(
      new KafkaLeadershipMonitor("kafka:9092", "sync", "sync"));

  UUIDCountersMonitor monitor(db.release(), leadership_monitor.release());

  while (true) {
    cout << "Leader: " << monitor.check_leadership() << endl;
    usleep(500000);
  }

  // while (true) {
  // 	unique_ptr<Message> message(kafka_consumer->consume(1000));
  // 	if (message->err() == ERR_NO_ERROR) {
  // 		cout << string(static_cast<const char *>(
  // 					message->payload()))
  // 		     << endl;
  // 	}
  // }

  // kafka_consumer->close();
}
};

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
