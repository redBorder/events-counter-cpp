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

#include <rapidjson/encodings.h>
#include <rapidjson/writer.h>

#include <string>
#include <vector>

template <typename T> struct json_child {
  std::string key;
  T value;
};

namespace EventsCounter {
namespace CounterFormatter {

template <typename OutputStream = rapidjson::StringBuffer,
          typename SourceEncoding = rapidjson::UTF8<>,
          typename TargetEncoding = rapidjson::UTF8<>,
          typename StackAllocator = rapidjson::CrtAllocator,
          unsigned writeFlags = rapidjson::kWriteDefaultFlags>
class JSONCountersWriter
    : public rapidjson::Writer<OutputStream, SourceEncoding, TargetEncoding,
                               StackAllocator, writeFlags> {
private:
  ///////////
  // Types //
  ///////////

  typedef rapidjson::Writer<OutputStream, SourceEncoding, TargetEncoding,
                            StackAllocator>
      Base;
  typedef typename Base::Ch Ch;

  /////////////
  // Methods //
  /////////////

  /// TODO check add value return type
  template <typename T> void dump_vars(T vars) {
    for (const auto &itr : vars) {
      add_value(itr.key);
      add_value(itr.value);
    }
  }
  static uint64_t current_unix_timestamp() { return time(nullptr); }
  bool add_value(const std::string &str) {
    return Base::String(str.c_str(), str.size());
  }
  bool add_value(const uint64_t val) { return Base::Uint64(val); }

public:
  explicit JSONCountersWriter(const std::string &uuid, const uint64_t bytes,
                              OutputStream &os,
                              StackAllocator *allocator = nullptr,
                              size_t levelDepth = Base::kDefaultLevelDepth)
      : Base(os, allocator, levelDepth) {
    Base::StartObject();

    const std::vector<struct json_child<std::string>> strings {
      {"type", "data"}, {"unit", "bytes"}, {"monitor", "uuid_received"},
          {"uuid", uuid},
    };
    const std::vector<struct json_child<uint64_t>> numbers {
      {"timestamp", current_unix_timestamp()}, {"value", bytes},
    };

    dump_vars(strings);
    dump_vars(numbers);

    Base::EndObject();
  }
};
};
};