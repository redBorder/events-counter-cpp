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

#include "config/config.hpp"
#include "config/json_config.hpp"
#include "consumers/kafka_json_counter_consumer.hpp"
#include "consumers/kafka_json_uuid_consumer.hpp"
#include "consumers/kafka_json_uuid_consumer_factory.hpp"
#include "producers/kafka_json_counter_producer.hpp"
#include "uuid_counter/uuid_counter.hpp"

#include <chrono>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::chrono;
using namespace EventsCounter;
using namespace EventsCounter::Configuration;
using namespace EventsCounter::Consumers;

static const struct option long_options[] = {
    {"help", no_argument, nullptr, 'h'},
    {"help", no_argument, nullptr, '?'},
    {"config", required_argument, nullptr, 'c'},
    {0, 0, 0, 0}};

/// @TODO
static void usage() {}

/// Return input file stream size
static ssize_t ifstream_binary_size(ifstream &is) {
  const int original_pos = is.tellg();

  is.seekg(0, is.beg);
  const int first_pos = is.tellg();

  is.seekg(0, is.end);
  const int last_pos = is.tellg();

  is.seekg(original_pos);

  return last_pos - first_pos;
}

static unique_ptr<JsonConfig> parse_json_config_file(const string &path) {
  ifstream file{};
  file.exceptions(ifstream::failbit | ifstream::badbit);

  try {
    file.open(path, ifstream::binary);
    const ssize_t file_size = ifstream_binary_size(file);
    if (file_size <= 0) {
      cerr << "Invalid length " << file_size << "of config file " << path << '.'
           << endl;
      return nullptr;
    }
    char config_text[file_size + 1];
    file.read(config_text, file_size);
    config_text[file_size] = '\0';
    return unique_ptr<JsonConfig>(new JsonConfig(config_text));
  } catch (const JSONParserException &e) {
    cerr << "Couldn't parse JSON config in " << path << ": " << e.what()
         << endl;
  } catch (const ifstream::failure &e) {
    cerr << "Couldn't open file " << path << ": " << e.what() << endl;
  }

  return nullptr;
}

// Using all duration, not time point. Make the code simpler.
static seconds next_tick(const seconds ticks_period, const seconds ticks_offset,
                         const seconds now) {

  if (ticks_period == seconds(0)) {
    cerr << "Period can't be zero" << endl;
    exit(1);
  }

  // Number of ticks since epoch
  const uint64_t n_ticks = now / ticks_period;

  // Next tick
  auto next_tick = ticks_period * n_ticks + ticks_offset;
  if (next_tick <= now) {
    // we are in duration interval, but tick is in the past
    next_tick += ticks_period;
  }

  return next_tick;
}

static UUIDCountersDB::UUIDCountersDB::counters_t
make_uuid_counters_boostrap_db(vector<string> uuid_list) {
  UUIDCountersDB::UUIDCountersDB::counters_t boostrap_db;
  for (const auto &uuid : uuid_list) {
    boostrap_db[uuid] = 0;
  }

  return boostrap_db;
}

int main(int argc, char **argv) {
  string config_path;
  int opt, optidx;

  while ((opt = getopt_long(argc, argv, "hc:", long_options, &optidx)) != EOF) {
    switch (opt) {
    case 'h':
    case '?':

      return 0;
    case 'c':
      config_path = optarg;
      break;
    default:
      // getopt should emit an error.
      break;
    };
  }

  if (config_path.empty()) {
    cerr << "You didn't specified a config file." << endl;
    usage();
    return 1;
  }

  unique_ptr<JsonConfig> config = parse_json_config_file(config_path);
  if (config.get() == nullptr) {
    return 1;
  }

  unique_ptr<KafkaUUIDConsumerFactory> factory(new KafkaUUIDConsumerFactory(
      config->get_counter_read_topics(), config->get_counter_uuid_key(),
      config->get_counter_consumer_rk_conf_v(),
      config->get_counter_consumer_rkt_conf_v()));

  UUIDCountersDB::UUIDCountersDB::counters_t aux_counters =
      make_uuid_counters_boostrap_db(config->counters_uuids());
  UUIDCountersDB::UUIDCountersDB boostrap_uuid_db(aux_counters);
  std::shared_ptr<Producers::KafkaJSONCounterProducer> producer =
      config->get_counters_producer();

  /// @TODO UUID counter should accept consumer in unique_ptr<> reference
  /// to reference
  try {
    unique_ptr<KafkaJSONUUIDConsumer> consumer(factory->create());
    UUIDCounter::UUIDCounter counter(consumer.release(), boostrap_uuid_db);

    for (;;) {
      const chrono::seconds ticks_period = config->get_counters_timer_period();
      const chrono::seconds ticks_offset = config->get_counters_timer_offset();

      chrono::seconds now = chrono::seconds(std::time(NULL));
      const chrono::seconds next_counters_tick =
          next_tick(ticks_period, ticks_offset, now);

      // Do idle tasks until I need something
      while (now < next_counters_tick) {
        this_thread::sleep_for(next_counters_tick - now);
        now = chrono::seconds(std::time(NULL));
      }

      // Tick! produce UUID messages and clear counter
      counter.swap_counters(aux_counters);
      for (auto &t_counter : aux_counters) {

        if (t_counter.second > 0) {
          producer->produce(Utils::UUIDBytes(t_counter.first, t_counter.second),
                            next_counters_tick);
          t_counter.second = 0;
        }
      }
    }
  } catch (const Consumers::CreateConsumerException &e) {
    cerr << "UUIDConsumer Exception: " << e.what() << endl;
  }

  return 0;
}
