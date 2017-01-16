#include <string>

#include <rapidjson/document.h>

namespace EventsCounter {
class JSON : public rapidjson::Document {
public:
	JSON(char *buf, size_t len);
};
};
