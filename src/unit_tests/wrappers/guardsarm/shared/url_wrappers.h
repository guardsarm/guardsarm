/* Copyright (C) 2026 GuardSarm, Inc.
 * All rights reserved.
 *
 * Proprietary and confidential property of GuardSarm, Inc. Unauthorized copying, distribution, modification, or use is prohibited except under a written license agreement with GuardSarm, Inc.
 */

#ifndef URL_WRAPPERS_H
#define URL_WRAPPERS_H

#include "shared.h"
#include "url.h"

int __wrap_wurl_request(const char* url, const char* dest, const char* header, const char* data, const long timeout);

char* __wrap_wurl_http_get(const char* url, size_t max_size, long timeout);

curl_response* __wrap_wurl_http_request(char* method,
                                        char** headers,
                                        const char* url,
                                        const char* payload,
                                        size_t max_size,
                                        long timeout,
                                        const char* userpass,
                                        bool ssl_verify);

void expect_wrap_wurl_http_request(char* method,
                                   char** headers,
                                   const char* url,
                                   const char* payload,
                                   size_t max_size,
                                   long timeout,
                                   const char* userpass,
                                   bool ssl_verify,
                                   curl_response* response);

CURL* __wrap_curl_easy_init();

void __wrap_curl_easy_cleanup(CURL* curl);

CURLcode __wrap_curl_easy_setopt(CURL* curl, CURLoption option, void* parameter);

struct curl_slist* __wrap_curl_slist_append(struct curl_slist* list, const char* string);

CURLcode __wrap_curl_easy_perform(CURL* curl);

void __wrap_curl_slist_free_all(struct curl_slist* list);

CURLcode __wrap_curl_easy_getinfo(CURL* curl, CURLoption option, void* parameter);

void __wrap_wurl_free_response(curl_response* response);

#endif
