#pragma once

#include <string>
#include <utility>

namespace EventsCounter {

class UUIDBytes {
private:
	std::pair<std::string, uint64_t> pair;
	bool is_empty;

public:
	/**
	 *
	 */
	UUIDBytes(std::string uuid, uint64_t bytes)
	    : pair(uuid, bytes), is_empty(false) {
	}

	/**
	 *
	 */
	UUIDBytes() : is_empty(true) {
	}

	/**
	 * [get_uuid description]
	 * @return [description]
	 */
	const std::string &get_uuid() const {
		return this->pair.first;
	}

	/**
	 * [get_bytes description]
	 * @return [description]
	 */
	uint64_t get_bytes() const {
		return this->pair.second;
	}

	/**
	 * [empty description]
	 * @return [description]
	 */
	bool empty() const {
		return this->is_empty;
	}
};
};
