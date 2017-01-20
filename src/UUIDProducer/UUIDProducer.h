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

#pragma once

#include "Utils/UUIDBytes.h"

#include <chrono>
#include <memory>
#include <thread>

namespace EventsCounter {

class UUIDProducer {
public:
	virtual ~UUIDProducer(){};

	typedef enum {
		NO_ERROR,
		ERR_QUEUE_FULL,
		ERR_MSG_TOO_LARGE,
	} ErrorCode;
	virtual ErrorCode
	produce(const UUIDBytes &counter, std::chrono::seconds timestamp) = 0;
	/// no idle tasks by default
	virtual void
	do_idle_tasks(std::chrono::milliseconds timeout =
				      std::chrono::milliseconds(0)) {
		std::this_thread::sleep_for(timeout);
	}
};

}; // namespace EventsCounter
