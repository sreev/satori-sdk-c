#ifndef TEST_CONFIG_H__INCLUDED
#define TEST_CONFIG_H__INCLUDED

#include <string>
#include <fstream>
#include <nlohmann_json/json.hpp>

using json = nlohmann::json;

const char *ws_endpoint = nullptr;
const char *wss_endpoint = nullptr;
const char *appkey = nullptr;
const char *role_secret = nullptr;

void load_credentials(void) {
    try {
        std::ifstream creds_stream("credentials.json");
        std::stringstream buffer;
        buffer << creds_stream.rdbuf();
        json const creds = json::parse(buffer);

        std::string const endpoint_s = creds["endpoint"];
        std::string const appkey_s = creds["appkey"];
        std::string const superuser_role_secret_s = creds["superuser_role_secret"];

        std::string wss_endpoint_s(endpoint_s);
        std::string ws_endpoint_s(endpoint_s);

        if (wss_endpoint_s.substr(0, 3) == "wss") {
            ws_endpoint_s.replace(0, 3, "ws");
        } else {
            wss_endpoint_s.replace(0, 2, "wss");
        }

        ws_endpoint = strdup(ws_endpoint_s.c_str());
        wss_endpoint = strdup(wss_endpoint_s.c_str());
        appkey = strdup(appkey_s.c_str());
        role_secret = strdup(superuser_role_secret_s.c_str());
    } catch (...) {
        puts("Could not get test credentials from credentials.json");
        throw;
    }
}

#endif // TEST_CONFIG_H__INCLUDED
