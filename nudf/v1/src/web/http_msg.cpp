
#include <Windows.h>

#include <nudf\web\http_msg.hpp>
#include <nudf\web\http_constant.hpp>
#include <nudf\web\producerconsumerstream.hpp>
#include <nudf\web\http_helpers.hpp>

using namespace NX::web;
using namespace NX::utility;
using namespace concurrency;
using namespace NX::utility::conversions;
using namespace NX::web::http::details;

namespace NX {
namespace web {
namespace http
{

#define CRLF L"\r\n"

static const wchar_t * stream_was_set_explicitly = L"A stream was set on the message and extraction is not possible";
static const wchar_t * textual_content_type_missing = L"Content-Type must be textual to extract a string.";
static const wchar_t * unsupported_charset = L"Charset must be iso-8859-1, utf-8, utf-16, utf-16le, or utf-16be to be extracted.";

http_msg_base::http_msg_base()
    : m_headers(),
    m_default_outstream(false)
{
}

/// <summary>
/// Prepare the message with an output stream to receive network data
/// </summary>
void http_msg_base::_prepare_to_receive_data()
{
    // See if the user specified an outstream
    if (!outstream())
    {
        // The user did not specify an outstream.
        // We will create one...
        concurrency::streams::producer_consumer_buffer<uint8_t> buf;
        set_outstream(buf.create_ostream(), true);

        // Since we are creating the streambuffer, set the input stream
        // so that the user can retrieve the data.
        set_instream(buf.create_istream());
    }

    // If the user did specify an outstream we leave the instream
    // as invalid. It is assumed that user either has a read head
    // to the out streambuffer or the data is streamed into a container
    // or media (like file) that the user can read from...
}

/// <summary>
///     Determine the content length
/// </summary>
/// <returns>
///     size_t::max if there is content with unknown length (transfer_encoding:chunked)
///     0           if there is no content
///     length      if there is content with known length
/// </returns>
/// <remarks>
///     This routine should only be called after a msg (request/response) has been
///     completely constructed.
/// </remarks>
size_t http_msg_base::_get_content_length()
{
    // An invalid response_stream indicates that there is no body
    if ((bool)instream())
    {
        size_t content_length = 0;
        std::wstring transfer_encoding;

        bool has_cnt_length = headers().match(header_names::content_length, content_length);
        bool has_xfr_encode = headers().match(header_names::transfer_encoding, transfer_encoding);

        if (has_xfr_encode)
        {
#pragma push_macro ("max")
#undef max
            return std::numeric_limits<size_t>::max();
#pragma pop_macro ("max")
        }

        if (has_cnt_length)
        {
            return content_length;
        }

        // Neither is set. Assume transfer-encoding for now (until we have the ability to determine
        // the length of the stream).
        headers().add(header_names::transfer_encoding, L"chunked");
#pragma push_macro ("max")
#undef max
        return std::numeric_limits<size_t>::max();
#pragma pop_macro ("max")
    }

    return 0;
}

// Helper function to inline continuation if possible.
struct inline_continuation
{
    inline_continuation(pplx::task<void> &prev, const std::function<void(pplx::task<void>)> &next) : m_prev(prev), m_next(next) {}
    ~inline_continuation()
    {
        if (m_prev.is_done())
        {
            m_next(m_prev);
        }
        else
        {
            m_prev.then(m_next);
        }
    }
    pplx::task<void> & m_prev;
    std::function<void(pplx::task<void>)> m_next;
private:
    inline_continuation(const inline_continuation &);
    inline_continuation &operator=(const inline_continuation &);
};

void http_msg_base::_complete(unsigned __int64 body_size, const std::exception_ptr &exceptionPtr)
{
    const auto &completionEvent = _get_data_available();
    auto closeTask = pplx::task_from_result();

    if (exceptionPtr == std::exception_ptr())
    {
        if (m_default_outstream)
        {
            closeTask = outstream().close();
        }

        inline_continuation(closeTask, [completionEvent, body_size](pplx::task<void> t)
        {
            try
            {
                t.get();
                completionEvent.set(body_size);
            }
            catch (...)
            {
                // If close throws an exception report back to user.
                completionEvent.set_exception(std::current_exception());
                pplx::create_task(completionEvent).then([](pplx::task<unsigned __int64> t)
                {
                    try { t.get(); }
                    catch (...) {}
                });
            }
        });
    }
    else
    {
        if (outstream().is_valid())
        {
            closeTask = outstream().close(exceptionPtr);
        }

        inline_continuation(closeTask, [completionEvent, exceptionPtr](pplx::task<void> t)
        {
            // If closing stream throws an exception ignore since we already have an error.
            try { t.get(); }
            catch (...) {}
            completionEvent.set_exception(exceptionPtr);
            pplx::create_task(completionEvent).then([](pplx::task<unsigned __int64> t)
            {
                try { t.get(); }
                catch (...) {}
            });
        });
    }
}

std::wstring details::http_msg_base::_extract_string(bool force)
{
    std::wstring content, charset = charset_types::utf8;
    if (!force)
    {
        parse_content_type_and_charset(headers().content_type(), content, charset);

        // If no Content-Type then just return an empty string.
        if (content.empty())
        {
            return std::wstring();
        }

        // Content-Type must have textual type.
        if (!is_content_type_textual(content))
        {
            throw http_exception(textual_content_type_missing);
        }
    }

    if (!instream())
    {
        throw http_exception(stream_was_set_explicitly);
    }

    auto buf_r = instream().streambuf();

    if (buf_r.in_avail() == 0)
    {
        return std::wstring();
    }

    // Perform the correct character set conversion if one is necessary.
    if (utility::details::str_icmp(charset, charset_types::usascii)
        || utility::details::str_icmp(charset, charset_types::ascii))
    {
        std::string body;
        body.resize((std::string::size_type)buf_r.in_avail());
        buf_r.getn((uint8_t*)&body[0], body.size()).get(); // There is no risk of blocking.
        return NX::utility::conversions::to_string(std::move(body));
    }

    // Latin1
    if (utility::details::str_icmp(charset, charset_types::latin1))
    {
        std::string body;
        body.resize((std::string::size_type)buf_r.in_avail());
        buf_r.getn((uint8_t*)&body[0], body.size()).get(); // There is no risk of blocking.
                                                           // Could optimize for linux in the future if a latin1_to_utf8 function was written.
        return NX::utility::conversions::to_string(NX::utility::conversions::latin1_to_utf16(std::move(body)));
    }

    // utf-8.
    else if (utility::details::str_icmp(charset, charset_types::utf8))
    {
        std::string body;
        body.resize((std::string::size_type)buf_r.in_avail());
        buf_r.getn((uint8_t*)&body[0], body.size()).get(); // There is no risk of blocking.
        return NX::utility::conversions::to_string(std::move(body));
    }

    // utf-16.
    else if (utility::details::str_icmp(charset, charset_types::utf16))
    {
        std::wstring body;
        body.resize(buf_r.in_avail() / sizeof(std::wstring::value_type));
        buf_r.getn((uint8_t*)&body[0], body.size() * sizeof(std::wstring::value_type)); // There is no risk of blocking.
        return convert_utf16_to_string_t(std::move(body));
    }

    // utf-16le
    else if (utility::details::str_icmp(charset, charset_types::utf16le))
    {
        std::wstring body;
        body.resize(buf_r.in_avail() / sizeof(std::wstring::value_type));
        buf_r.getn((uint8_t*)&body[0], body.size() * sizeof(std::wstring::value_type)); // There is no risk of blocking.
        return convert_utf16le_to_string_t(std::move(body), false);
    }

    // utf-16be
    else if (utility::details::str_icmp(charset, charset_types::utf16be))
    {
        std::wstring body;
        body.resize(buf_r.in_avail() / sizeof(std::wstring::value_type));
        buf_r.getn((uint8_t*)&body[0], body.size() * sizeof(std::wstring::value_type)); // There is no risk of blocking.
        return convert_utf16be_to_string_t(std::move(body), false);
    }

    else
    {
        throw http_exception(unsupported_charset);
    }
}

json::value details::http_msg_base::_extract_json(bool force)
{
    std::wstring content, charset = charset_types::utf8;
    if (!force)
    {
        parse_content_type_and_charset(headers().content_type(), content, charset);

        // If no Content-Type then just return a null json value.
        if (content.empty())
        {
            return json::value();
        }

        // Content-Type must be "application/json" or one of the unofficial, but common JSON types.
        if (!is_content_type_json(content))
        {
            const std::wstring actualContentType = utility::conversions::to_string(content);
            throw http_exception((L"Content-Type must be JSON to extract (is: " + actualContentType + L")"));
        }
    }

    if (!instream())
    {
        throw http_exception(stream_was_set_explicitly);
    }

    auto buf_r = instream().streambuf();

    if (buf_r.in_avail() == 0)
    {
        return json::value::parse(std::wstring());
    }

    // Latin1
    if (utility::details::str_icmp(charset, charset_types::latin1))
    {
        std::string body;
        body.resize(buf_r.in_avail());
        buf_r.getn((uint8_t*)&body[0], body.size()).get(); // There is no risk of blocking.
                                                           // On Linux could optimize in the future if a latin1_to_utf8 function is written.
        return json::value::parse(NX::utility::conversions::to_string(NX::utility::conversions::latin1_to_utf16(std::move(body))));
    }

    // utf-8, usascii and ascii
    else if (utility::details::str_icmp(charset, charset_types::utf8)
        || utility::details::str_icmp(charset, charset_types::usascii)
        || utility::details::str_icmp(charset, charset_types::ascii))
    {
        std::string body;
        body.resize(buf_r.in_avail());
        buf_r.getn((uint8_t*)&body[0], body.size()).get(); // There is no risk of blocking.
        return json::value::parse(NX::utility::conversions::to_string(std::move(body)));
    }

    // utf-16.
    else if (utility::details::str_icmp(charset, charset_types::utf16))
    {
        std::wstring body;
        body.resize(buf_r.in_avail() / sizeof(std::wstring::value_type));
        buf_r.getn((uint8_t*)&body[0], body.size() * sizeof(std::wstring::value_type)); // There is no risk of blocking.
        return json::value::parse(convert_utf16_to_string_t(std::move(body)));
    }

    // utf-16le
    else if (utility::details::str_icmp(charset, charset_types::utf16le))
    {
        std::wstring body;
        body.resize(buf_r.in_avail() / sizeof(std::wstring::value_type));
        buf_r.getn((uint8_t*)&body[0], body.size() * sizeof(std::wstring::value_type)); // There is no risk of blocking.
        return json::value::parse(convert_utf16le_to_string_t(std::move(body), false));
    }

    // utf-16be
    else if (utility::details::str_icmp(charset, charset_types::utf16be))
    {
        std::wstring body;
        body.resize(buf_r.in_avail() / sizeof(std::wstring::value_type));
        buf_r.getn((uint8_t*)&body[0], body.size() * sizeof(std::wstring::value_type)); // There is no risk of blocking.
        return json::value::parse(convert_utf16be_to_string_t(std::move(body), false));
    }

    else
    {
        throw http_exception(unsupported_charset);
    }
}

std::vector<uint8_t> details::http_msg_base::_extract_vector()
{
    if (!instream())
    {
        throw http_exception(stream_was_set_explicitly);
    }

    std::vector<uint8_t> body;
    auto buf_r = instream().streambuf();
    const size_t size = buf_r.in_avail();

    if (size > 0)
    {
        body.resize(size);
        buf_r.getn((uint8_t*)&body[0], size).get(); // There is no risk of blocking.
    }

    return body;
}

// Helper function to convert message body without extracting.
static std::wstring convert_body_to_string_t(const std::wstring &content_type, concurrency::streams::istream instream)
{
    if (!instream)
    {
        // The instream is not yet set
        return std::wstring();
    }

    concurrency::streams::streambuf<uint8_t>  streambuf = instream.streambuf();

    _ASSERTE((bool)streambuf);
    _ASSERTE(streambuf.is_open());
    _ASSERTE(streambuf.can_read());

    std::wstring content, charset;
    parse_content_type_and_charset(content_type, content, charset);

    // Content-Type must have textual type.
    if (!is_content_type_textual(content) || streambuf.in_avail() == 0)
    {
        return std::wstring();
    }

    // Latin1
    if (utility::details::str_icmp(charset, charset_types::latin1))
    {
        std::string body;
        body.resize(streambuf.in_avail());
        if (streambuf.scopy((unsigned char *)&body[0], body.size()) == 0) return std::wstring();
        return to_string(latin1_to_utf16(std::move(body)));
    }

    // utf-8.
    else if (utility::details::str_icmp(charset, charset_types::utf8))
    {
        std::string body;
        body.resize(streambuf.in_avail());
        if (streambuf.scopy((unsigned char *)&body[0], body.size()) == 0) return std::wstring();
        return to_string(std::move(body));
    }

    // utf-16.
    else if (utility::details::str_icmp(charset, charset_types::utf16))
    {
        std::wstring body;
        body.resize(streambuf.in_avail() / sizeof(std::wstring::value_type));
        if (streambuf.scopy((unsigned char *)&body[0], body.size() * sizeof(std::wstring::value_type)) == 0) return std::wstring();
        return convert_utf16_to_string_t(std::move(body));
    }

    // utf-16le
    else if (utility::details::str_icmp(charset, charset_types::utf16le))
    {
        std::wstring body;
        body.resize(streambuf.in_avail() / sizeof(std::wstring::value_type));
        if (streambuf.scopy((unsigned char *)&body[0], body.size() * sizeof(std::wstring::value_type)) == 0) return std::wstring();
        return convert_utf16le_to_string_t(std::move(body), false);
    }

    // utf-16be
    else if (utility::details::str_icmp(charset, charset_types::utf16be))
    {
        std::wstring body;
        body.resize(streambuf.in_avail() / sizeof(std::wstring::value_type));
        if (streambuf.scopy((unsigned char *)&body[0], body.size() * sizeof(std::wstring::value_type)) == 0) return std::wstring();
        return convert_utf16be_to_string_t(std::move(body), false);
    }

    else
    {
        return std::wstring();
    }
}

//
// Helper function to generate a wstring from given http_headers and message body.
//
static std::wstring http_headers_body_to_string(const http_headers &headers, concurrency::streams::istream instream)
{
    std::wostringstream buffer;

    for (auto iter = headers.begin(); iter != headers.end(); ++iter)
    {
        buffer << iter->first << L": " << iter->second << CRLF;
    }
    buffer << CRLF;

    std::wstring content_type;
    if (headers.match(http::header_names::content_type, content_type))
    {
        buffer << convert_body_to_string_t(content_type, instream);
    }

    return buffer.str();
}

std::wstring details::http_msg_base::to_string() const
{
    return http_headers_body_to_string(m_headers, instream());
}

static void set_content_type_if_not_present(http::http_headers &headers, const std::wstring &content_type)
{
    std::wstring temp;
    if (!headers.match(http::header_names::content_type, temp))
    {
        headers.add(http::header_names::content_type, content_type);
    }
}

void details::http_msg_base::set_body(const streams::istream &instream, const std::string &contentType)
{
    set_content_type_if_not_present(
        headers(),
        utility::conversions::utf8_to_utf16(contentType));
    set_instream(instream);
}

void details::http_msg_base::set_body(const streams::istream &instream, const std::wstring &contentType)
{
    set_content_type_if_not_present(
        headers(),
        contentType);
    set_instream(instream);
}

void details::http_msg_base::set_body(const streams::istream &instream, unsigned __int64 contentLength, const std::string &contentType)
{
    headers().set_content_length(contentLength);
    set_body(instream, contentType);
    m_data_available.set(contentLength);
}

void details::http_msg_base::set_body(const concurrency::streams::istream &instream, unsigned __int64 contentLength, const std::wstring &contentType)
{
    headers().set_content_length(contentLength);
    set_body(instream, contentType);
    m_data_available.set(contentLength);
}

details::_http_request::_http_request(http::method mtd)
    : m_method(std::move(mtd)),
    m_initiated_response(0),
    m_server_context(),
    m_cancellationToken(pplx::cancellation_token::none())
{
    if (m_method.empty())
    {
        throw std::invalid_argument("Invalid HTTP method specified. Method can't be an empty string.");
    }
}

details::_http_request::_http_request(std::unique_ptr<http::details::_http_server_context> server_context)
    : m_initiated_response(0),
    m_server_context(std::move(server_context)),
    m_cancellationToken(pplx::cancellation_token::none())
{
}


}
}
} // namespace NX::web::http
