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

#include "../uuid_consumer/uuid_consumer.hpp"
#include "../uuid_counters_db/uuid_counters_db.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

namespace EventsCounter {
namespace UUIDCounter {

class UUIDCounter {
private:
  std::unique_ptr<UUIDConsumer::UUIDConsumer> consumer;
  UUIDCountersDB::UUIDCountersDB uuid_counters_db;
  std::mutex mtx;
  std::atomic<bool> running{true};
  std::thread worker{run, this, this->consumer.get()};

  static void run(UUIDCounter *instance, UUIDConsumer::UUIDConsumer *consumer);

public:
  UUIDCounter(UUIDConsumer::UUIDConsumer *t_consumer,
              UUIDCountersDB::UUIDCountersDB counters_boostrap)
      : consumer(t_consumer), uuid_counters_db(counters_boostrap) {}
  ~UUIDCounter();
  UUIDCounter &operator=(const UUIDCounter &) = delete;

  /**
   * [swap_counter description]
   * @param  counter_db [description]
   * @return            [description]
   */
  void swap_counters(UUIDCountersDB::UUIDCountersDB::counters_t &counter_db);
};
};
};
