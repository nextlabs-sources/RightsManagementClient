
#include <Windows.h>

#include <nudf\web\http_msg.hpp>
#include <nudf\web\http_client.hpp>
#include <nudf\web\http_constant.hpp>
#include <nudf\web\producerconsumerstream.hpp>
#include <nudf\web\http_helpers.hpp>

namespace NX {
namespace web {
namespace http
{

uri details::_http_request::relative_uri() const
{
    // If the listener path is empty, then just return the request URI.
    if (m_listener_path.empty() || m_listener_path == L"/")
    {
        return m_uri.resource();
    }

    std::wstring prefix = uri::decode(m_listener_path);
    std::wstring path = uri::decode(m_uri.resource().to_string());
    if (path.empty())
    {
        path = L"/";
    }

    auto pos = path.find(prefix);
    if (pos == 0)
    {
        return uri(uri::encode_uri(path.erase(0, prefix.length())));
    }
    else
    {
        throw http_exception(L"Error: request was not prefixed with listener uri");
    }
}

uri details::_http_request::absolute_uri() const
{
    if (m_base_uri.is_empty())
    {
        return m_uri;
    }
    else
    {
        return uri_builder(m_base_uri).append(m_uri).to_uri();
    }
}

void details::_http_request::set_request_uri(const uri& relative)
{
    m_uri = relative;
}

std::wstring details::_http_request::to_string() const
{
    std::wostringstream buffer;
    buffer << m_method << L" " << (this->m_uri.is_empty() ? L"/" : this->m_uri.to_string()) << L" HTTP/1.1\r\n";
    buffer << http_msg_base::to_string();
    return buffer.str();
}

std::wstring details::_http_response::to_string() const
{
    // If the user didn't explicitly set a reason phrase then we should have it default
    // if they used one of the standard known status codes.
    auto reason_phrase = m_reason_phrase;
    if (reason_phrase.empty())
    {
        static http_status_to_phrase idToPhraseMap[] = {
            {status_codes::Continue,              L"Continue"},
            {status_codes::SwitchingProtocols,    L"Switching Protocols"},
            {status_codes::OK,                    L"OK"},
            {status_codes::Created,               L"Created"},
            {status_codes::Accepted,              L"Accepted"},
            {status_codes::NonAuthInfo,           L"Non-Authoritative Information" },
            {status_codes::NoContent,             L"No Content" },
            {status_codes::ResetContent,          L"Reset Content" },
            {status_codes::PartialContent,        L"Partial Content" },
            {status_codes::MultipleChoices,       L"Multiple Choices" },
            {status_codes::MovedPermanently,      L"Moved Permanently" },
            {status_codes::Found,                 L"Found" },
            {status_codes::SeeOther,              L"See Other" },
            {status_codes::NotModified,           L"Not Modified" },
            {status_codes::UseProxy,              L"Use Proxy" },
            {status_codes::TemporaryRedirect,     L"Temporary Redirect" },
            {status_codes::BadRequest,            L"Bad Request"},
            {status_codes::Unauthorized,          L"Unauthorized"},
            {status_codes::PaymentRequired,       L"Payment Required"},
            {status_codes::Forbidden,             L"Forbidden"},
            {status_codes::NotFound,              L"Not Found"},
            {status_codes::MethodNotAllowed,      L"Method Not Allowed"},
            {status_codes::NotAcceptable,         L"Not Acceptable"},
            {status_codes::ProxyAuthRequired,     L"Proxy Authentication Required"},
            {status_codes::RequestTimeout,        L"Request Time-out"},
            {status_codes::Conflict,              L"Conflict"},
            {status_codes::Gone,                  L"Gone"},
            {status_codes::LengthRequired,        L"Length Required"},
            {status_codes::PreconditionFailed,    L"Precondition Failed"},
            {status_codes::RequestEntityTooLarge, L"Request Entity Too Large"},
            {status_codes::RequestUriTooLarge,    L"Request Uri Too Large"},
            {status_codes::UnsupportedMediaType,  L"Unsupported Media Type"},
            {status_codes::RangeNotSatisfiable,   L"Requested range not satisfiable"},
            {status_codes::ExpectationFailed,     L"Expectation Failed"},
            {status_codes::InternalError,         L"Internal Error"},
            {status_codes::NotImplemented,        L"Not Implemented"},
            {status_codes::BadGateway,            L"Bad Gateway"},
            {status_codes::ServiceUnavailable,    L"Service Unavailable"},
            {status_codes::GatewayTimeout,        L"Gateway Time-out"},
            {status_codes::HttpVersionNotSupported, L"HTTP Version not supported"}
        };

        for (auto iter = std::begin(idToPhraseMap); iter != std::end(idToPhraseMap); ++iter)
        {
            if (iter->id == status_code())
            {
                reason_phrase = iter->phrase;
                break;
            }
        }
    }

    std::wostringstream buffer;
    buffer << L"HTTP/1.1 " << m_status_code << L" " << reason_phrase << L"\r\n";

    buffer << http_msg_base::to_string();
    return buffer.str();
}

// Macros to help build string at compile time and avoid overhead.
#define STRINGIFY(x) _XPLATSTR(#x)
#define TOSTRING(x) STRINGIFY(x)
#define USERAGENT L"nxrmc/8.1.0"

pplx::task<http_response> client::http_client::request(http_request request, const pplx::cancellation_token &token)
{
    if (!request.headers().has(header_names::user_agent))
    {
        request.headers().add(header_names::user_agent, USERAGENT);
    }

    request._set_base_uri(base_uri());
    request._set_cancellation_token(token);
    return m_pipeline->propagate(request);
}

}
}
} // namespace NX::web::http
