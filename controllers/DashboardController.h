#pragma once

#include <drogon/HttpController.h>

using namespace drogon;

class DashboardController
    : public drogon::HttpController<DashboardController>
{
public:
    METHOD_LIST_BEGIN

    ADD_METHOD_TO(
        DashboardController::getDashboard,
        "/seller/dashboard",
        Get);

    METHOD_LIST_END

    void getDashboard(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback);
};