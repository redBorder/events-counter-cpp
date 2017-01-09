#include <stdint.h>
#include <string>

class UUIDConsumer {
private:
public:
  virtual ~UUIDConsumer() = 0;
  virtual std::string consume() = 0;
};
