

#include <Windows.h>

#include <nudf\web\http_constant.hpp>


using namespace NX::web::http;



const method methods::GET                                           (L"GET");
const method methods::POST                                          (L"POST");
const method methods::PUT                                           (L"PUT");
const method methods::DEL                                           (L"DELETE");
const method methods::HEAD                                          (L"HEAD");
const method methods::OPTIONS                                       (L"OPTIONS");
const method methods::TRCE                                          (L"TRACE");
const method methods::CONNECT                                       (L"CONNECT");
const method methods::MERGE                                         (L"MERGE");
const method methods::PATCH                                         (L"PATCH");

const std::wstring header_names::accept                             (L"Accept");
const std::wstring header_names::accept_charset                     (L"Accept-Charset");
const std::wstring header_names::accept_encoding                    (L"Accept-Encoding");
const std::wstring header_names::accept_language                    (L"Accept-Language");
const std::wstring header_names::accept_ranges                      (L"Accept-Ranges");
const std::wstring header_names::age                                (L"Age");
const std::wstring header_names::allow                              (L"Allow");
const std::wstring header_names::authorization                      (L"Authorization");
const std::wstring header_names::cache_control                      (L"Cache-Control");
const std::wstring header_names::connection                         (L"Connection");
const std::wstring header_names::content_encoding                   (L"Content-Encoding");
const std::wstring header_names::content_language                   (L"Content-Language");
const std::wstring header_names::content_length                     (L"Content-Length");
const std::wstring header_names::content_location                   (L"Content-Location");
const std::wstring header_names::content_md5                        (L"Content-MD5");
const std::wstring header_names::content_range                      (L"Content-Range");
const std::wstring header_names::content_type                       (L"Content-Type");
const std::wstring header_names::date                               (L"Date");
const std::wstring header_names::etag                               (L"ETag");
const std::wstring header_names::expect                             (L"Expect");
const std::wstring header_names::expires                            (L"Expires");
const std::wstring header_names::from                               (L"From");
const std::wstring header_names::host                               (L"Host");
const std::wstring header_names::if_match                           (L"If-Match");
const std::wstring header_names::if_modified_since                  (L"If-Modified-Since");
const std::wstring header_names::if_none_match                      (L"If-None-Match");
const std::wstring header_names::if_range                           (L"If-Range");
const std::wstring header_names::if_unmodified_since                (L"If-Unmodified-Since");
const std::wstring header_names::last_modified                      (L"Last-Modified");
const std::wstring header_names::location                           (L"Location");
const std::wstring header_names::max_forwards                       (L"Max-Forwards");
const std::wstring header_names::pragma                             (L"Pragma");
const std::wstring header_names::proxy_authenticate                 (L"Proxy-Authenticate");
const std::wstring header_names::proxy_authorization                (L"Proxy-Authorization");
const std::wstring header_names::range                              (L"Range");
const std::wstring header_names::referer                            (L"Referer");
const std::wstring header_names::retry_after                        (L"Retry-After");
const std::wstring header_names::server                             (L"Server");
const std::wstring header_names::te                                 (L"TE");
const std::wstring header_names::trailer                            (L"Trailer");
const std::wstring header_names::transfer_encoding                  (L"Transfer-Encoding");
const std::wstring header_names::upgrade                            (L"Upgrade");
const std::wstring header_names::user_agent                         (L"User-Agent");
const std::wstring header_names::vary                               (L"Vary");
const std::wstring header_names::via                                (L"Via");
const std::wstring header_names::warning                            (L"Warning");
const std::wstring header_names::www_authenticate                   (L"WWW-Authenticate");


const std::wstring mime_types::application_atom_xml                 (L"application/atom+xml");
const std::wstring mime_types::application_http                     (L"application/http");
const std::wstring mime_types::application_javascript               (L"application/javascript");
const std::wstring mime_types::application_json                     (L"application/json");
const std::wstring mime_types::application_xjson                    (L"application/x-json");
const std::wstring mime_types::application_octetstream              (L"application/octet-stream");
const std::wstring mime_types::application_x_www_form_urlencoded    (L"application/x-www-form-urlencoded");
const std::wstring mime_types::application_xjavascript              (L"application/x-javascript");
const std::wstring mime_types::application_xml                      (L"application/xml");
const std::wstring mime_types::message_http                         (L"message/http");
const std::wstring mime_types::text                                 (L"text");
const std::wstring mime_types::text_javascript                      (L"text/javascript");
const std::wstring mime_types::text_json                            (L"text/json");
const std::wstring mime_types::text_plain                           (L"text/plain");
const std::wstring mime_types::text_plain_utf16                     (L"text/plain; charset=utf-16");
const std::wstring mime_types::text_plain_utf16le                   (L"text/plain; charset=utf-16le");
const std::wstring mime_types::text_plain_utf8                      (L"text/plain; charset=utf-8");
const std::wstring mime_types::text_xjavascript                     (L"text/x-javascript");
const std::wstring mime_types::text_xjson                           (L"text/x-json");


const std::wstring charset_types::ascii                             (L"ascii");
const std::wstring charset_types::usascii                           (L"us-ascii");
const std::wstring charset_types::latin1                            (L"iso-8859-1");
const std::wstring charset_types::utf8                               (L"utf-8");
const std::wstring charset_types::utf16                             (L"utf-16");
const std::wstring charset_types::utf16le                           (L"utf-16le");
const std::wstring charset_types::utf16be                           (L"utf-16be");


const std::wstring oauth1_methods::hmac_sha1                        (L"HMAC-SHA1");
const std::wstring oauth1_methods::plaintext                        (L"PLAINTEXT");


const std::wstring oauth1_strings::callback                         (L"oauth_callback");
const std::wstring oauth1_strings::callback_confirmed               (L"oauth_callback_confirmed");
const std::wstring oauth1_strings::consumer_key                     (L"oauth_consumer_key");
const std::wstring oauth1_strings::nonce                            (L"oauth_nonce");
const std::wstring oauth1_strings::realm                            (L"realm"); // NOTE: No "oauth_" prefix.
const std::wstring oauth1_strings::signature                        (L"oauth_signature");
const std::wstring oauth1_strings::signature_method                 (L"oauth_signature_method");
const std::wstring oauth1_strings::timestamp                        (L"oauth_timestamp");
const std::wstring oauth1_strings::token                            (L"oauth_token");
const std::wstring oauth1_strings::token_secret                     (L"oauth_token_secret");
const std::wstring oauth1_strings::verifier                         (L"oauth_verifier");
const std::wstring oauth1_strings::version                          (L"oauth_version");


const std::wstring oauth2_strings::access_token                     (L"access_token");
const std::wstring oauth2_strings::authorization_code               (L"authorization_code");
const std::wstring oauth2_strings::bearer                           (L"bearer");
const std::wstring oauth2_strings::client_id                        (L"client_id");
const std::wstring oauth2_strings::client_secret                    (L"client_secret");
const std::wstring oauth2_strings::code                             (L"code");
const std::wstring oauth2_strings::expires_in                       (L"expires_in");
const std::wstring oauth2_strings::grant_type                       (L"grant_type");
const std::wstring oauth2_strings::redirect_uri                     (L"redirect_uri");
const std::wstring oauth2_strings::refresh_token                    (L"refresh_token");
const std::wstring oauth2_strings::response_type                    (L"response_type");
const std::wstring oauth2_strings::scope                            (L"scope");
const std::wstring oauth2_strings::state                            (L"state");
const std::wstring oauth2_strings::token                            (L"token");
const std::wstring oauth2_strings::token_type                       (L"token_type");