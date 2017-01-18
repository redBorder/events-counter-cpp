#include "UUIDCounter.h"

#include <map>

using namespace std;
using namespace EventsCounter;

void UUIDCounter::run(UUIDCounter *instance, UUIDConsumer *consumer) {
	unique_ptr<UUIDConsumer> consumer_ptr(consumer);

	while (instance->running.load()) {
		UUIDBytes data = consumer_ptr->consume(1000);
		lock_guard<mutex> lock(instance->mtx);
		instance->uuid_counters_db->uuid_increment(data.get_uuid(), 1);
	}
}

UUIDCounter::~UUIDCounter() {
	this->running.store(false);
}

UUIDCountersDB *UUIDCounter::swap_counter(UUIDCountersDB *_uuid_counters_db) {
	lock_guard<mutex> lock(this->mtx);
	UUIDCountersDB *tmp_db = this->uuid_counters_db;
	this->uuid_counters_db = _uuid_counters_db;

	return tmp_db;
}
