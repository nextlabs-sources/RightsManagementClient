

#include <Windows.h>

#include <set>
#include <boost\algorithm\string.hpp>

#include <nudf\web\json.hpp>
#include <nudf\exception.hpp>
#include <nudf\string.hpp>

#include <rapidxml\rapidxml.hpp>

#include "nxrmserv.h"
#include "policy.hpp"


using namespace NX;
using namespace NX::EVAL;



//
//  class policy_bundle::json_parser
//
policy_bundle::json_parser::json_parser() : policy_bundle::parser()
{
}

policy_bundle::json_parser::~json_parser()
{
}

policy_bundle policy_bundle::json_parser::parse(const std::string& s /*utf8*/)
{
    std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
    return parse(ws);
}

policy_bundle policy_bundle::json_parser::parse(const std::wstring& ws /*utf16*/)
{
    policy_bundle bundle;
    return bundle;
}
