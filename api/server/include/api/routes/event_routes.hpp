#pragma once

#include <string>

#include "api/http_app.hpp"

namespace border_api::routes {
void registerEventRoutes(HttpApp& app, std::string allowedOrigin);
}
