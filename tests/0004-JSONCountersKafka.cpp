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

#include "UUIDCountersDB/UUIDCountersDBKafka.h"

#include <gtest/gtest.h>
#include <rapidjson/document.h>

using namespace std;
using namespace EventsCounter;

namespace {

class UUIDConsumerTest : public ::testing::Test {
public:
	static void counter_test() {
		static const map<string, uint64_t> counters{
				{"1", 0}, {"2", 10}, {"3", 20}};

		unique_ptr<CounterToKafkaFormatter> formatter(
				CounterToKafkaFormatter::create());

		for (const auto &counter : counters) {
			rapidjson::Document d;
			std::unique_ptr<RdKafka::Message> msg =
					formatter->format(counter.first,
							  counter.second);

			ASSERT_EQ(*msg->key(), counter.first);
			d.Parse(static_cast<char *>(msg->payload()),
				msg->len());
			ASSERT_FALSE(d.HasParseError());

			const vector<struct json_child<string>>
					expected_strings {

				{"type", "data"}, {"unit", "bytes"},
						{"monitor", "uuid_received"},
						{"uuid", counter.first},
			};
			const vector<struct json_child<uint64_t>>
					expected_numbers {
				// @todo check timestamp somehow
				// {"timestamp", current_unix_timestamp()},
				{"value", counter.second},
			};

			/// @todo can't pass callback in template?
			for (const auto &expected_child : expected_strings) {
				assert_json_member(
						d,
						expected_child.key,
						expected_child.value,
						&rapidjson::Value::GetString);
			}

			for (const auto &expected_child : expected_numbers) {
				assert_json_member(d,
						   expected_child.key,
						   expected_child.value,
						   &rapidjson::Document::
								   GetUint64);
			}
		}
	}

private:
	/// Aux struct to help checking
	template <typename T> struct json_child {
		const char *key;
		T value;
	};

	template <typename T, typename F>
	static void assert_json_member(const rapidjson::Value &document,
				       const char *key,
				       const T &expected,
				       F value_cb) {
		const auto itr = document.FindMember(key);
		ASSERT_NE(itr, document.MemberEnd());
		ASSERT_EQ((itr->value.*value_cb)(), expected);
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
