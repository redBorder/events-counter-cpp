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

#include "uuid_counters_monitor.hpp"

#include <iostream>

using namespace EventsCounter::UUIDCountersDB;
using namespace EventsCounter::UUIDCountersMonitor;
using namespace EventsCounter::Consumers;
using namespace EventsCounter::Producers;
using namespace EventsCounter::Utils;
using namespace std;

UUIDCountersMonitor::UUIDCountersMonitor(
    UUIDCountersDB::UUIDCountersDB t_db,
    UUIDCountersDB::UUIDCountersDB::counters_t t_limits)
    : db(t_db), limits(t_limits) {}

bool UUIDCountersMonitor::check(const string &uuid, uint64_t bytes) {
  UUIDBytes uuid_bytes(uuid, bytes);

  UUIDCountersDB::UUIDCountersDB::increment_result_t increment_rc =
      this->db.uuid_increment(uuid, bytes, &this->limits);
  switch (increment_rc) {
  case UUIDCountersDB::UUIDCountersDB::INCREMENT_LIMIT_REACHED:
    return true;

  case UUIDCountersDB::UUIDCountersDB::INCREMENT_NOT_EXISTS:
    cerr << "UUID " << uuid_bytes.get_uuid() << " does not exists in db"
         << endl;
    break;

  case UUIDCountersDB::UUIDCountersDB::INCREMENT_NOT_EXISTS_IN_LIMITS:
    cerr << "UUID " << uuid_bytes.get_uuid() << " does not exists in limits"
         << endl;
    break;

  case UUIDCountersDB::UUIDCountersDB::INCREMENT_OK:
  default:
    break;
  }

  return false;
}

void UUIDCountersMonitor::reset(
    UUIDCountersDB::UUIDCountersDB::counters_t counters) {
  this->db.swap(counters);
}
