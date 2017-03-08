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

#include "kafka_utils.hpp"

#include <iostream>

using namespace std;
using namespace RdKafka;
using namespace EventsCounter::Utils;

void EventsCounter::Utils::rdkafka_set_conf_vector(
    const vector<pair<std::string, string>> &conf_parameters,
    const unique_ptr<Conf> &conf, const string &err_conf_type) {

  string errstr;
  for (const auto &itr : conf_parameters) {
    const Conf::ConfResult rc = conf->set(itr.first, itr.second, errstr);

    switch (rc) {
    case Conf::CONF_UNKNOWN:
      cerr << "Unknown " << err_conf_type << " property " << itr.first << ": "
           << errstr << endl;
      continue;

    case Conf::CONF_INVALID:
      cerr << "Unknown " << err_conf_type << " property value " << itr.second
           << " for key " << itr.first << ": " << errstr << endl;
      continue;

    case Conf::CONF_OK:
      break;

    default:
      cerr << "Unknown " << err_conf_type << " property " << itr.first << ": "
           << errstr << endl;
      break;
    };
  }
}
