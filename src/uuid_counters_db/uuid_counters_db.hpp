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

#include <cstdint>
#include <map>
#include <string>

namespace EventsCounter {
namespace UUIDCountersDB {

/// Per UUId counter database
class UUIDCountersDB {
public:
  /// UUID
  typedef std::string uuid;

  /// supported external counter
  typedef std::map<uuid, uint64_t> counters_t;

  /// Default constructor with initialized data
  UUIDCountersDB(const counters_t &base) : counters(base) {}

  /// Increment result code
  typedef enum {
    /// Increment ok
    INCREMENT_OK,

    /// Per client limit reached
    INCREMENT_LIMIT_REACHED,

    /// Client does not exists in database
    INCREMENT_NOT_EXISTS,

    /// Client does not exists in limits
    INCREMENT_NOT_EXISTS_IN_LIMITS,
  } increment_result_t;

  /// Increments UUID counter
  increment_result_t uuid_increment(const std::string &t_uuid, const uint64_t n,
                                    const counters_t *limits = NULL);

  /// Swap counters with external source
  void swap(counters_t &other) { this->counters.swap(other); }

private:
  counters_t counters;

  const UUIDCountersDB &operator=(const UUIDCountersDB &other);
};
};
};
