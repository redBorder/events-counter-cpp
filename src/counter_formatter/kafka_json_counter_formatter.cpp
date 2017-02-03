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

#include "json_counters_writter.hpp"
#include "json_kafka_message.hpp"
#include "kafka_json_counter_formatter.hpp"

using namespace std;
using namespace EventsCounter::CounterFormatter;

unique_ptr<RdKafka::Message>
KafkaJSONCounterFormatter::format(const string &uuid, const uint64_t bytes) {
  unique_ptr<JSONKafkaMessage> ret(new JSONKafkaMessage(uuid));
  JSONCountersWriter<>(uuid, bytes, ret->string_buffer);
  return std::move(ret);
}
