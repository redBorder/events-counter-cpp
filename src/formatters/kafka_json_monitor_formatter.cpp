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

#include "json_kafka_message.hpp"
#include "json_monitor_writter.hpp"
#include "kafka_json_monitor_formatter.hpp"

using namespace EventsCounter::Formatters;
using namespace std;

unique_ptr<RdKafka::Message>
KafkaJSONMonitorFormatter::format(Utils::UUIDBytes &uuid_bytes) {
  unique_ptr<JSONKafkaMessage> ret(new JSONKafkaMessage(uuid_bytes.get_uuid()));
  JSONMonitorWritter<>(uuid_bytes.get_uuid(), uuid_bytes.get_bytes(),
                       ret->string_buffer);
  return std::move(ret);
}
