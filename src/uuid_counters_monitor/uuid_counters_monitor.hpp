#pragma once

#include "../uuid_consumer/uuid_consumer.hpp"
#include "../uuid_counters_db/uuid_counters_db.hpp"
#include "leadership_monitor.hpp"

#include <memory>

namespace EventsCounter {
namespace UUIDCountersMonitor {

class UUIDCountersMonitor {
private:
  std::unique_ptr<UUIDCountersDB::UUIDCountersDB> db;
  // TODO CounterProducer
  std::unique_ptr<LeadershipMonitor> leadership_monitor;

  void alert_uuid(const std::string &uuid) const;

public:
  UUIDCountersMonitor(UUIDCountersDB::UUIDCountersDB *t_db,
                      LeadershipMonitor *t_leadership_monitor)
      : db(t_db), leadership_monitor(t_leadership_monitor) {}

  void uuid_inc(const std::string &uuid, uint64_t inc);

  void uuid_inc(const std::string &uuid, uint64_t inc,
                const UUIDCountersDB::UUIDCountersDB::counters_t *limits);

  bool check_leadership() const;
};
};
};
