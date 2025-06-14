// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/**
 * @file restclient.cpp
 * @brief implementation of the restclient class
 * @author Daniel Schauenberg <d@unwiredcouch.com>
 */

/*========================
         INCLUDES
  ========================*/
#include "restclient.h"

#include "strutils.h"

#include <cstring>
#include <map>
#include <string>

/** initialize user agent string */
const char *RestClient::user_agent = "ed_highway";
/** initialize authentication variable */
std::string RestClient::user_pass = std::string();
std::string RestClient::curl_interface = std::string();

static void hexchar(unsigned char c, unsigned char &hex1, unsigned char &hex2)
{
    hex1 = c / 16;
    hex2 = c % 16;
    hex1 += hex1 <= 9 ? '0' : 'a' - 10;
    hex2 += hex2 <= 9 ? '0' : 'a' - 10;
}

std::string RestClient::urlencode(const std::string &s)
{
    std::vector<char> v;
    v.reserve(s.size());
    for (const char c : s)
    {
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '-'
            || c == '_' || c == '.' || c == '!' || c == '~' || c == '*' || c == '\'' || c == '('
            || c == ')')
            v.push_back(c);
        else if (c == ' ')
            v.push_back('+');
        else
        {
            v.push_back('%');
            unsigned char d1, d2;
            hexchar(c, d1, d2);
            v.push_back(d1);
            v.push_back(d2);
        }
    }

    return std::string(v.cbegin(), v.cend());
}

std::string RestClient::encodePOSTParameters(const RestClient::parameters &params)
{
    std::vector<std::string> tmp;
    tmp.reserve(params.size());
    for (const auto &kv : params)
    {
        const std::string k{urlencode(kv.first)};
        const std::string v{urlencode(kv.second)};
        tmp.push_back(k + "=" + v);
    }
    return utility::join(tmp, "&");
}

/** Authentication Methods implementation */
void RestClient::clearAuth()
{
    RestClient::user_pass.clear();
}

void RestClient::setAuth(const std::string &user, const std::string &password)
{
    RestClient::user_pass.clear();
    RestClient::user_pass += user + ":" + password;
}
/**
 * @brief HTTP GET method
 *
 * @param url to query
 *
 * @return response struct
 */
RestClient::response RestClient::get(const std::string &url, const size_t timeout)
{
    headermap emptyMap;
    return RestClient::get(url, emptyMap, timeout);
}

/**
 * @brief HTTP GET method
 *
 * @param url to query
 * @param headers HTTP headers
 *
 * @return response struct
 */
RestClient::response RestClient::get(const std::string &url, const headermap &headers,
                                     const size_t timeout)
{
    return dosomething(
             url,
             [](CURL *curl) -> void {
                 (void)curl;
                 curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
             },
             headers, timeout)
      .setMethod("GET");
}
/**
 * @brief HTTP POST method
 *
 * @param url to query
 * @param ctype content type as string
 * @param data HTTP POST body
 *
 * @return response struct
 */
RestClient::response RestClient::post(const std::string &url, const std::string &data,
                                      const size_t timeout)
{
    const static headermap emptyMap;
    return RestClient::post(url, data, emptyMap, timeout);
}

/**
 * @brief HTTP POST method
 *
 * @param url to query
 * @param ctype content type as string
 * @param data HTTP POST body
 * @param headers HTTP headers
 *
 * @return response struct
 */
RestClient::response RestClient::post(const std::string &url, const std::string &data,
                                      const headermap &headers, const size_t timeout,
                                      const RestClient::builder &func)
{
    return dosomething(
             url,
             [&data, &func](CURL *curl) -> void {
                 // curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
                 /** set post fields */
                 curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
                 curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE,
                                  data.size()); // allowing more then 2Gb by _LARGE
                 func(curl);
             },
             headers, timeout)
      .setMethod("POST");
}

RestClient::response RestClient::patch(const std::string &url, const std::string &data,
                                       const size_t timeout)
{
    const static headermap emptyMap;
    return RestClient::patch(url, data, emptyMap, timeout);
}

RestClient::response RestClient::patch(const std::string &url, const std::string &data,
                                       const RestClient::headermap &headers, const size_t timeout,
                                       const RestClient::builder &func)
{
    return dosomething(
             url,
             [&data, &func](CURL *curl) -> void {
                 curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
                 /** set post fields */
                 curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
                 curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE,
                                  data.size()); // allowing more then 2Gb by _LARGE
                 func(curl);
             },
             headers, timeout)
      .setMethod("PATCH");
}

RestClient::response RestClient::del_put(const std::string &url, const std::string &data,
                                         const size_t timeout)
{
    const static headermap emptyMap;
    return RestClient::del_put(url, data, emptyMap, timeout);
}

RestClient::response RestClient::del_put(const std::string &url, const std::string &data,
                                         const RestClient::headermap &headers, const size_t timeout,
                                         const RestClient::builder &func)
{
    return dosomething(
             url,
             [&data, &func](CURL *curl) -> void {
                 curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
                 /** set post fields */
                 curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
                 curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE_LARGE,
                                  data.size()); // allowing more then 2Gb by _LARGE
                 func(curl);
             },
             headers, timeout)
      .setMethod("DELETE");
}

/**
 * @brief HTTP PUT method
 *
 * @param url to query
 * @param ctype content type as string
 * @param data HTTP PUT body
 *
 * @return response struct
 */
RestClient::response RestClient::put(const std::string &url, const std::string &data,
                                     const size_t timeout)
{
    headermap emptyMap;
    return RestClient::put(url, data, emptyMap, timeout);
}

/**
 * @brief HTTP PUT method
 *
 * @param url to query
 * @param ctype content type as string
 * @param data HTTP PUT body
 * @param headers HTTP headers
 *
 * @return response struct
 */
RestClient::response RestClient::put(const std::string &url, const std::string &data,
                                     const headermap &headers, const size_t timeout,
                                     const builder &func)
{
    /** initialize upload object */
    RestClient::upload_object up_obj;
    up_obj.data = data.c_str();
    up_obj.length = data.size();

    return dosomething(
             url,
             [&up_obj, &func](CURL *curl) -> void {
                 /** Now specify we want to PUT data */
                 curl_easy_setopt(curl, CURLOPT_PUT, 1L);
                 curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
                 /** set read callback function */
                 curl_easy_setopt(curl, CURLOPT_READFUNCTION, RestClient::read_callback);
                 /** set data object to pass to callback function */
                 curl_easy_setopt(curl, CURLOPT_READDATA, &up_obj);
                 /** set data size */
                 curl_easy_setopt(curl, CURLOPT_INFILESIZE, static_cast<long>(up_obj.length));
                 func(curl);
             },
             headers, timeout)
      .setMethod("PUT");
}

/**
 * @brief HTTP DELETE method
 *
 * @param url to query
 *
 * @return response struct
 */
RestClient::response RestClient::del(const std::string &url, const size_t timeout)
{
    headermap emptyMap;
    return RestClient::del(url, emptyMap, timeout);
}

/**
 * @brief HTTP DELETE method
 *
 * @param url to query
 * @param headers HTTP headers
 *
 * @return response struct
 */
RestClient::response RestClient::del(const std::string &url, const headermap &headers,
                                     const size_t timeout)
{
    return custom_method_get_family("DELETE", url, headers, timeout);
}

RestClient::response RestClient::options(const std::string &url,
                                         const RestClient::headermap &headers, const size_t timeout)
{
    return custom_method_get_family("OPTIONS", url, headers, timeout);
}

RestClient::response RestClient::custom_method_get_family(const std::string &method,
                                                          const std::string &url,
                                                          const RestClient::headermap &headers,
                                                          const size_t timeout, bool keepalive)
{
    return dosomething(
             url,
             [&method, keepalive](CURL *curl) -> void {
                 curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method.c_str());
                 if (keepalive)
                 {
                     /* enable TCP keep-alive for this transfer */
                     curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

                     /* keep-alive idle time to 120 seconds */
                     curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);

                     /* interval time between keep-alive probes: 60 seconds */
                     curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
                 }
                 else
                     curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 0L);
             },
             headers, timeout)
      .setMethod(method);
}

/**
 * @brief write callback function for libcurl
 *
 * @param data returned data of size (size*nmemb)
 * @param size size parameter
 * @param nmemb memblock parameter
 * @param userdata pointer to user data to save/work with return data
 *
 * @return (size * nmemb)
 */
size_t RestClient::write_callback(void *data, size_t size, size_t nmemb, void *userdata)
{
    RestClient::response *r;
    r = reinterpret_cast<RestClient::response *>(userdata);
    r->body.append(reinterpret_cast<char *>(data), size * nmemb);

    return (size * nmemb);
}

/**
 * @brief header callback for libcurl
 *
 * @param data returned (header line)
 * @param size of data
 * @param nmemb memblock
 * @param userdata pointer to user data object to save headr data
 * @return size * nmemb;
 */
size_t RestClient::header_callback(void *data, size_t size, size_t nmemb, void *userdata)
{
    RestClient::response *r;
    r = reinterpret_cast<RestClient::response *>(userdata);
    std::string header(reinterpret_cast<char *>(data), size * nmemb);
    size_t seperator = header.find_first_of(":");
    if (std::string::npos == seperator)
    {
        // roll with non seperated headers...
        trim(header);
        if (0 == header.length())
        {
            return (size * nmemb); // blank line;
        }
        r->headers[header] = "present";
    }
    else
    {
        std::string key = header.substr(0, seperator);
        trim(key);
        std::string value = header.substr(seperator + 1);
        trim(value);
        if (utility::toLower(key) == "set-cookie" && r->headers.count(key))
            r->headers[key] += "; " + value;
        else
            r->headers[key] = value;
    }

    return (size * nmemb);
}

/**
 * @brief read callback function for libcurl
 *
 * @param pointer of max size (size*nmemb) to write data to
 * @param size size parameter
 * @param nmemb memblock parameter
 * @param userdata pointer to user data to read data from
 *
 * @return (size * nmemb)
 */
size_t RestClient::read_callback(void *data, size_t size, size_t nmemb, void *userdata)
{
    /** get upload struct */
    RestClient::upload_object *u;
    u = reinterpret_cast<RestClient::upload_object *>(userdata);
    /** set correct sizes */
    size_t curl_size = size * nmemb;
    size_t copy_size = (u->length < curl_size) ? u->length : curl_size;
    /** copy data to buffer */
    memcpy(data, u->data, copy_size);
    /** decrement length and increment data pointer */
    u->length -= copy_size;
    u->data += copy_size;
    /** return copied size */
    return copy_size;
}

const char *RestClient::getUser_agent()
{
    return user_agent;
}

RestClient::response RestClient::dosomething(const std::string &url,
                                             const RestClient::builder &func,
                                             const RestClient::headermap &headers,
                                             const size_t timeout)
{
    /** create return struct */
    RestClient::response ret = {};
    /** build content-type header string */
    std::string header;

    // use libcurl
    CURL *curl = nullptr;
    CURLcode res = CURLE_OK;

    ret.curlError.resize(CURL_ERROR_SIZE + 1, 0);
    curl = curl_easy_init();
    if (curl)
    {
        applyCommonOptions(curl);
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, ret.curlError.c_str());

        // chrome shows failed in 20s
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT,
                         std::min(static_cast<size_t>(20), timeout + 1));
        /** set user agent */
        curl_easy_setopt(curl, CURLOPT_USERAGENT, RestClient::user_agent);
        /** set query URL */
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        /** set callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RestClient::write_callback);

        /** set data object to pass to callback function */
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);
        /** set the header callback function */
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, RestClient::header_callback);
        /** callback object for headers */
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ret);
        /** set content-type header */
        curl_slist *hlist = nullptr;
        // hlist = curl_slist_append(hlist, ctype_header.c_str());
        for (headermap::const_iterator it = headers.begin(); it != headers.end(); ++it)
        {
            header = it->first;
            header += ": ";
            header += it->second;
            hlist = curl_slist_append(hlist, header.c_str());
        }

        func(curl); // do anything specifiec
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, hlist);
        curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, ""); // all built-in encodings

        // set timeout
        if (timeout)
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1); // dont want to get a sig alarm on timeout

        /** perform the actual query */
        res = curl_easy_perform(curl);
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        // std::string custom_redirect;
        ret.curlCode = res;
        bool success = (res == CURLE_OK);
        if (success)
            ret.code = static_cast<int>(http_code);
        else
        {
            ret.code = res;
            ret.body = "CURL FAILED, code is curl-fail-code.";
        }
        curl_slist_free_all(hlist);
        curl_easy_cleanup(curl);

        // error buffer must be freed ONLY after curl_easy_cleanup
        if (success)
            ret.curlError.clear();
        else
        {
            std::string tmp(ret.curlError.c_str());
            ret.curlError = tmp;
        }
    }
    return ret;
}

void RestClient::setCURLInterface(const std::string &val)
{
    curl_interface = val;
}

void RestClient::applyCommonOptions(CURL *curl)
{
    if (!curl_interface.empty())
        curl_easy_setopt(curl, CURLOPT_INTERFACE, curl_interface.c_str());
    else
        curl_easy_setopt(curl, CURLOPT_INTERFACE, nullptr);

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

    // #ifdef _DEBUG
    //     curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    // #endif
}
