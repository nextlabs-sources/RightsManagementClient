

#include <Windows.h>

#include <string>
#include <vector>

#include <nxrmrest\nxrmrest.hpp>
#include <nxrmrest\schema\RegisterAgentRequest.hpp>
#include <nxrmrest\schema\RegisterAgentResponse.hpp>

using namespace nxrmrest;



CRegisterRequest::CRegisterRequest() : nudf::http::CRequest(L"POST")
{
    schema::CAgentRegisterRequest  regrequest;
    schema::CAgentRegisterResponse  regresponse;
}

CRegisterRequest::CRegisterRequest(_In_ const std::wstring& path) : nudf::http::CRequest(L"POST", path)
{
}

CRegisterRequest::~CRegisterRequest()
{
}