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

#include <librdkafka/rdkafkacpp.h>

#include <iostream>

namespace EventsCounter {
namespace Configuration {

class EventCounterCb : public RdKafka::DeliveryReportCb {
public:
  virtual void dr_cb(RdKafka::Message &message) {
    if (message.err() != RdKafka::ERR_NO_ERROR) {
      std::cerr << "Couldn't deliver [";
      std::cerr.write(static_cast<char *>(message.payload()), message.len())
          << "] to topic [" << message.topic_name() << ':'
          << message.partition() << "]. Error: " << message.errstr()
          << std::endl;
    }
  }
};

static EventCounterCb events_counter_cb;
};
};
