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

#include "TestUtils.h"

#include "Config/Config.h"
#include "UUIDConsumer/UUIDConsumerKafka.h"
#include "UUIDCounter/UUIDCounter.h"

#include <gtest/gtest.h>
#include <librdkafka/rdkafkacpp.h>

#include <chrono>
#include <cstdlib>
#include <memory>
#include <string>

namespace {

using namespace std;
using namespace EventsCounter;
using namespace EventsCounter::TestUtils;
using namespace EventsCounter::Configuration;

class UUIDConsumerTest : public ::testing::Test {
protected:
	static char *rand_tmpl(char *tmpl) {
		int fd = mkstemp(tmpl);
		close(fd);
		remove(tmpl);

		return tmpl;
	}

	static string test_config(const vector<string> read_topics,
				  const string read_group_id) {
		stringstream read_topics_s;
		for (auto i = read_topics.cbegin(); i != read_topics.cend();
		     i++) {
			if (i != read_topics.cbegin()) {
				read_topics_s << ',';
			}
			read_topics_s << "\"" << *i << "\"";
		}

		stringstream ss;
		// clang-format off
		ss << '{' <<
                    "\"counters_config\":{" <<
                        "\"read_topics\":[" <<
                            read_topics_s.rdbuf() <<
                        "]," <<
                        "\"rdkafka\": {" <<
                            "\"read\":{" <<
                                "\"group.id\":\"" << read_group_id << "\"," <<
                                "\"metadata.broker.list\":\"kafka:9092\","
                                "\"topic.auto.offset.reset\":\"smallest\""
                            "}" <<
                        "}" <<
                    "}" <<
               '}';
		// clang-format on

		return ss.str();
	}

public:
	static void counter_test() {
		static const map<string, uint64_t> zero_uuid_counters{
				{"123456", 0}};
		UUIDCountersDB db0(zero_uuid_counters);

		const vector<string> read_topics{random_topic()};
		const string group_id = string("group_") + read_topics[0];

		unique_ptr<RdKafka::Conf> conf =
				create_test_kafka_consumer_config("kafka:9092",
								  group_id);
		unique_ptr<Config> config(JsonConfig::json_parse(
				test_config(read_topics, group_id)));
		unique_ptr<UUIDConsumer> uuid_consumer = config->get_consumer();
		UUIDCounter counter(uuid_consumer.release(), db0);

		EventsCounter::UUIDCountersDB::counters_t aux_counters =
				zero_uuid_counters;
		UUIDProduce("123455", read_topics[0]); // Invalid UUID, should
						       // ignore
		UUIDProduce("123456", read_topics[0]);
		while (true) {
			counter.swap_counters(aux_counters);
			if (aux_counters["123456"] != 0) {
				break;
			}

			sleep(1); // @TODO C++ version?
		}

		// Check that any map contains invalid uuid
		ASSERT_EQ(aux_counters.find("123455"), aux_counters.end());
		counter.swap_counters(aux_counters);
		ASSERT_EQ(aux_counters.find("123455"), aux_counters.end());
		counter.swap_counters(aux_counters);

		// And bytes have been incremented for valid one
		ASSERT_NE(aux_counters["123456"], 0);
	}
};

TEST_F(UUIDConsumerTest, consumer_uuid) {
	EXPECT_NO_THROW(counter_test());
}

} // anonymous namespace

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
