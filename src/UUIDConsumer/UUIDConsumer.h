#pragma once

#include "Utils/UUIDBytes.h"

#include <string>
namespace EventsCounter {

class UUIDConsumer {
private:
public:
	/**
	 *
	 */
	virtual ~UUIDConsumer(){};

	/**
	 * [consume description]
	 * @param  timeout [description]
	 * @return         [description]
	 */
	virtual UUIDBytes consume(uint32_t timeout) const = 0;
};
};
