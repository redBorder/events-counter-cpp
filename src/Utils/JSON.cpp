#include "JSON.h"

#include <iostream>

using namespace EventsCounter;
using namespace rapidjson;
using namespace std;

template <typename Encoding = UTF8<>> class SizedBufferStream {
public:
	typedef typename Encoding::Ch Ch;

	SizedBufferStream(Ch *t_msg, size_t t_msg_size)
	    : payload(t_msg), size(t_msg_size), read_pos(0), write_pos(0) {
	}

	Ch Peek() const {
		return this->read_pos < this->size
				       ? this->payload[this->read_pos]
				       : '\0';
	}

	Ch Take() {
		return this->read_pos < this->size
				       ? this->payload[this->read_pos++]
				       : '\0';
	}

	size_t Tell() {
		return this->read_pos;
	}

	void Put(Ch c) {
		if (this->write_pos < this->size) {
			this->payload[this->write_pos++] = c;
		}
	}

	Ch *PutBegin() {
		return &this->payload[this->write_pos = this->read_pos];
	}

	size_t PutEnd(Ch *begin) {
		return write_pos - static_cast<size_t>(begin - this->payload);
	}

private:
	Ch *payload;
	size_t size, read_pos, write_pos;
};

JSON::JSON(char *buf, size_t size) {
	SizedBufferStream<> is(buf, size);
	this->ParseStream<kParseDefaultFlags | kParseInsituFlag>(is);
}
