

#pragma once
#ifndef __NX_POLICY_PARSER_HPP__
#define __NX_POLICY_PARSER_HPP__


#include "policy_impl.hpp"

namespace NXPOLICY_IMPL {

std::shared_ptr<condition_object> parse_pql(const std::wstring& s, condition_map& cmap);
std::shared_ptr<policy_bundle> parse_pql_bundle_file(const std::wstring& file);
std::shared_ptr<policy_bundle> parse_pql_bundle_string(const std::wstring& file);

}


#endif

