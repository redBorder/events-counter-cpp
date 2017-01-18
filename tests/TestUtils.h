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

#include <librdkafka/rdkafkacpp.h>

#include <memory>
#include <unistd.h>

namespace EventsCounter {

namespace TestUtils {

/// Create a random pattern over a given name
static char *rand_tmpl(char *tmpl) __attribute__((unused));
static char *rand_tmpl(char *tmpl) {
	int fd = mkstemp(tmpl);
	close(fd);
	remove(tmpl);

	return tmpl;
}

/// Create standard consumer config based on given brokers and group id
std::unique_ptr<RdKafka::Conf>
create_test_kafka_consumer_config(const std::string &brokers,
				  const std::string &group_id);

/// Create tests random topic
static std::string random_topic() __attribute__((unused));
static std::string random_topic() {
	char tmp_topic_tmpl[] = "rb_flow_XXXXXX";
	rand_tmpl(tmp_topic_tmpl);
	const std::string topic_str = tmp_topic_tmpl;
	const std::string group_id = std::string("group_") + tmp_topic_tmpl;

	return tmp_topic_tmpl;
}

/// Produce a single kafka message
void UUIDProduce(const std::string &uuid, const std::string &topic_str);

/// Wait some time to kafka destruction
static void wait_kafka_destroyed(void) __attribute__((unused));
static void wait_kafka_destroyed(void) {
	for (int i = 0; i < 10; ++i) {
		static const int timeout_ms = 100;
		const int destroy_rc = RdKafka::wait_destroyed(timeout_ms);
		if (0 == destroy_rc) {
			break;
		}
	}
}

}; // TestUtils namespace
}; // EventsCounter namespace
