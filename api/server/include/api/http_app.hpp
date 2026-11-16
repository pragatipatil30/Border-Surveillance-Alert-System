#pragma once

#include <crow.h>
#include <crow/middlewares/cors.h>

namespace border_api {
using HttpApp = crow::App<crow::CORSHandler>;
}
