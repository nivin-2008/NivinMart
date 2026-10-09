#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class RegisterController : public drogon::HttpController<RegisterController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(RegisterController::registerUser, "/register", Post);

    METHOD_LIST_END

    void registerUser(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);
};