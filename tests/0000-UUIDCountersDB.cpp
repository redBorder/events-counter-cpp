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

#include "../src/uuid_counters_db/uuid_counters_db.hpp"

#include <gtest/gtest.h>
#include <memory>

namespace {
using namespace std;
using namespace EventsCounter::UUIDCountersDB;

class UUIDConsumerTest : public ::testing::Test {};

typedef UUIDCountersDB::counters_t CountersMap;

TEST_F(UUIDConsumerTest, increment_test) {
  const CountersMap boostrap_map = {
      {"a", 50}, {"b", 100}, {"c", 150}, {"d", 200}};
  UUIDCountersDB db(boostrap_map);

  /* Try to add with no limit */
  const UUIDCountersDB::increment_result_t rc = db.uuid_increment("a", 50);
  EXPECT_EQ(UUIDCountersDB::INCREMENT_OK, rc);

  CountersMap checkmap;
  db.swap(checkmap);
  CountersMap expected_map{
      {"a", 100}, {"b", 100}, {"c", 150}, {"d", 200},
  };
  EXPECT_EQ(expected_map, checkmap);
}

TEST_F(UUIDConsumerTest, undefined_uuid_test) {
  const CountersMap boostrap_map = {
      {"a", 50}, {"b", 100}, {"c", 150}, {"d", 200}};

  UUIDCountersDB db(boostrap_map);

  /* Try to add with no limit */
  const UUIDCountersDB::increment_result_t rc = db.uuid_increment("g", 50);
  EXPECT_EQ(UUIDCountersDB::INCREMENT_NOT_EXISTS, rc);

  CountersMap checkmap;
  db.swap(checkmap);
  EXPECT_EQ(boostrap_map, checkmap);
}

TEST_F(UUIDConsumerTest, uuid_limit_test) {
  const CountersMap boostrap_map = {{"a", 50},
                                    {"b", 100},
                                    {"c", 150},
                                    {"d", 200}},
                    limits = {{"a", 120}, {"b", 10}, {"c", 200}};
  UUIDCountersDB db(boostrap_map);

  // 1st increment should be OK
  const UUIDCountersDB::increment_result_t rc1 =
      db.uuid_increment("a", 50, &limits);
  EXPECT_EQ(UUIDCountersDB::INCREMENT_OK, rc1);

  // 2nd increment should raise limit reached
  const UUIDCountersDB::increment_result_t rc2 =
      db.uuid_increment("a", 50, &limits);
  EXPECT_EQ(UUIDCountersDB::INCREMENT_LIMIT_REACHED, rc2);

  // 3rd increment should also raise limit reached
  const UUIDCountersDB::increment_result_t rc3 =
      db.uuid_increment("a", 50, &limits);
  EXPECT_EQ(UUIDCountersDB::INCREMENT_LIMIT_REACHED, rc3);

  // counter should have all read elements
  CountersMap checkmap;
  db.swap(checkmap);
  CountersMap expected_counter{
      {"a", 200}, {"b", 100}, {"c", 150}, {"d", 200},
  };
  EXPECT_EQ(expected_counter, checkmap);
  db.swap(checkmap); // Restore state

  // Try to increment an uuid that does not exists in limits
  const UUIDCountersDB::increment_result_t rc4 =
      db.uuid_increment("d", 50, &limits);
  EXPECT_EQ(UUIDCountersDB::INCREMENT_NOT_EXISTS_IN_LIMITS, rc4);
}
};

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
