#ifndef COUNTER_H
#define COUNTER_H

#include <iostream>
#include <librdkafka/rdkafkacpp.h>
#include <mutex>
#include <thread>
#include <unistd.h>

#include "../UUIDConsumer/UUIDConsumer.hpp"
#include "../UUIDCountersDB/UUIDCountersDB.hpp"

class UUIDCounter {
private:
  bool running = true;
  UUIDConsumer *consumer;
  UUIDCountersDB *uuid_counters_db;
  std::mutex mtx;
  std::thread worker{run, this, consumer};

  static void run(UUIDCounter *instance, UUIDConsumer *consumer);

public:
  UUIDCounter(UUIDConsumer *_consumer, UUIDCountersDB *_uuid_counters_db)
      : consumer(_consumer), uuid_counters_db(_uuid_counters_db) {}
  ~UUIDCounter();
  UUIDCounter &operator=(const UUIDCounter &) = delete;
  UUIDCountersDB *swap_counter(UUIDCountersDB *counter_db);
};

#endif /* COUNTER_H */
