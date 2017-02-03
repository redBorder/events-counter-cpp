#pragma once

#include "../UUIDCountersDB/UUIDCountersDB.h"
#include "../uuid_consumer/uuid_consumer.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

namespace EventsCounter {
namespace UUIDCounter {

class UUIDCounter {
private:
	std::unique_ptr<UUIDConsumer::UUIDConsumer> consumer;
	UUIDCountersDB uuid_counters_db;
	std::mutex mtx;
	std::atomic<bool> running{true};
	std::thread worker{run, this, this->consumer.get()};

	static void
	run(UUIDCounter *instance, UUIDConsumer::UUIDConsumer *consumer);

public:
	UUIDCounter(UUIDConsumer::UUIDConsumer *t_consumer,
		    UUIDCountersDB counters_boostrap)
	    : consumer(t_consumer), uuid_counters_db(counters_boostrap) {
	}
	~UUIDCounter();
	UUIDCounter &operator=(const UUIDCounter &) = delete;

	/**
	 * [swap_counter description]
	 * @param  counter_db [description]
	 * @return            [description]
	 */
	void swap_counters(UUIDCountersDB::counters_t &counter_db);
};
};
};
