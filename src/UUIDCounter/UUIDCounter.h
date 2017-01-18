#ifndef COUNTER_H
#define COUNTER_H

#include "../UUIDConsumer/UUIDConsumer.h"
#include "../UUIDCountersDB/UUIDCountersDB.h"

#include <atomic>
#include <mutex>
#include <thread>

namespace EventsCounter {

class UUIDCounter {
private:
	UUIDConsumer *consumer;
	UUIDCountersDB *uuid_counters_db;
	std::mutex mtx;
	std::atomic<bool> running{true};
	std::thread worker{run, this, consumer};

	static void run(UUIDCounter *instance, UUIDConsumer *consumer);

public:
	/**
	 *
	 */
	UUIDCounter(UUIDConsumer *_consumer, UUIDCountersDB *_uuid_counters_db)
	    : consumer(_consumer), uuid_counters_db(_uuid_counters_db) {
	}

	/**
	 *
	 */
	~UUIDCounter();

	/**
	 *
	 */
	UUIDCounter &operator=(const UUIDCounter &) = delete;

	/**
	 * [swap_counter description]
	 * @param  counter_db [description]
	 * @return            [description]
	 */
	UUIDCountersDB *swap_counter(UUIDCountersDB *counter_db);
};
};
#endif /* COUNTER_H */
