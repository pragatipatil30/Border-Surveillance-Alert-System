#pragma once

#include <string>
#include <utility>

#include <crow.h>

namespace border_api {

inline crow::response successResponse(crow::json::wvalue data, std::string message)
{
    crow::json::wvalue body;
    body["success"] = true;
    body["data"] = std::move(data);
    body["message"] = std::move(message);
    return crow::response{200, body};
}

inline crow::response errorResponse(int status, std::string code, std::string message)
{
    crow::json::wvalue error;
    error["code"] = std::move(code);
    error["message"] = std::move(message);

    crow::json::wvalue body;
    body["success"] = false;
    body["error"] = std::move(error);
    return crow::response{status, body};
}

} // namespace border_api
