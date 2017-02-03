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

using namespace std;
using namespace EventsCounter::CounterFormatter;

JSONKafkaMessage::JSONKafkaMessage(string t_kafka_key, int32_t t_partition,
                                   void *t_opaque,
                                   RdKafka::MessageTimestamp t_timestamp)
    : m_kafka_key(t_kafka_key), m_partition(t_partition), m_opaque(t_opaque),
      m_timestamp(t_timestamp) {}
