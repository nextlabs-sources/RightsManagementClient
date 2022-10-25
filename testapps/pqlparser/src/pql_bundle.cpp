

#include <Windows.h>

#include <boost\algorithm\string.hpp>

#include "pql_bundle.hpp"


const std::string pql::bundle::_Bundle::TAG_DEPLOYMENT_TIME("Deployment time:");
const std::string pql::bundle::_Bundle::TAG_DEPLOYMENT_ENTITIES("Deployment entities:");
const std::string pql::bundle::_Bundle::TAG_POLICY_TO_ALL_USERS("Policies that apply to all users:");
const std::string pql::bundle::_Bundle::TAG_POLICY_TO_ALL_HOSTS("Policies that apply to all hosts:");
const std::string pql::bundle::_Bundle::TAG_POLICY_TO_ALL_APPLICATIONS("Policies that apply to all applications:");
const std::string pql::bundle::_Bundle::TAG_SUBJECT_TO_POLICY_MAP("Subject-Policy Mappings:");
const std::string pql::bundle::_Bundle::TAG_SUBJECT_KEYS("Subject keys:");


void pql::bundle::_Bundle::parse(std::basic_istream<char>& ss)
{
    std::map<__int64, std::set<__int64>> policy_to_subject_id_map;

    while (!ss.eof()) {

        std::string line;
        get_nextline(ss, line);
        
        // empty line
        if (line.empty()) {
            continue;
        }

        //
        // Deployment time
        //
        if (boost::algorithm::starts_with(line, TAG_DEPLOYMENT_TIME)) {
            continue;
        }

        //
        // Deployment entities
        //
        if (boost::algorithm::starts_with(line, TAG_DEPLOYMENT_ENTITIES)) {

            get_nextline(ss, line);

            do {

                // Empty line?
                if (line.empty()) {
                    break;
                }

                // First line of a policy
                if (boost::algorithm::starts_with(line, pql::bundle::_Policy::TAG_ID)) {

                    std::shared_ptr<pql::bundle::_Policy> policy(new pql::bundle::_Policy(line));

                    // get all other lines of this policy
                    do {
                        get_nextline(ss, line);
                        if (boost::algorithm::starts_with(line, pql::bundle::_Policy::TAG_ID) || line.empty()) {
                            break;
                        }
                        policy->add_line(line);
                    } while (true);

                    // get all the lines for this policy, add it to map
                    if (policy->is_valid()) {
                        _policy_map[policy->id()] = policy;
                    }

                    // don't need to get a new line because a new line has been gotten in above do-while loop
                    continue;
                }

            } while (!ss.eof() && !line.empty());

            // Done with Deployment entities, start a new loop to get next line
            continue;
        }

        //
        // Policies that apply to all users
        //
        if (boost::algorithm::starts_with(line, TAG_POLICY_TO_ALL_USERS)) {
            std::string::size_type pos = line.find('{');
            if (pos != std::string::npos) {
                line = line.substr(pos + 1);                // remove header until '{'
                line = line.substr(0, line.length() - 1);   // remove end '}'
                boost::algorithm::trim(line);               // remove all the space
                line_to_set<__int64>(line, ',', _policy_to_all_user, [&](const std::string& s) -> __int64 {
                    if (!is_digit<char>(s)) {
                        throw std::bad_cast();
                    }
                    return std::atoll(s.c_str());
                });
            }
            // Done with "Policies that apply to all users", start a new loop to get next line
            continue;
        }

        //
        // Policies that apply to all hosts
        //
        if (boost::algorithm::starts_with(line, TAG_POLICY_TO_ALL_HOSTS)) {
            std::string::size_type pos = line.find('{');
            if (pos != std::string::npos) {
                line = line.substr(pos + 1);                // remove header until '{'
                line = line.substr(0, line.length() - 1);   // remove end '}'
                boost::algorithm::trim(line);               // remove all the space
                line_to_set<__int64>(line, ',', _policy_to_all_host, [&](const std::string& s) -> __int64 {
                    if (!is_digit<char>(s)) {
                        throw std::bad_cast();
                    }
                    return std::atoll(s.c_str());
                });
            }
            // Done with "Policies that apply to all hosts", start a new loop to get next line
            continue;
        }

        //
        // Policies that apply to all applications
        //
        if (boost::algorithm::starts_with(line, TAG_POLICY_TO_ALL_APPLICATIONS)) {
            std::string::size_type pos = line.find('{');
            if (pos != std::string::npos) {
                line = line.substr(pos + 1);                // remove header until '{'
                line = line.substr(0, line.length() - 1);   // remove end '}'
                boost::algorithm::trim(line);               // remove all the space
                line_to_set<__int64>(line, ',', _policy_to_all_host, [&](const std::string& s) -> __int64 {
                    if (!is_digit<char>(s)) {
                        throw std::bad_cast();
                    }
                    return std::atoll(s.c_str());
                });
            }
            // Done with "Policies that apply to all applications", start a new loop to get next line
            continue;
        }

        //
        // Subject-Policy Mappings
        //
        if (boost::algorithm::starts_with(line, TAG_SUBJECT_TO_POLICY_MAP)) {
            
            static const std::string ARROW_SIGN(" -> {");

            while (!ss.eof()) {
                get_nextline(ss, line);
                std::string::size_type pos = line.find(ARROW_SIGN);
                if (pos == std::string::npos) {
                    // not a valid map item
                    break;
                }
                std::string subject_id = line.substr(0, pos);
                std::string policy_id_list = line.substr(pos + ARROW_SIGN.length());
                policy_id_list = policy_id_list.substr(0, policy_id_list.length()-1);
                boost::algorithm::trim(policy_id_list);
                int n_subject_id = atoi(subject_id.c_str());
                if (n_subject_id < 0) {
                    // invalid subject id?
                    continue;
                }
                // add a new item
                policy_to_subject_id_map[n_subject_id] = std::set<__int64>();
                std::set<__int64>& set = policy_to_subject_id_map[n_subject_id];
                line_to_set<__int64>(policy_id_list, ',', set, [&](const std::string& s) -> __int64 {
                    if (!is_digit<char>(s)) {
                        throw std::bad_cast();
                    }
                    return std::atoll(s.c_str());
                });
            }


            // Done with "Subject-Policy Mappings", start a new loop to get next line
            continue;
        }

        //
        // Subject keys:
        //
        if (boost::algorithm::starts_with(line, TAG_SUBJECT_KEYS)) {
            
            static const std::string SID_ARROW_SIGN(" (user-windowsSid) -> ");
            static const std::string UNKNOWN_USER_ARROW_SIGN("{UNKNOWN-USER} (user-builtin) -> ");
            static const std::string UNKNOWN_HOST_ARROW_SIGN("{UNKNOWN-HOST} (host-builtin) -> ");
            static const std::string UNKNOWN_APP_ARROW_SIGN("{UNKNOWN-APP} (application-builtin) -> ");

            while (!ss.eof()) {

                get_nextline(ss, line);

                if (line.empty()) {
                    break;
                }

                if (boost::algorithm::istarts_with(line, UNKNOWN_USER_ARROW_SIGN)) {
                    _unknown_user_id = std::atoll(line.substr(UNKNOWN_USER_ARROW_SIGN.length()).c_str());
                }
                else if (boost::algorithm::istarts_with(line, UNKNOWN_APP_ARROW_SIGN)) {
                    _unknown_app_id = std::atoll(line.substr(UNKNOWN_APP_ARROW_SIGN.length()).c_str());
                }
                else if (boost::algorithm::istarts_with(line, UNKNOWN_HOST_ARROW_SIGN)) {
                    _unknown_host_id = std::atoll(line.substr(UNKNOWN_HOST_ARROW_SIGN.length()).c_str());
                }
                else {
                    std::string::size_type pos = line.find(SID_ARROW_SIGN);
                    if (std::string::npos == pos) {
                        // Not a SID?
                        continue;
                    }
                    else {
                        // Sid, for example:
                        // S-1-5-21-1461305-839808047-932995037-183417 (user-windowsSid) -> 57505
                        std::string sid = line.substr(0, pos); boost::algorithm::trim(sid);
                        std::string subject_id = line.substr(pos + SID_ARROW_SIGN.length()); boost::algorithm::trim(subject_id);
                        _subject_id_map[sid] = std::atoll(subject_id.c_str());
                    }
                }
            }


            // Done with "Subject-Policy Mappings", start a new loop to get next line
            continue;
        }

    }

    //
    //  At last, set all the policy ids to subjects
    //
    std::for_each(_subject_id_map.begin(), _subject_id_map.end(), [&](const std::pair<std::string, __int64>& it) {
        if (policy_to_subject_id_map[it.second].size() != 0) {
            _policy_to_subject[it.first] = policy_to_subject_id_map[it.second];
        }
    });
    if (-1 != _unknown_user_id) {
        _policy_to_unknown_users = policy_to_subject_id_map[_unknown_user_id];
    }
    if (-1 != _unknown_app_id) {
        _policy_to_unknown_apps = policy_to_subject_id_map[_unknown_app_id];
    }
    if (-1 != _unknown_host_id) {
        _policy_to_unknown_hosts = policy_to_subject_id_map[_unknown_host_id];
    }

    //
    //  DONE
    //
}

void pql::bundle::_Bundle::get_nextline(std::basic_istream<char>& ss, std::string& line)
{
    std::getline(ss, line);
    boost::algorithm::trim_left(line);
    boost::algorithm::trim_right(line);
}



const std::string pql::bundle::_Policy::TAG_ID("ID ");
const std::string pql::bundle::_Policy::TAG_DESCRIPTION("DESCRIPTION ");
const std::string pql::bundle::_Policy::TAG_ATTRIBUTE("ATTRIBUTE ");
const std::string pql::bundle::_Policy::TAG_FOR("FOR ");
const std::string pql::bundle::_Policy::TAG_ON("ON ");
const std::string pql::bundle::_Policy::TAG_TO("TO ");
const std::string pql::bundle::_Policy::TAG_BY("BY ");
const std::string pql::bundle::_Policy::TAG_WHERE("WHERE ");
const std::string pql::bundle::_Policy::TAG_DO("DO ");
const std::string pql::bundle::_Policy::TAG_ON_ALLOW_DO("ON allow DO ");
const std::string pql::bundle::_Policy::TAG_ON_DENY_DO("ON deny DO ");


pql::bundle::_Policy::_Policy(const std::string& s) : _id(-1), _approved(false), _hidden(false)
{
    _map[TAG_ID] = "";
    _map[TAG_DESCRIPTION] = "";
    _map[TAG_ATTRIBUTE] = "";
    _map[TAG_FOR] = "";
    _map[TAG_ON] = "";
    _map[TAG_TO] = "";
    _map[TAG_BY] = "";
    _map[TAG_WHERE] = "";
    _map[TAG_DO] = "";
    _map[TAG_ON_ALLOW_DO] = "";
    _map[TAG_ON_DENY_DO] = "";
    set_title(s);
}

void pql::bundle::_Policy::clear()
{
    _id = -1;
    _approved = false;
    _hidden = false;
    _name.clear();
    _attribute.clear();
    _map.clear();
}

void pql::bundle::_Policy::set_title(const std::string& s)
{
    static const std::string TAG_POLICY_NAME(" POLICY \"");
    static const std::string TAG_POLICY_STATUS_APPROVED(" APPROVED ");
    static const std::string TAG_POLICY_STATUS_HIDDEN(" HIDDEN ");

    std::string ls;
    std::string::size_type pos;

    if (!boost::algorithm::istarts_with(s, TAG_ID)) {
        throw std::invalid_argument("bad policy title line: ID tag not found");
    }

    pos = s.find(TAG_POLICY_NAME);
    if (pos == std::string::npos) {
        throw std::invalid_argument("bad policy title line: POLICY NAME tag not found");
    }

    // get name, and string without name
    ls = s.substr(TAG_ID.length(), pos-TAG_ID.length());
    _name = s.substr(pos + TAG_POLICY_NAME.length());
    if (boost::algorithm::ends_with(_name, "\"")) {
        _name = _name.substr(0, _name.length() - 1);
    }

    // get id and other attributes
    _approved = boost::algorithm::contains(ls, TAG_POLICY_STATUS_APPROVED);
    _hidden = boost::algorithm::contains(ls, TAG_POLICY_STATUS_HIDDEN);
    boost::algorithm::trim_left(ls);
    pos = ls.find(' ');
    if (std::string::npos == pos) {
        throw std::invalid_argument("bad policy title line: POLICY ID not found");
    }
    ls = ls.substr(0, pos);
    std::for_each(ls.begin(), ls.end(), [](char c) {
        if (c<'0' || c>'9') {
            throw std::invalid_argument("bad policy title line: POLICY ID is not a number");
        }
    });
    _id = atoi(ls.c_str());

    // put this to map
    _map[TAG_ID] = s.substr(TAG_ID.length());
}

void pql::bundle::_Policy::add_line(const std::string& s)
{
    if (s.empty()) {
        throw std::invalid_argument("bad policy line: empty");
    }

    if (boost::algorithm::starts_with(s, TAG_ID)) {
        set_title(s);
        return;
    }
    if (boost::algorithm::starts_with(s, TAG_ON_ALLOW_DO)) {
        _map[TAG_ON_ALLOW_DO] = s.substr(TAG_ON_ALLOW_DO.length());
        return;
    }
    if (boost::algorithm::starts_with(s, TAG_ON_DENY_DO)) {
        _map[TAG_ON_DENY_DO] = s.substr(TAG_ON_DENY_DO.length());
        return;
    }
    
    if (boost::algorithm::starts_with(s, TAG_DESCRIPTION)) {
        _map[TAG_DESCRIPTION] = s.substr(TAG_DESCRIPTION.length());
    }
    else if (boost::algorithm::starts_with(s, TAG_ATTRIBUTE)) {
        _map[TAG_ATTRIBUTE] = s.substr(TAG_ATTRIBUTE.length());
    }
    else if (boost::algorithm::starts_with(s, TAG_FOR)) {
        _map[TAG_FOR] = s.substr(TAG_FOR.length());
    }
    else if (boost::algorithm::starts_with(s, TAG_ON)) {
        _map[TAG_ON] = s.substr(TAG_ON.length());
    }
    else if (boost::algorithm::starts_with(s, TAG_TO)) {
        _map[TAG_TO] = s.substr(TAG_TO.length());
    }
    else if (boost::algorithm::starts_with(s, TAG_BY)) {
        _map[TAG_BY] = s.substr(TAG_BY.length());
    }
    else if (boost::algorithm::starts_with(s, TAG_WHERE)) {
        _map[TAG_WHERE] = s.substr(TAG_WHERE.length());
    }
    else if (boost::algorithm::starts_with(s, TAG_DO)) {
        _map[TAG_DO] = s.substr(TAG_DO.length());
    }
    else {
        ; // Nothing
    }
}