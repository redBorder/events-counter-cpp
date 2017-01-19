#pragma once

#include "../Utils/UUIDBytes.h"

namespace EventsCounter {

class UUIDProducer {
private:
public:
	/**
	 *
	 */
	virtual ~UUIDProducer() = 0;

	/**
	 * [produce description]
	 * @param  timeout [description]
	 * @return         [description]
	 */
	virtual UUIDBytes produce() const = 0;
};
};
