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
	std::pair<std::string, uint64_t> pair;
	bool is_empty;

public:
	UUIDBytes(std::string uuid, uint64_t bytes)
	    : pair(uuid, bytes), is_empty(false) {
	}

	UUIDBytes() : is_empty(true) {
	}

	/**
	 * [get_uuid description]
	 * @return [description]
	 */
	const std::string &get_uuid() const {
		return this->pair.first;
	}

	/**
	 * [get_bytes description]
	 * @return [description]
	 */
	uint64_t get_bytes() const {
		return this->pair.second;
	}

	/**
	 * [empty description]
	 * @return [description]
	 */
	bool empty() const {
		return this->is_empty;
	}
};
};
};
