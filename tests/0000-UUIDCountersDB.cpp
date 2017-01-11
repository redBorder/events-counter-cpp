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

#include "UUIDCountersDB/UUIDCountersDB.h"
#include <memory>
#include <exception>

using namespace std;
using namespace EventsCounter;

class TestException: public exception {
public:
	TestException(const char *t_what): m_what(t_what) {};
	virtual const char* what() const throw() {
		return m_what;
	}

private:
	const char *m_what;
};

typedef UUIDCountersDB::counters_t CountersMap;

static void increment_test() {
	const CountersMap boostrap_map = {
		{"a", 50}, {"b", 100}, {"c", 150}, {"d", 200}
	};
	UUIDCountersDB db(boostrap_map);

	/* Try to add with no limit */
	const UUIDCountersDB::increment_result_t rc = db.uuid_increment("a", 50);
	if (rc != UUIDCountersDB::INCREMENT_OK) {
		throw TestException(__func__);
	}

	CountersMap checkmap;
	db.swap(checkmap);
	if (CountersMap({{"a", 100}, {"b", 100}, {"c", 150}, {"d", 200}})
			!= checkmap) {
		throw TestException(__func__);
	}
}

static void undefined_uuid_test() {
	const CountersMap boostrap_map = {{"a", 50}, {"b", 100}, {"c", 150},
		{"d", 200}};

	UUIDCountersDB db(boostrap_map);

	/* Try to add with no limit */
	const UUIDCountersDB::increment_result_t rc = db.uuid_increment("g", 50);
	if (rc != UUIDCountersDB::INCREMENT_NOT_EXISTS) {
		throw TestException(__func__);
	}

	CountersMap checkmap;
	db.swap(checkmap);
	if (boostrap_map != checkmap) {
		throw TestException(__func__);
	}
}

static void uuid_limit_test() {
	const CountersMap boostrap_map = {
			{"a", 50}, {"b", 100}, {"c", 150}, {"d", 200}
		},
		limits = {
			{"a", 120}, {"b", 10}, {"c", 200}
		};
	UUIDCountersDB db(boostrap_map);


	// 1st increment should be OK
	const UUIDCountersDB::increment_result_t rc1 = db.uuid_increment("a",
		50, &limits);
	if (rc1 != UUIDCountersDB::INCREMENT_OK) {
		throw TestException(__func__);
	}

	// 2nd increment should raise limit reached
	const UUIDCountersDB::increment_result_t rc2 = db.uuid_increment("a",
		50, &limits);
	if (rc2 != UUIDCountersDB::INCREMENT_LIMIT_REACHED) {
		throw TestException(__func__);
	}

	// 3rd increment should also raise limit reached
	const UUIDCountersDB::increment_result_t rc3 = db.uuid_increment("a",
		50, &limits);
	if (rc3 != UUIDCountersDB::INCREMENT_LIMIT_REACHED) {
		throw TestException(__func__);
	}

	// counter should have all read elements
	CountersMap checkmap;
	db.swap(checkmap);
	if (CountersMap{{"a", 200}, {"b", 100}, {"c", 150}, {"d", 200}}
			!= checkmap) {
		throw TestException(__func__);
	}
	db.swap(checkmap); // Restore state

	// Try to increment an uuid that does not exists in limits
	const UUIDCountersDB::increment_result_t rc4 = db.uuid_increment("d",
		50, &limits);
	if (rc4 != UUIDCountersDB::INCREMENT_NOT_EXISTS_IN_LIMITS) {
		throw TestException(__func__);
	}
}

int main(void) {
	increment_test();
	undefined_uuid_test();
	uuid_limit_test();
	return 0;
}
