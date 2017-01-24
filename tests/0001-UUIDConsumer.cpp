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

#include "../src/UUIDConsumer/UUIDConsumerKafka.h"
#include "../src/UUIDCountersDB/UUIDCountersDB.h"

#include <gtest/gtest.h>

// #include <cstdlib>
// #include <memory>
// #include <string>

namespace {

using namespace std;
using namespace EventsCounter;
using namespace EventsCounter::TestUtils;

class UUIDConsumerTest : public ::testing::Test {};

TEST_F(UUIDConsumerTest, consumer_uuid) {
	EXPECT_NO_THROW({
		static const string json_uuid_key = "sensor_uuid";
		const string topic_str = random_topic();
		const string group_id = string("group_") + topic_str;

		unique_ptr<RdKafka::Conf> kafka_conf =
				create_test_kafka_consumer_config("kafka:9092",
								  group_id);
		UUIDConsumerKafka consumer(vector<string>{topic_str},
					   json_uuid_key,
					   kafka_conf.get());

		UUIDProduce(json_uuid_key, "123456", topic_str);
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
