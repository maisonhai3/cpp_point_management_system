#include "../../include/utils/UUIDGenerator.h"
#include <uuid/uuid.h>

namespace UUIDGenerator {

std::string generateUUID() {
    uuid_t uuid;
    char uuid_str[37];  // UUID string is 36 characters + null terminator
    
    uuid_generate(uuid);
    uuid_unparse_lower(uuid, uuid_str);
    
    return std::string(uuid_str);
}

} // namespace UUIDGenerator
