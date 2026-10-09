#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class LoginController : public drogon::HttpController<LoginController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(LoginController::login, "/login", Post);

    METHOD_LIST_END

    void login(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);
};