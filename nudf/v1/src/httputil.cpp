

#include <Windows.h>
#include <assert.h>

#include <string>
#include <vector>

#include <nudf\exception.hpp>
#include <nudf\httputil.hpp>


using namespace nudf::http;



//
//  class CHandle
//
CHandle::CHandle() : _handle(NULL)
{
}

CHandle::~CHandle()
{
    Close();
}

void CHandle::Attach(_In_ HINTERNET h) throw()
{
    assert(NULL == _handle);
    InterlockedExchangePointer(&_handle, h);
}

HINTERNET CHandle::Detach() throw()
{
    return InterlockedExchangePointer(&_handle, NULL);
}

void CHandle::Close() throw()
{
    if(NULL != _handle) {
        ::WinHttpCloseHandle(_handle);
        _handle = NULL;
    }
}

HRESULT CHandle::SetOption(_In_ DWORD option, _In_ const void* value, _In_ DWORD length) throw()
{
    return (::WinHttpSetOption(_handle, option, const_cast<void*>(value), length)) ? S_OK : HRESULT_FROM_WIN32(::GetLastError());
}

HRESULT CHandle::QueryOption(_In_ DWORD option, _Out_ void* value, _Inout_ DWORD& length) const throw()
{
    return (::WinHttpQueryOption(_handle, option, value, &length)) ? S_OK : HRESULT_FROM_WIN32(::GetLastError());
}


//
//  class CSession
//
CSession::CSession() : CHandle(), _async(false)
{
}

CSession::CSession(_In_ const std::wstring& agent) : CHandle(), _agent(agent), _async(false)
{
}

CSession::~CSession()
{
    Close();
}

HRESULT CSession::Open(_In_ bool async)
{
    Attach(::WinHttpOpen(_agent.empty() ? NULL : _agent.c_str(),
                         WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                         WINHTTP_NO_PROXY_NAME,
                         WINHTTP_NO_PROXY_BYPASS,
                         async ? WINHTTP_FLAG_ASYNC : 0));
    return IsHandleValid() ? S_OK : HRESULT_FROM_WIN32(::GetLastError());
}

void CSession::Close() throw()
{
    CHandle::Close();
    _async = false;
}



//
//  class CConnect
//

CConnection::CConnection() : CHandle(), _port(INTERNET_DEFAULT_PORT)
{
}

CConnection::CConnection(_In_ const std::wstring& server, _In_ INTERNET_PORT port) : CHandle(), _server(server), _port(port)
{
}

CConnection::~CConnection()
{
}

HRESULT CConnection::Connect(_In_ const CSession& session) throw()
{
    Attach(::WinHttpConnect(session, _server.c_str(), _port, 0));
    return IsHandleValid() ? S_OK : HRESULT_FROM_WIN32(::GetLastError());
}



//
//  class CResponse
//
HRESULT CResponse::GetResponse(_In_ HINTERNET hRequest)
{
    HRESULT hr = S_OK;

    if(!WinHttpReceiveResponse(hRequest, NULL)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    hr = GetHeaders(hRequest);
    if(FAILED(hr)) {
        return hr;
    }

    hr = GetContent(hRequest);
    if(FAILED(hr)) {
        return hr;
    }

    return hr;
}

void CResponse::Clear()
{
    _headers.clear();
    _content.clear();
}

HRESULT CResponse::GetHeaders(_In_ HINTERNET hRequest)
{
    WCHAR wzHeaders[4096] = {0};
    DWORD dwBufferLength = 4096 * sizeof(WCHAR);
    DWORD dwIndex = 0;
    if(!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_RAW_HEADERS, NULL, wzHeaders, &dwBufferLength, &dwIndex)) {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    LPCWSTR wzHeaderItem = wzHeaders;
    while(L'\0' != wzHeaderItem[0]) {
        std::wstring wsHeader(wzHeaderItem);
        wzHeaderItem += wsHeader.length() + 1;
        _headers.push_back(wsHeader);
    }
    return S_OK;
}

HRESULT CResponse::GetContent(_In_ HINTERNET hRequest)
{
    HRESULT hr = S_OK;
    DWORD   dwSize = 0;

    _content.clear();

    do {
    
        std::vector<char> buf;
        DWORD dwDownloaded = 0;

        // Check for available data.
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        // No more available data.
        if (0 == dwSize) {
            break;
        }
        
        buf.resize(dwSize+1, 0);

        if (!WinHttpReadData(hRequest, (LPVOID)(&buf[0]), dwSize, &dwDownloaded)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            break;
        }

        // This condition should never be reached since WinHttpQueryDataAvailable
        // reported that there are bits to read.
        if (0 == dwDownloaded) {
            break;
        }

        _content += (&buf[0]);

    } while (dwSize > 0);

    return hr;
}


//
//  class CRequest
//
static void CALLBACK RequestCallback(_In_ HINTERNET handle, _In_ DWORD_PTR context, _In_ DWORD code, _In_ void* info, _In_ DWORD length);

CRequest::CRequest() : CHandle(), _secure(false), _async(false)
{
}

CRequest::CRequest(_In_ const std::wstring& verb) : CHandle(), _verb(verb), _secure(false), _async(false)
{
}

CRequest::CRequest(_In_ const std::wstring& verb, _In_ const std::wstring& path) : CHandle(), _path(path), _verb(verb), _secure(false), _async(false)
{
}

CRequest::~CRequest()
{
}

HRESULT CRequest::Initialize(_In_ const CSession& session, _In_ const std::wstring& server, _In_ INTERNET_PORT port) throw()
{
    HRESULT hr = S_OK;

    _conn.SetServer(server);
    _conn.SetConnPort(port);

    hr = _conn.Connect(session);
    if(FAILED(hr)) {
        return hr;
    }

    Attach(::WinHttpOpenRequest(_conn, _verb.c_str(), _path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, _secure ? WINHTTP_FLAG_SECURE : 0));
    if(NULL == GetHandle()) {
        _conn.Close();
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    if(_secure) {

        DWORD dwSecureFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
            SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
            SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
            SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

        hr = SetOption(WINHTTP_OPTION_SECURITY_FLAGS, &dwSecureFlags, sizeof(DWORD));
        if(FAILED(hr)) {
            Close();
            return hr;
        }

        DWORD dwSecureProto = WINHTTP_FLAG_SECURE_PROTOCOL_TLS1_2;
        hr = SetOption(WINHTTP_OPTION_SECURE_PROTOCOLS, &dwSecureProto, sizeof(DWORD));
        if(FAILED(hr)) {
            Close();
            return hr;
        }
    }

    if(session.IsAsync()) {

        _async = true;

        if(WINHTTP_INVALID_STATUS_CALLBACK == ::WinHttpSetStatusCallback(GetHandle(), RequestCallback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            Close();
            return hr;
        }
    }

    return S_OK;
}

HRESULT CRequest::Reset()
{
    HRESULT hr = S_OK;

    CHandle::Close();    
    Attach(::WinHttpOpenRequest(_conn, _verb.c_str(), _path.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, _secure ? WINHTTP_FLAG_SECURE : 0));
    if(NULL == GetHandle()) {
        _conn.Close();
        return HRESULT_FROM_WIN32(::GetLastError());
    }

    if(_secure) {

        DWORD dwSecureFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
            SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
            SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
            SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;

        hr = SetOption(WINHTTP_OPTION_SECURITY_FLAGS, &dwSecureFlags, sizeof(DWORD));
        if(FAILED(hr)) {
            Close();
            return hr;
        }
    }

    if(_async) {

        if(WINHTTP_INVALID_STATUS_CALLBACK == ::WinHttpSetStatusCallback(GetHandle(), RequestCallback, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0)) {
            hr = HRESULT_FROM_WIN32(::GetLastError());
            Close();
            return hr;
        }
    }

    return hr;
}

void CRequest::Close() throw()
{
    _conn.Close();
    CHandle::Close();
}

HRESULT CRequest::AddHeaders(_In_ LPCWSTR headers) throw()
{
    return (WinHttpAddRequestHeaders(GetHandle(), headers, -1L, WINHTTP_ADDREQ_FLAG_ADD) ? S_OK : HRESULT_FROM_WIN32(::GetLastError()));
}

HRESULT CRequest::MergeHeaders(_In_ LPCWSTR headers) throw()
{
    return (WinHttpAddRequestHeaders(GetHandle(), headers, -1L, WINHTTP_ADDREQ_FLAG_COALESCE) ? S_OK : HRESULT_FROM_WIN32(::GetLastError()));
}

HRESULT CRequest::MergeHeadersByComma(_In_ LPCWSTR headers) throw()
{
    return (WinHttpAddRequestHeaders(GetHandle(), headers, -1L, WINHTTP_ADDREQ_FLAG_COALESCE_WITH_COMMA) ? S_OK : HRESULT_FROM_WIN32(::GetLastError()));
}

HRESULT CRequest::MergeHeadersBySemicolon(_In_ LPCWSTR headers) throw()
{
    return (WinHttpAddRequestHeaders(GetHandle(), headers, -1L, WINHTTP_ADDREQ_FLAG_COALESCE_WITH_SEMICOLON) ? S_OK : HRESULT_FROM_WIN32(::GetLastError()));
}

HRESULT CRequest::RemoveHeader(_In_ LPCWSTR names) throw()
{
    return (WinHttpAddRequestHeaders(GetHandle(), names, -1L, WINHTTP_ADDREQ_FLAG_REPLACE) ? S_OK : HRESULT_FROM_WIN32(::GetLastError()));
}

HRESULT CRequest::ReplaceHeaders(_In_ LPCWSTR headers) throw()
{
    return (WinHttpAddRequestHeaders(GetHandle(), headers, -1L, WINHTTP_ADDREQ_FLAG_REPLACE) ? S_OK : HRESULT_FROM_WIN32(::GetLastError()));
}

HRESULT CRequest::Send(_In_ DWORD data_size) throw()
{
    return (::WinHttpSendRequest( GetHandle(),
                                  WINHTTP_NO_ADDITIONAL_HEADERS,
                                  0,
                                  WINHTTP_NO_REQUEST_DATA,
                                  0,
                                  data_size,
                                  (DWORD_PTR)this)) ? S_OK : HRESULT_FROM_WIN32(::GetLastError());
}

HRESULT CRequest::Send(_In_ PVOID data, _In_ DWORD data_size) throw()
{
    return (::WinHttpSendRequest( GetHandle(),
                                  WINHTTP_NO_ADDITIONAL_HEADERS,
                                  0,
                                  data,
                                  data_size,
                                  data_size,
                                  (DWORD_PTR)this)) ? S_OK : HRESULT_FROM_WIN32(::GetLastError());
}

HRESULT CRequest::GetResponse(_Out_ CResponse& response)
{
    return response.GetResponse(GetHandle());
}

void CRequest::OnCallback(_In_ DWORD code, _In_ void* info, _In_ DWORD length)
{
    switch(code)
    {
    case WINHTTP_CALLBACK_STATUS_CLOSING_CONNECTION:
        OnClosingConnection();
        break;
    case WINHTTP_CALLBACK_STATUS_CONNECTED_TO_SERVER:
        OnConnectedToServer((LPCWSTR)info);
        break;
    case WINHTTP_CALLBACK_STATUS_CONNECTING_TO_SERVER:
        OnConnectingToServer((LPCWSTR)info);
        break;
    case WINHTTP_CALLBACK_STATUS_CONNECTION_CLOSED:
        OnConnectionClosed();
        break;
    case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE:
        OnDataAvailable(*((PDWORD)info));
        break;
    case WINHTTP_CALLBACK_STATUS_HANDLE_CREATED:
        OnHandleCreated(info ? (*((HINTERNET*)info)) : NULL);
        break;
    case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING:
        OnHandleClosing(info ? (*((HINTERNET*)info)) : NULL);
        break;
    case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE:
        OnHeaderAvailable();
        break;
    case WINHTTP_CALLBACK_STATUS_INTERMEDIATE_RESPONSE:
        OnIntermediateResponse(*((PDWORD)info));
        break;
    case WINHTTP_CALLBACK_STATUS_NAME_RESOLVED:
        OnNameResolved((LPCWSTR)info);
        break;
    case WINHTTP_CALLBACK_STATUS_READ_COMPLETE:
        OnReadComplete((PUCHAR)info, length);
        break;
    case WINHTTP_CALLBACK_STATUS_RECEIVING_RESPONSE:
        OnReceivingResponse();
        break;
    case WINHTTP_CALLBACK_STATUS_REDIRECT:
        OnRedirect((LPCWSTR)info);
        break;
    case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR:
        OnRequestError((const WINHTTP_ASYNC_RESULT*)info);
        break;
    case WINHTTP_CALLBACK_STATUS_REQUEST_SENT:
        OnRequestSent(*((PDWORD)info));
        break;
    case WINHTTP_CALLBACK_STATUS_RESOLVING_NAME:
        OnResolvingName((LPCWSTR)info);
        break;
    case WINHTTP_CALLBACK_STATUS_RESPONSE_RECEIVED:
        OnResponseReceived(*((PDWORD)info));
        break;
    case WINHTTP_CALLBACK_STATUS_SECURE_FAILURE:
        OnSecureFailure(*((PDWORD)info));
        break;
    case WINHTTP_CALLBACK_STATUS_SENDING_REQUEST:
        OnSendingRequest();
        break;
    case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE:
        OnSendRequestComplete();
        break;
    case WINHTTP_CALLBACK_STATUS_WRITE_COMPLETE:
        OnWriteComplete(info, length);
        break;
    case WINHTTP_CALLBACK_STATUS_GETPROXYFORURL_COMPLETE:
        OnGetProxyForUrlComplete();
        break;
    case WINHTTP_CALLBACK_STATUS_CLOSE_COMPLETE:
        OnCloseComplete();
        break;
    case WINHTTP_CALLBACK_STATUS_SHUTDOWN_COMPLETE:
        OnShutdownComplete();
        break;
    default:
        OnUnknownStatus(code, info, length);
        break;
    }
}

static void CALLBACK RequestCallback(_In_ HINTERNET handle, _In_ DWORD_PTR context, _In_ DWORD code, _In_ void* info, _In_ DWORD length)
{
    if (NULL != context) {
        //T* pT = reinterpret_cast<T*>(context);
        CRequest* pT = (CRequest*)context;
        pT->OnCallback(code, info, length);
    }
}
