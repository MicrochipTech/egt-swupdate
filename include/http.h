#ifndef __HTTP_H__
#define __HTTP_H__

#include <string>
#include <curl/curl.h>

typedef struct writeData {
        const char *pData;
        size_t remaining;
} writeData;

class HTTP {
public:
        HTTP();
        ~HTTP() noexcept;

        std::string get(const std::string& uri, const std::string& sslkey, const std::string& sslcert);
        bool post(const std::string& uri, const std::string& sslkey, const std::string& sslcert, const char *data, ssize_t size);
        bool put(const std::string& uri, const std::string& sslkey, const std::string& sslcert, const char *data, ssize_t size);

private:
         CURL *curl;
         void *putData;
         ssize_t putSize;
};

#endif