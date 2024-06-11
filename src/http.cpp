#include <sstream>
#include <iostream>
#include <string>
#include <algorithm>
#include <cctype>
#include <cwctype>
#include <cstring>
#include "http.h"

using namespace std;

int strncasecmp(std::string s1, std::string s2) {
        transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
        transform(s2.begin(), s2.end(), s2.begin(), ::tolower);

        if (s1.compare(s2) == 0) {
                return 1;
        }
        return 0;
}

int strncasecmp(std::string s1, std::string s2, ssize_t len) {
        transform(s1.begin(), s1.end(), s1.begin(), ::tolower);
        transform(s2.begin(), s2.end(), s2.begin(), ::tolower);

        if (s1.compare(0, len, s2) == 0) {
                return 1;
        }
        return 0;
}

size_t writeCb(void *buffer, size_t size, size_t nmemb, void *userptr) {
        string data((const char*)buffer, (size_t) size * nmemb);

        *((stringstream*) userptr) << data << endl;

        return size * nmemb;
}

HTTP::HTTP() {
        curl = curl_easy_init();
}

HTTP::~HTTP() {
        curl_easy_cleanup(curl);
}

std::string HTTP::get(const std::string& uri, const std::string& sslkey, const std::string& sslcert) {
        std::stringstream response;

        curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        if ((sslkey.empty() != true) && (sslcert.empty() != true)) {
                curl_easy_setopt(curl, CURLOPT_SSLKEY, sslkey.c_str());
                curl_easy_setopt(curl, CURLOPT_SSLCERT, sslcert.c_str());

                if ((strncasecmp(sslkey, "pkcs11:", 7) == 1) || (strncasecmp(sslcert, "pkcs11:", 7) == 1)) {
                        curl_easy_setopt(curl, CURLOPT_SSLENGINE, "pkcs11");
                }

                if (strncasecmp(sslkey, "pkcs11:", 7) == 1) {
                        curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "ENG");
                }
                if (strncasecmp(sslcert, "pkcs11:", 7) == 1) {
                        curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "ENG");
                }

                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        }

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
                cout << "Error, curl_easy_perform: " << curl_easy_strerror(res) << endl;
        }

        return response.str();
}

bool HTTP::post(const std::string& uri, const std::string& sslkey, const std::string& sslcert, const char *data, ssize_t size) {
        writeData send;

        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: application/json");

        send.pData = data;
        send.remaining = size;

        curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        if ((sslkey.empty() != true) && (sslcert.empty() != true)) {
                curl_easy_setopt(curl, CURLOPT_SSLKEY, sslkey.c_str());
                curl_easy_setopt(curl, CURLOPT_SSLCERT, sslcert.c_str());

                if ((strncasecmp(sslkey, "pkcs11:", 7) == 1) || (strncasecmp(sslcert, "pkcs11:", 7) == 1)) {
                        curl_easy_setopt(curl, CURLOPT_SSLENGINE, "pkcs11");
                }

                if (strncasecmp(sslkey, "pkcs11:", 7) == 1) {
                        curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "ENG");
                }
                if (strncasecmp(sslcert, "pkcs11:", 7) == 1) {
                        curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "ENG");
                }

                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        }

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
                cout << "Error, curl_easy_perform: " << curl_easy_strerror(res) << endl;
                return false;
        }

        curl_slist_free_all(list);

        return true;
}

bool HTTP::put(const std::string& uri, const std::string& sslkey, const std::string& sslcert, const char *data, ssize_t size) {
        struct curl_slist *list = NULL;
        list = curl_slist_append(list, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, uri.c_str());
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);

        if ((sslkey.empty() != true) && (sslcert.empty() != true)) {
                curl_easy_setopt(curl, CURLOPT_SSLKEY, sslkey.c_str());
                curl_easy_setopt(curl, CURLOPT_SSLCERT, sslcert.c_str());

                if ((strncasecmp(sslkey, "pkcs11:", 7) == 1) || (strncasecmp(sslcert, "pkcs11:", 7) == 1)) {
                        curl_easy_setopt(curl, CURLOPT_SSLENGINE, "pkcs11");
                }

                if (strncasecmp(sslkey, "pkcs11:", 7) == 1) {
                        curl_easy_setopt(curl, CURLOPT_SSLKEYTYPE, "ENG");
                }
                if (strncasecmp(sslcert, "pkcs11:", 7) == 1) {
                        curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "ENG");
                }

                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
                curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        }

        CURLcode res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
                cout << "Error, curl_easy_perform: " << curl_easy_strerror(res) << endl;
                return false;
        }

        curl_slist_free_all(list);

        return true;
}
