#include <drogon/drogon.h>
#include <cstdlib>
#include <string>

int main()
{
    const char *portValue = std::getenv("PORT");
    const unsigned short port = portValue
        ? static_cast<unsigned short>(std::stoi(portValue))
        : 5555;

    const char *dbHost = std::getenv("DB_HOST");
    const char *dbName = std::getenv("DB_NAME");
    const char *dbUser = std::getenv("DB_USER");
    const char *dbPassword = std::getenv("DB_PASSWORD");
    const char *dbPortValue = std::getenv("DB_PORT");

    auto &app = drogon::app();
    app.addListener("0.0.0.0", port)
       .setDocumentRoot("./frontend");

    // Configure PostgreSQL only when deployment database variables exist.
    // Set these in the hosting dashboard; never commit credentials.
    if (dbHost && dbName && dbUser && dbPassword)
    {
        const unsigned short dbPort = dbPortValue
            ? static_cast<unsigned short>(std::stoi(dbPortValue))
            : 5432;

        app.createDbClient(
            "postgresql",
            dbHost,
            dbPort,
            dbName,
            dbUser,
            dbPassword,
            4);
    }

    app.run();
    return 0;
}