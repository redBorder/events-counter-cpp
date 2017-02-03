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

#include "json_zerocopy.hpp"

#include <iostream>

using namespace EventsCounter::Utils;
using namespace rapidjson;
using namespace std;

namespace {
template <typename Encoding = UTF8<>> class SizedBufferStream {
public:
  typedef typename Encoding::Ch Ch;

  SizedBufferStream(Ch *t_msg, size_t t_msg_size)
      : payload(t_msg), size(t_msg_size), read_pos(0), write_pos(0) {}

  Ch Peek() const {
    return this->read_pos < this->size ? this->payload[this->read_pos] : '\0';
  }

  Ch Take() {
    return this->read_pos < this->size ? this->payload[this->read_pos++] : '\0';
  }

  size_t Tell() { return this->read_pos; }

  void Put(Ch c) {
    if (this->write_pos < this->size) {
      this->payload[this->write_pos++] = c;
    }
  }

  Ch *PutBegin() { return &this->payload[this->write_pos = this->read_pos]; }

  size_t PutEnd(Ch *begin) {
    return write_pos - static_cast<size_t>(begin - this->payload);
  }

private:
  Ch *payload;
  size_t size, read_pos, write_pos;
};
};

JSON::JSON(char *buf, size_t size) {
  SizedBufferStream<> is(buf, size);
  this->ParseStream<kParseDefaultFlags | kParseInsituFlag>(is);
}
