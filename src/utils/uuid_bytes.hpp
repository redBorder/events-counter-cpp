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

#include <string>
#include <utility>

namespace EventsCounter {
namespace Utils {

class UUIDBytes {
private:
  std::string uuid;
  uint64_t bytes;
  uint64_t timestamp;
  bool is_empty;

public:
  UUIDBytes(std::string t_uuid, uint64_t t_bytes, uint64_t t_timestamp = 0)
      : uuid(t_uuid), bytes(t_bytes), timestamp(t_timestamp), is_empty(false) {}

  UUIDBytes() : is_empty(true) {}

  /**
   * [get_uuid description]
   * @return [description]
   */
  const std::string &get_uuid() const { return this->uuid; }

  /**
   * [get_bytes description]
   * @return [description]
   */
  uint64_t get_bytes() const { return this->bytes; }

  /**
   * [get_bytes description]
   * @return [description]
   */
  uint64_t get_timestamp() const { return this->timestamp; }

  /**
   * [empty description]
   * @return [description]
   */
  bool empty() const { return this->is_empty; }
};
};
};
