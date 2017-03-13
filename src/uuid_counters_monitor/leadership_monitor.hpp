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

#include <chrono>
#include <iostream>

namespace EventsCounter {
namespace UUIDCountersMonitor {

/**
 * Interface used to select a leader among other instances.
 */
class LeadershipMonitor {
public:
  virtual ~LeadershipMonitor(){};

  /**
   * Returns if the instance is the leader or not.
   *
   * @param  timeout_ms Max time to wait for a rebalance.
   * @return            True if leadership has been acquired. Flase in
   *                    other case
   */
  virtual bool check_leadership(std::chrono::milliseconds timeout_ms) const = 0;
};
};
};
