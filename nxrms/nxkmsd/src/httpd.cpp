
#include <Windows.h>
#include <assert.h>

#include <iostream>


#include <nudf\string.hpp>

#include "nxkmsd.hpp"
#include "httpd.hpp"


using namespace web::http::experimental::listener;
using namespace web::http;
using namespace web;


rms_basic_httpd::rms_basic_httpd()
{
}

rms_basic_httpd::~rms_basic_httpd()
{
    stop();
}

void rms_basic_httpd::handle_get(web::http::http_request message)
{
    const web::uri& request_uri = message.request_uri();
    const std::wstring& request_path = request_uri.path();

    message.reply(status_codes::NotFound);
}

void rms_basic_httpd::handle_post(web::http::http_request message)
{
    message.reply(status_codes::NotImplemented);
}

void rms_basic_httpd::handle_put(web::http::http_request message)
{
    message.reply(status_codes::NotImplemented);
}

void rms_basic_httpd::handle_delete(web::http::http_request message)
{
    message.reply(status_codes::NotImplemented);
}

void rms_basic_httpd::start(const std::wstring& server_url)
{
    web::http::uri_builder uri(server_url);

    _listener = std::shared_ptr<http_listener>(new http_listener(uri.to_uri()));
    _listener->support(methods::GET, std::bind(&rms_basic_httpd::handle_get, this, std::placeholders::_1));
    _listener->support(methods::POST, std::bind(&rms_basic_httpd::handle_post, this, std::placeholders::_1));
    _listener->support(methods::PUT, std::bind(&rms_basic_httpd::handle_put, this, std::placeholders::_1));
    _listener->support(methods::DEL, std::bind(&rms_basic_httpd::handle_delete, this, std::placeholders::_1));

    try {
        _listener->open().then([server_url]() {std::wcout << L"\nstarting to listen to: " << server_url << L"\n" << std::endl; }).wait();
    }
    catch (std::exception const & e) {
        std::cout << e.what() << std::endl;
    }
    catch (...) {
        std::cout << "Unknown exception" << std::endl;
    }
}

void rms_basic_httpd::stop() noexcept
{
    if (_listener != nullptr) {
        _listener->close();
    }
}


rms_httpd::rms_httpd() : rms_basic_httpd()
{
}

rms_httpd::~rms_httpd()
{
}

void rms_httpd::handle_get(web::http::http_request message)
{
    message.reply(status_codes::OK, U("Hello, World!"));
}

void rms_httpd::handle_post(web::http::http_request message)
{
    rms_basic_httpd::handle_post(message);
}

void rms_httpd::handle_put(web::http::http_request message)
{
    rms_basic_httpd::handle_put(message);
}

void rms_httpd::handle_delete(web::http::http_request message)
{
    rms_basic_httpd::handle_delete(message);
}




rms_control_httpd::rms_control_httpd() : rms_basic_httpd()
{
}

rms_control_httpd::~rms_control_httpd()
{
}

void rms_control_httpd::handle_get(web::http::http_request message)
{
    const web::uri& request_uri = message.request_uri();
    const std::wstring& request_path = request_uri.path();

    message.reply(status_codes::OK, U("Control page!"));
}

void rms_control_httpd::handle_post(web::http::http_request message)
{
    rms_basic_httpd::handle_post(message);
}

void rms_control_httpd::handle_put(web::http::http_request message)
{
    rms_basic_httpd::handle_put(message);
}

void rms_control_httpd::handle_delete(web::http::http_request message)
{
    rms_basic_httpd::handle_delete(message);
}
