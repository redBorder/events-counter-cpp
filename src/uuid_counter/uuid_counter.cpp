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

#include "uuid_counter.hpp"

#include <iostream>
#include <map>

using namespace std;
using namespace EventsCounter::UUIDCounter;

void UUIDCounter::run(UUIDCounter *instance,
                      Consumers::KafkaJSONUUIDConsumer *consumer) {
  while (instance->running.load()) {
    Utils::UUIDBytes data = consumer->consume(1000);
    if (data.empty()) {
      continue;
    }
    UUIDCountersDB::UUIDCountersDB::increment_result_t increment_rc;
    {
      lock_guard<mutex> lock(instance->mtx);
      increment_rc = instance->uuid_counters_db.uuid_increment(
          data.get_uuid(), data.get_bytes());
    }
    switch (increment_rc) {
    case UUIDCountersDB::UUIDCountersDB::INCREMENT_NOT_EXISTS:
      cerr << "UUID " << data.get_uuid() << " does not exists in db" << endl;
      break;
    case UUIDCountersDB::UUIDCountersDB::INCREMENT_NOT_EXISTS_IN_LIMITS:
    case UUIDCountersDB::UUIDCountersDB::INCREMENT_LIMIT_REACHED:
    case UUIDCountersDB::UUIDCountersDB::INCREMENT_OK:
    default:
      break;
    }
  }
}

UUIDCounter::~UUIDCounter() {
  this->running.store(false);
  this->worker.join();
}

void UUIDCounter::swap_counters(
    UUIDCountersDB::UUIDCountersDB::counters_t &_uuid_counters_db) {
  lock_guard<mutex> lock(this->mtx);
  this->uuid_counters_db.swap(_uuid_counters_db);
}
