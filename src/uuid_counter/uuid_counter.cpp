#include "uuid_counter.hpp"

#include <iostream>
#include <map>

using namespace std;
using namespace EventsCounter::UUIDCounter;

void UUIDCounter::run(UUIDCounter *instance,
		      UUIDConsumer::UUIDConsumer *consumer) {
	while (instance->running.load()) {
		Utils::UUIDBytes data = consumer->consume(1000);
		if (data.empty()) {
			continue;
		}
		UUIDCountersDB::increment_result_t increment_rc;
		{
			lock_guard<mutex> lock(instance->mtx);
			increment_rc = instance->uuid_counters_db.uuid_increment(
					data.get_uuid(), data.get_bytes());
		}
		switch (increment_rc) {
		case UUIDCountersDB::INCREMENT_NOT_EXISTS:
			cerr << "UUID " << data.get_uuid()
			     << " does not exists in db" << endl;
			break;
		case UUIDCountersDB::INCREMENT_NOT_EXISTS_IN_LIMITS:
		case UUIDCountersDB::INCREMENT_LIMIT_REACHED:
		case UUIDCountersDB::INCREMENT_OK:
		default:
			break;
		}
	}
}

UUIDCounter::~UUIDCounter() {
	this->running.store(false);
	this->worker.join();
}

void UUIDCounter::swap_counters(UUIDCountersDB::counters_t &_uuid_counters_db) {
	lock_guard<mutex> lock(this->mtx);
	this->uuid_counters_db.swap(_uuid_counters_db);
}
