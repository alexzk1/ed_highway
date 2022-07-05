/**
 * @file restclient.h
 * @brief libcurl wrapper for REST calls
 * @author Daniel Schauenberg <d@unwiredcouch.com>
 * @version
 * @date 2010-10-11
 */

#pragma once

#include <curl/curl.h>
#include <string>
#include <map>
#include <cstdlib>
#include <algorithm>
#include <functional>

#define CURL_CUST_TIMEOUT_MESSAGE "CURL: Operation Timeouted"

class RestClient
{
public:
    /**
     * public data definitions
     */
    using headermap  = std::map<std::string, std::string>;
    using parameters = std::map<std::string, std::string>;
    using builder   = std::function<void(CURL *curl)>;

    /** response struct for queries */
    struct response
    {
        int code;
        int curlCode;
        std::string httpmethod;
        std::string body;
        std::string curlError;
        headermap headers;
        response& setMethod(const std::string& m)
        {
            httpmethod = m;
            return *this;
        }
    };
    /** struct used for uploading data */
    typedef struct
    {
        const char* data;
        size_t length;
    } upload_object;

    /** public methods */
    static std::string urlencode(const std::string& s);
    static std::string encodePOSTParameters(const parameters& params);

    // Auth
    void clearAuth();
    void setAuth(const std::string& user, const std::string& password);
    // HTTP GET
    static response get(const std::string& url, const size_t timeout = 0);
    static response get(const std::string& url, const headermap& headers,
                        const size_t timeout = 0);
    // HTTP POST
    static response post(const std::string& url, const std::string& data, const size_t timeout = 0);
    static response post(const std::string& url, const std::string& data, const headermap& headers,
                         const size_t timeout = 0, const builder &func = [](CURL *curl)
    {
        (void)curl;
    });

    //HTTP PATCH

    static response patch(const std::string& url, const std::string& data, const size_t timeout = 0);
    static response patch(const std::string& url, const std::string& data, const headermap& headers,
                          const size_t timeout = 0, const builder &func = [](CURL *curl)
    {
        (void)curl;
    });


    static response del_put(const std::string& url, const std::string& data, const size_t timeout = 0);
    static response del_put(const std::string& url, const std::string& data, const headermap& headers,
                            const size_t timeout = 0, const builder &func = [](CURL *curl)
    {
        (void)curl;
    });

    // HTTP PUT
    static response put(const std::string& url,
                        const std::string& data, const size_t timeout = 0);
    static response put(const std::string& url,
                        const std::string& data, const headermap& headers,
                        const size_t timeout = 0, const builder &func = [](CURL *curl)
    {
        (void)curl;
    });

    static response dosomething(const std::string& url, const builder& func, const headermap& headers, const size_t timeout = 0);

    // HTTP DELETE
    static response del(const std::string& url, const size_t timeout = 0);
    static response del(const std::string& url, const headermap& headers,
                        const size_t timeout = 0);

    //options
    static response options(const std::string& url, const headermap& headers,
                            const size_t timeout = 0);


    static response custom_method_get_family(const std::string& method, const std::string& url, const headermap& headers,
            const size_t timeout = 0, bool keepalive = false);
    static const char *getUser_agent();
    static  void setCURLInterface(const std::string& val);

    static void applyCommonOptions(CURL *curl);
private:
    // writedata callback function
    static size_t write_callback(void *ptr, size_t size, size_t nmemb,
                                 void *userdata);

    // header callback function
    static size_t header_callback(void *ptr, size_t size, size_t nmemb,
                                  void *userdata);
    // read callback function
    static size_t read_callback(void *ptr, size_t size, size_t nmemb,
                                void *userdata);
    static const char* user_agent;
    static std::string user_pass;
    static std::string curl_interface;

    // trim from start
    static inline std::string &ltrim(std::string &s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](auto c)
        {
            return !std::isspace(c);
        }));
        return s;
    }

    // trim from end
    static inline std::string &rtrim(std::string &s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](auto c)
        {
            return !std::isspace(c);
        }).base(), s.end());
        return s;
    }

    // trim from both ends
    static inline std::string &trim(std::string &s)
    {
        return ltrim(rtrim(s));
    }

};

