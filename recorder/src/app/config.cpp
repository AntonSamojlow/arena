#include "config.h"

namespace spdlog::level {

// NOLINTBEGIN: nlohmann macros violate plenty checks
NLOHMANN_JSON_SERIALIZE_ENUM(level_enum,
	{
		{off, "off"},
		{critical, "critical"},
		{trace, "trace"},
		{info, "info"},
		{err, "err"},
		{warn, "warn"},
		{debug, "debug"},
	})
// NOLINTEND

}  // namespace spdlog::level