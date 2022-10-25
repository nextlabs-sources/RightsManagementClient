
#pragma once
#ifndef __NXRMS_HTTPD_HPP__
#define __NXRMS_HTTPD_HPP__

#include <memory>

#include <cpprest\http_listener.h>


class rms_basic_httpd
{
public:
    rms_basic_httpd();
    virtual ~rms_basic_httpd();

    void start(const std::wstring& server_url);
    void stop() noexcept;

protected:
    virtual void handle_get(web::http::http_request message);
    virtual void handle_post(web::http::http_request message);
    virtual void handle_put(web::http::http_request message);
    virtual void handle_delete(web::http::http_request message);

private:
    std::shared_ptr<web::http::experimental::listener::http_listener>  _listener;
};


class rms_httpd : public rms_basic_httpd
{
public:
    rms_httpd();
    virtual ~rms_httpd();

protected:
    virtual void handle_get(web::http::http_request message);
    virtual void handle_post(web::http::http_request message);
    virtual void handle_put(web::http::http_request message);
    virtual void handle_delete(web::http::http_request message);
};


class rms_control_httpd : public rms_basic_httpd
{
public:
    rms_control_httpd();
    virtual ~rms_control_httpd();

protected:
    virtual void handle_get(web::http::http_request message);
    virtual void handle_post(web::http::http_request message);
    virtual void handle_put(web::http::http_request message);
    virtual void handle_delete(web::http::http_request message);
};


#endif