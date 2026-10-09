#include "RegisterController.h"
#include <drogon/drogon.h>
#include <sodium.h>
#include <string>

void RegisterController::registerUser(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    auto json = req->getJsonObject();

    Json::Value response;

    if (!json ||
        !json->isMember("name") ||
        !json->isMember("email") ||
        !json->isMember("password") ||
        !json->isMember("role"))
    {
        response["success"] = false;
        response["message"] =
            "Name, email, password and role are required";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    std::string name = (*json)["name"].asString();
    std::string email = (*json)["email"].asString();
    std::string password = (*json)["password"].asString();
    std::string role = (*json)["role"].asString();

    if (name.empty() || email.empty() || password.empty())
    {
        response["success"] = false;
        response["message"] = "All fields are required";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    if (role != "buyer" && role != "seller")
    {
        response["success"] = false;
        response["message"] = "Role must be buyer or seller";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    if (sodium_init() < 0)
    {
        response["success"] = false;
        response["message"] =
            "Password security initialization failed";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    char hashedPassword[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(
            hashedPassword,
            password.c_str(),
            static_cast<unsigned long long>(password.size()),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0)
    {
        response["success"] = false;
        response["message"] = "Password hashing failed";

        callback(HttpResponse::newHttpJsonResponse(response));
        return;
    }

    std::string passwordHash = hashedPassword;
    auto dbClient = app().getDbClient();

    dbClient->execSqlAsync(
        "INSERT INTO users (name, email, password, role) "
        "VALUES ($1, $2, $3, $4) "
        "RETURNING id, name, email, role",

        [callback](const drogon::orm::Result& result)
        {
            Json::Value response;

            if (result.size() == 1)
            {
                response["success"] = true;
                response["message"] =
                    "Account created successfully";
                response["id"] = result[0]["id"].as<int>();
                response["name"] =
                    result[0]["name"].as<std::string>();
                response["email"] =
                    result[0]["email"].as<std::string>();
                response["role"] =
                    result[0]["role"].as<std::string>();
            }
            else
            {
                response["success"] = false;
                response["message"] =
                    "Account creation failed";
            }

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        [callback](const drogon::orm::DrogonDbException& e)
        {
            Json::Value response;
            response["success"] = false;

            std::string error = e.base().what();

            if (error.find("duplicate key") != std::string::npos)
            {
                response["message"] =
                    "Email already registered";
            }
            else
            {
                response["message"] = "Database error";
            }

            LOG_ERROR << "Registration error: " << error;

            callback(HttpResponse::newHttpJsonResponse(response));
        },

        name,
        email,
        passwordHash,
        role);
}