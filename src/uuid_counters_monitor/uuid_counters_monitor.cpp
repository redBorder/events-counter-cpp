#include "uuid_counters_monitor.hpp"

using namespace EventsCounter::UUIDCountersMonitor;
using namespace std;

void UUIDCountersMonitor::uuid_inc(
    const string &uuid, uint64_t inc,
    const UUIDCountersDB::UUIDCountersDB::counters_t *limits) {

  switch (this->db->uuid_increment(uuid, inc, limits)) {
  case UUIDCountersDB::UUIDCountersDB::INCREMENT_LIMIT_REACHED:
    alert_uuid(uuid);
    break;

  case UUIDCountersDB::UUIDCountersDB::INCREMENT_NOT_EXISTS:
    break;

  case UUIDCountersDB::UUIDCountersDB::INCREMENT_NOT_EXISTS_IN_LIMITS:
    break;

  case UUIDCountersDB::UUIDCountersDB::INCREMENT_OK:
  default:
    break;
  }
}

void UUIDCountersMonitor::alert_uuid(const string &) const {
  // this->producer->produce();
}

bool UUIDCountersMonitor::check_leadership() const {
  chrono::milliseconds timeout(1000);
  return this->leadership_monitor->check_leadership(timeout);
}
