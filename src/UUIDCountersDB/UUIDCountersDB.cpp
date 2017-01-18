/*
  Copyright (C) 2017 Eneo Tecnologia S.L.
  Authors: Diego Fernandez <bigomby@gmail.com>
	   Eugenio Perez <eupm90@gmail.com>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU Affero General Public License as
  published by the Free Software Foundation, either version 3 of the
  License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UUIDCountersDB.h"

using namespace EventsCounter;

UUIDCountersDB::increment_result_t
UUIDCountersDB::uuid_increment(const std::string &t_uuid,
			       const uint64_t n,
			       const counters_t *limits) {
	counters_t::iterator it = this->counters.find(t_uuid);
	if (it == this->counters.end()) {
		return INCREMENT_NOT_EXISTS;
	}

	it->second += n;
	if (!limits) {
		return INCREMENT_OK;
	}

	const counters_t::const_iterator limits_it = limits->find(t_uuid);
	if (limits->end() == limits_it) {
		return INCREMENT_NOT_EXISTS_IN_LIMITS;
	} else if (it->second > limits_it->second) {
		return INCREMENT_LIMIT_REACHED;
	}

	return INCREMENT_OK;
}
