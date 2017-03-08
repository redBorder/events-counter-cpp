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
#include "consumers/events_consumer.hpp"
#include "consumers/kafka_json_counter_consumer.hpp"
#include "consumers/kafka_json_uuid_consumer.hpp"
#include "consumers/kafka_json_uuid_consumer_factory.hpp"
#include "producers/kafka_json_counter_producer.hpp"
#include "producers/kafka_json_counter_producer_factory.hpp"
#include "producers/kafka_monitor_producer.hpp"
#include "utils/uuid_bytes.hpp"
#include "uuid_counter/uuid_counter.hpp"
#include "uuid_counters_monitor/uuid_counters_monitor.hpp"

#include <chrono>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <string>

using namespace std;
using namespace std::chrono;
using namespace EventsCounter::Utils;
using namespace EventsCounter::Configuration;
using namespace EventsCounter::UUIDCountersDB;
using namespace EventsCounter::UUIDCountersMonitor;
using namespace EventsCounter::UUIDCounter;
using namespace EventsCounter::Consumers;
using namespace EventsCounter::Producers;

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

static unique_ptr<Config> parse_json_config_file(const string &path) {
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
    return unique_ptr<Config>(new JsonConfig(config_text));
  } catch (const JSONParserException &e) {
    cerr << "Couldn't parse JSON config in " << path << ": " << e.what()
         << endl;
  } catch (const ifstream::failure &e) {
    cerr << "Couldn't open file " << path << ": " << e.what() << endl;
  }

  return nullptr;
}

// Using all duration, not time point. Make the code simpler.
static seconds next_tick(const seconds ticks_period, const seconds now,
                         const seconds ticks_offset = seconds(0)) {
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

static UUIDCountersDB::counters_t
make_uuid_counters_boostrap_db(vector<string> uuid_list) {
  UUIDCountersDB::counters_t boostrap_db;
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

  unique_ptr<Config> config = parse_json_config_file(config_path);
  if (config.get() == nullptr) {
    return 1;
  }

  try {
    ////////////////////////////////////////////////////////////////////////////
    // COUNTER                                                                //
    ////////////////////////////////////////////////////////////////////////////

    uuid_counter_config_s uuid_counter_config =
        config->get_uuid_counter_config();

    ///////////////////
    // UUID Consumer //
    ///////////////////

    KafkaUUIDConsumerFactory consumer_factory(
        uuid_counter_config.uuid_key, uuid_counter_config.read_topics,
        uuid_counter_config.kafka_config.consumer_rk_conf_v,
        uuid_counter_config.kafka_config.consumer_rkt_conf_v);
    unique_ptr<KafkaJSONUUIDConsumer> uuid_consumer(consumer_factory.create());

    //////////////////////
    // Counter Producer //
    //////////////////////

    KafkaJSONCounterProducerFactory producer_factory(
        config->get_uuid_counter_config());
    shared_ptr<KafkaJSONCounterProducer> counter_producer(
        producer_factory.create());

    /////////////
    // Counter //
    /////////////

    UUIDCountersDB boostrap_uuid_db(
        make_uuid_counters_boostrap_db(config->counters_uuids()));
    UUIDCounter counter(move(uuid_consumer), boostrap_uuid_db);

    ////////////////////////////////////////////////////////////////////////////
    // MONITOR                                                                //
    ////////////////////////////////////////////////////////////////////////////

    struct counters_monitor_config_s counters_monitor_config =
        config->get_monitor_config();

    //////////////////////
    // Counter Consumer //
    //////////////////////

    unique_ptr<EventsConsumer> counter_consumer(new KafkaJSONCounterConsumer(
        counters_monitor_config.period.count(),
        counters_monitor_config.offset.count(),
        counters_monitor_config.read_topic,
        counters_monitor_config.kafka_config.consumer_rk_conf_v,
        counters_monitor_config.kafka_config.consumer_rkt_conf_v));

    //////////////////////
    // Monitor Producer //
    //////////////////////

    unique_ptr<MonitorProducer> monitor_producer(
        new KafkaMonitorProducer(counters_monitor_config));

    //////////////////////
    // Counters Monitor //
    //////////////////////

    UUIDCountersDB monitor_db(
        make_uuid_counters_boostrap_db(config->counters_uuids()));
    UUIDCountersDB::counters_t limits(counters_monitor_config.limits);
    UUIDCountersMonitor monitor(monitor_db, limits);

    ///////////////
    // Main loop //
    ///////////////

    chrono::seconds now = chrono::seconds(time(NULL));
    chrono::seconds next_monitor_tick =
        next_tick(config->get_monitor_config().period, now,
                  config->get_monitor_config().offset);

    while (true) {
      const chrono::seconds next_counters_tick =
          next_tick(config->get_uuid_counter_config().update_period, now);

      ////////////////////////////////////////////
      // Monitor counters between counter ticks //
      ////////////////////////////////////////////

      while (now < next_counters_tick) {
        now = chrono::seconds(time(NULL));

        // Time to clear monitor
        if (now > next_monitor_tick) {
          monitor.reset(
              make_uuid_counters_boostrap_db(config->counters_uuids()));
          next_monitor_tick = chrono::seconds(
              next_tick(config->get_monitor_config().period, now,
                        config->get_monitor_config().offset));
        }

        UUIDBytes uuid_bytes = counter_consumer->consume(1);
        if (uuid_bytes.empty()) {
          continue;
        }

        if (monitor.check(uuid_bytes.get_uuid(), uuid_bytes.get_bytes())) {
          monitor_producer->produce(uuid_bytes);
        }
      }

      ///////////////////
      // Counter ticks //
      ///////////////////

      UUIDCountersDB::counters_t counters_uuids =
          make_uuid_counters_boostrap_db(config->counters_uuids());
      counter.swap_counters(counters_uuids);
      for (auto &t_counter : counters_uuids) {
        if (t_counter.second > 0) {
          counter_producer->produce(
              UUIDBytes(t_counter.first, t_counter.second), next_counters_tick);
          t_counter.second = 0;
        }
      }
    }
  } catch (const CreateConsumerException &e) {
    cerr << "UUIDConsumer Exception: " << e.what() << endl;
  }

  return 0;
}
