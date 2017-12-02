#include "api.h"
#include "utils/restapi.h"
#include "utils/u_json.hpp"
#include "utils/hex.hpp"
#include <iostream>
#include <array>
#include <algorithm>
#include <fstream>
#include <cctype>
#include <cstdlib>
#include <iomanip>

namespace Api {
    static std::ofstream logFile("log.txt", std::ofstream::trunc);
    static std::string cert = "curl-ca-bundle.crt";

    static std::string api = "";
    static std::string secret = "";

    static json_t *authRequest(std::string);

    static std::string getSignature(std::string payload) {
        uint8_t *hmac_digest = HMAC(EVP_sha256(),secret.c_str(), secret.size(), reinterpret_cast<const uint8_t *>(payload.data()), payload.size(),NULL, NULL);
        std::string header = hex_str(hmac_digest, hmac_digest + SHA256_DIGEST_LENGTH);
        return header;
    } // getSignature()

    double getAvail(std::string currency) {
        u_json root{authRequest("/api/v3/account")};
        json_t *obj= json_object_get(root.get(), "balances");
        size_t arrayLen = json_array_size(obj);
        double avail = 0.0;
        obj= json_object_get(json_array_get(balances, i), "free");
        const char *currstr = json_string_value(obj);
        obj= json_object_get(root.get(), "balances");
        for (size_t i = 0; i < arrayLen; i++) {
            std::string tmp = json_string_value(json_object_get(json_array_get(obj, i), "asset"));
            if(tmp == currency) avail = atof(currstr);
        } // for
        return avail;
    } // getAvail()

    json_t *authRequest(std::string request) {
        RestApi &exchange = query("https://api.binance.com", cert.c_str());
        u_json root{exchange.getRequest("/api/v1/time")};
        json_t *obj= json_object_get(root.get(), "serverTime");
        long json_timestamp = json_integer_value(obj);
        std::string timestamp = std::to_string(json_timestamp);
        std::array<std::string, 1> headers{ "X-MBX-APIKEY:" + api };

        std::string payload += "timestamp=" + timestamp;
        std::string signature += getSignature(payload);
        std::string url += request + "?timestamp=" + timestamp + "&signature=" + signature;
        return exchange.getRequest(url, make_slist(std::begin(headers), std::end(headers)));
    } // authRequest()


}
