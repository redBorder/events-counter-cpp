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

#pragma once

#include "../consumers/events_consumer.hpp"
#include "../producers/monitor_producer.hpp"
#include "../uuid_counters_db/uuid_counters_db.hpp"

#include <memory>

namespace EventsCounter {
namespace UUIDCountersMonitor {

class UUIDCountersMonitor {
private:
  UUIDCountersDB::UUIDCountersDB db;
  UUIDCountersDB::UUIDCountersDB::counters_t limits;

public:
  UUIDCountersMonitor(UUIDCountersDB::UUIDCountersDB db,
                      UUIDCountersDB::UUIDCountersDB::counters_t limits);

  void reset(UUIDCountersDB::UUIDCountersDB::counters_t counters);
  bool check(const std::string &uuid, uint64_t bytes);
};
};
};
