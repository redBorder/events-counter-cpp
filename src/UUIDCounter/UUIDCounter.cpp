#include "UUIDCounter.hpp"

using namespace std;

void UUIDCounter::run(UUIDCounter *instance, UUIDConsumer *consumer) {
  unique_ptr<UUIDConsumer> consumer_ptr(consumer);

  while (instance->running) {
    string uuid = consumer_ptr->consume();
    lock_guard<mutex> lock(instance->mtx);
    instance->uuid_counters_db->incr(uuid, 1);
  }
}

UUIDCounter::~UUIDCounter() { this->running = false; }

UUIDCountersDB *UUIDCounter::swap_counter(UUIDCountersDB *_uuid_counters_db) {
  lock_guard<mutex> lock(this->mtx);
  UUIDCountersDB *tmp_db = this->uuid_counters_db;
  this->uuid_counters_db = _uuid_counters_db;

  return tmp_db;
}
