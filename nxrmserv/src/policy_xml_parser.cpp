

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


class xml_policy_object
{
public:
    xml_policy_object() {}
    ~xml_policy_object() {}

    std::wstring                        _id;
    std::wstring                        _name;
    std::wstring                        _user_group;
    std::vector<std::wstring/*hash*/>   _obligations;
    std::vector<std::wstring/*name*/>   _rights;
    //std::vector<std::vector<std::wstring/*hash*/>>  _include_subj_comp;
    //std::vector<std::vector<std::wstring/*hash*/>>  _exclude_subj_comp;
    std::vector<std::vector<std::wstring/*hash*/>>  _include_user_comp;
    std::vector<std::vector<std::wstring/*hash*/>>  _exclude_user_comp;
    std::vector<std::vector<std::wstring/*hash*/>>  _include_app_comp;
    std::vector<std::vector<std::wstring/*hash*/>>  _exclude_app_comp;
    std::vector<std::vector<std::wstring/*hash*/>>  _include_host_comp;
    std::vector<std::vector<std::wstring/*hash*/>>  _exclude_host_comp;
    std::vector<std::vector<std::wstring/*hash*/>>  _include_res_comp;
    std::vector<std::vector<std::wstring/*hash*/>>  _exclude_res_comp;
    std::vector<std::vector<std::wstring/*hash*/>>  _include_env_comp;
    std::vector<std::vector<std::wstring/*hash*/>>  _exclude_env_comp;
};

class xml_bundle
{
public:
    typedef std::map<std::wstring/*hash*/, std::shared_ptr<details::condition>/*condition sp*/>         ConditionMap;
    typedef std::map<std::wstring/*hash*/, std::shared_ptr<obligation>/*condition sp*/>                 ObligationMap;
    typedef std::map<std::wstring/*id*/, std::vector<std::wstring/*condition hash*/>/*condition list*/> ComponentMap;
    typedef std::pair<std::wstring/*context*/, std::vector<std::wstring/*group_id*/>/*group id list*/>  UserGroupInfo;
    typedef std::map<std::wstring/*sid*/, UserGroupInfo>                                                UserGroupMap;
    typedef std::map<std::wstring/*id*/, xml_policy_object>                                             PolicyMap;

    xml_bundle() {}
    virtual ~xml_bundle() {}
    xml_bundle(const xml_bundle& other)
    {
        _policy_map = other._policy_map;
        _component_map = other._component_map;
        _condition_map = other._condition_map;
        _obligation_map = other._obligation_map;
        _user_group_map = other._user_group_map;
        _unknown_user_group_info = other._unknown_user_group_info;
    }

    xml_bundle& operator = (const xml_bundle& other)
    {
        if (this != &other) {
            _policy_map = other._policy_map;
            _component_map = other._component_map;
            _condition_map = other._condition_map;
            _obligation_map = other._obligation_map;
            _user_group_map = other._user_group_map;
            _unknown_user_group_info = other._unknown_user_group_info;
        }

        return *this;
    }
    

    void get_user_groups(rapidxml::xml_node<wchar_t>* root)
    {
        rapidxml::xml_node<wchar_t>* user_node = root->first_node(L"USERMAP", 0, false);
        while (user_node) {

            std::wstring user_sid;
            std::wstring user_context;
            std::wstring user_group_list;
            std::vector<std::wstring> user_groups;

            rapidxml::xml_node<wchar_t>* current_node = user_node;
            user_node = user_node->next_sibling(L"USERMAP", 0, false);
            rapidxml::xml_attribute<wchar_t>* attr_sid = current_node->first_attribute(L"id", 0, false);
            if (NULL == attr_sid)
                continue;
            user_sid = attr_sid->value();
            rapidxml::xml_attribute<wchar_t>* attr_context = current_node->first_attribute(L"context", 0, false);
            if (NULL == attr_sid)
                continue;
            user_context = attr_context->value();

            user_group_list = current_node->value();
            nudf::string::Split<wchar_t>(user_group_list, L',', user_groups);

            if (0 == _wcsicmp(user_sid.c_str(), L"{UNKNOWN-USER}")) {
                _unknown_user_group_info.first = user_context;
                _unknown_user_group_info.second = user_groups;
                continue;
            }

            // is sid a real sid?
            if (!boost::algorithm::istarts_with(user_sid, L"S-1-5-")) {
                continue;
            }

            // add
            std::transform(user_sid.begin(), user_sid.end(), user_sid.begin(), toupper);
            _user_group_map[user_sid] = std::pair<std::wstring/*context*/, std::vector<std::wstring/*group_id*/>>(user_context, user_groups);
        }
    }

    void get_components(rapidxml::xml_node<wchar_t>* root)
    {
        std::wstring    root_name = root->name();
        std::wstring    comp_name;
        std::wstring    comp_prefix;

        if (0 == _wcsicmp(root_name.c_str(), L"APPLICATIONS")) {
            comp_name = L"APPLICATION";
            comp_prefix = L"application.";
        }
        else if (0 == _wcsicmp(root_name.c_str(), L"USERS")) {
            comp_name = L"USER";
            comp_prefix = L"user.";
        }
        else if (0 == _wcsicmp(root_name.c_str(), L"LOCATIONS")) {
            comp_name = L"LOCATION";
            comp_prefix = L"host.";
        }
        else if (0 == _wcsicmp(root_name.c_str(), L"RESOURCES")) {
            comp_name = L"RESOURCE";
            comp_prefix = L"resource.fso.";
        }
        else if (0 == _wcsicmp(root_name.c_str(), L"ENVS")) {
            comp_name = L"ENV";
            comp_prefix = L"environment.";
        }
        else {
            // something is wrong
            assert(false);
            return;
        }

        rapidxml::xml_node<wchar_t>* comp_node = root->first_node(comp_name.c_str(), 0, false);
        while (comp_node) {

            std::wstring user_sid;
            std::wstring user_context;
            std::wstring user_group_list;
            std::vector<std::wstring> user_groups;

            rapidxml::xml_node<wchar_t>* current_node = comp_node;
            comp_node = comp_node->next_sibling(comp_name.c_str(), 0, false);

            // get id
            std::wstring comp_id;
            std::wstring full_comp_id;
            rapidxml::xml_attribute<wchar_t>* attr_id = current_node->first_attribute(L"id", 0, false);
            if (NULL == attr_id)
                continue;
            comp_id = attr_id->value();
            full_comp_id = comp_prefix + comp_id;

            // Get sub-properties
            std::vector<std::wstring> comp_props;
            rapidxml::xml_node<wchar_t>* prop_node = current_node->first_node(L"PROPERTY", 0, false);
            while (prop_node) {

                std::wstring prop_name;
                std::wstring prop_value;
                std::wstring prop_op;

                rapidxml::xml_node<wchar_t>* current_prop = prop_node;
                prop_node = prop_node->next_sibling(L"PROPERTY", 0, false);

                rapidxml::xml_attribute<wchar_t>* attr_name = current_prop->first_attribute(L"name", 0, false);
                if (NULL == attr_name)
                    continue;
                rapidxml::xml_attribute<wchar_t>* attr_value = current_prop->first_attribute(L"value", 0, false);
                if (NULL == attr_value)
                    continue;

                prop_name = attr_name->value();
                if (!boost::algorithm::istarts_with(prop_name, comp_prefix)) {
                    prop_name = comp_prefix + prop_name;
                }

                prop_value = attr_value->value() ? attr_value->value() : L"";

                rapidxml::xml_attribute<wchar_t>* attr_method = current_prop->first_attribute(L"method", 0, false);
                if (NULL == attr_method) {
                    prop_op = L"EQ";
                }
                else {
                    prop_op = attr_method->value() ? attr_method->value() : L"EQ";
                }

                std::shared_ptr<details::condition> sp(new details::condition(prop_name, prop_value, prop_op));
                if (sp != NULL && !sp->empty() && !sp->hash().empty()) {
                    auto pos = _condition_map.find(sp->hash());
                    if (pos == _condition_map.end()) {
                        // not exist
                        _condition_map[sp->hash()] = sp;
                    }
                    // add this condition
                    comp_props.push_back(sp->hash());
                }
            }

            // If it has any properties, add it to map
            _component_map[full_comp_id] = comp_props;
            if (comp_props.empty()) {
                LOGWARN(0, L"Component <%s> (id = %s) doesn't have any properties.", current_node->name(), comp_id.c_str());
            }
        }
    }

    void load_policies(rapidxml::xml_node<wchar_t>* root)
    {
        rapidxml::xml_node<wchar_t>* policy_node = root->first_node(L"POLICY", 0, false);
        while (policy_node) {

            xml_policy_object   xmlpolicy;

            rapidxml::xml_node<wchar_t>* current_policy = policy_node;
            policy_node = policy_node->next_sibling(L"POLICY", 0, false);

            rapidxml::xml_attribute<wchar_t>* attr_id = current_policy->first_attribute(L"id", 0, false);
            if (NULL == attr_id)
                continue;
            xmlpolicy._id = attr_id->value();

            rapidxml::xml_attribute<wchar_t>* attr_name = current_policy->first_attribute(L"name", 0, false);
            if (NULL == attr_name)
                continue;
            xmlpolicy._name = attr_name->value();

            rapidxml::xml_attribute<wchar_t>* attr_usergroup = current_policy->first_attribute(L"usergroup", 0, false);
            if (NULL != attr_usergroup) {
                // a new condition is ==> "user.group = id"
                xmlpolicy._user_group = attr_usergroup->value();
                std::shared_ptr<details::condition> sp(new details::condition(L"user.group", xmlpolicy._user_group, L"="));
                if (sp != NULL && !sp->empty() && !sp->hash().empty()) {
                    auto pos = _condition_map.find(sp->hash());
                    if (pos == _condition_map.end()) {
                        // not exist
                        _condition_map[sp->hash()] = sp;
                    }
                    // add this condition
                    xmlpolicy._include_user_comp.push_back(std::vector<std::wstring>({ sp->hash() }));
                }
            }

            rapidxml::xml_attribute<wchar_t>* attr_rights = current_policy->first_attribute(L"rights", 0, false);
            if (NULL != attr_rights) {
                std::wstring wsrights(attr_rights->value());
                nudf::string::Split(wsrights, L',', xmlpolicy._rights);
            }
        
            rapidxml::xml_node<wchar_t>* child_node = current_policy->first_node();
            while (child_node) {

                rapidxml::xml_node<wchar_t>* current_child = child_node;
                child_node = child_node->next_sibling();

                if (current_child->type() != rapidxml::node_type::node_element)
                    continue;

                if (0 == _wcsicmp(current_child->name(), L"CONDITION")) {
                    // Get condition
                    std::wstring comp_exclude;
                    std::wstring comp_type;
                    std::wstring comp_list_str;
                    std::vector<std::wstring>   comp_list;

                    rapidxml::xml_attribute<wchar_t>* attr_cond_type = current_child->first_attribute(L"type", 0, false);
                    if (NULL == attr_cond_type)
                        continue;
                    comp_type = attr_cond_type->value();

                    rapidxml::xml_attribute<wchar_t>* attr_cond_exclude = current_child->first_attribute(L"exclude", 0, false);
                    if (NULL == attr_cond_exclude) {
                        comp_exclude = L"false";
                    }
                    else {
                        comp_exclude = attr_cond_exclude->value();

                    }

                    comp_list_str = current_child->value();
                    if (comp_list_str.empty()) {
                        continue;
                    }
                    std::transform(comp_list_str.begin(), comp_list_str.end(), comp_list_str.begin(), tolower);
                    nudf::string::Split<wchar_t>(comp_list_str, L',', comp_list);

                    std::vector<std::wstring>   comp_hash_list;
                    bool exclude = (0 == _wcsicmp(comp_exclude.c_str(), L"true"));
                    if (0 == _wcsicmp(comp_type.c_str(), L"app")) {
                        std::for_each(comp_list.begin(), comp_list.end(), [&](const std::wstring& id) {
                            std::wstring full_id = L"application." + id;
                            auto pos = _component_map.find(full_id);
                            if (pos != _component_map.end()) {
                                // good, we have condition list for policy
                                exclude ? xmlpolicy._exclude_app_comp.push_back((*pos).second) : xmlpolicy._include_app_comp.push_back((*pos).second);
                            }
                        });
                    }
                    else if (0 == _wcsicmp(comp_type.c_str(), L"usr")) {
                        std::for_each(comp_list.begin(), comp_list.end(), [&](const std::wstring& id) {
                            std::wstring full_id = L"user." + id;
                            auto pos = _component_map.find(full_id);
                            if (pos != _component_map.end()) {
                                // good, we have condition list for policy
                                exclude ? xmlpolicy._exclude_user_comp.push_back((*pos).second) : xmlpolicy._include_user_comp.push_back((*pos).second);
                            }
                        });
                    }
                    else if (0 == _wcsicmp(comp_type.c_str(), L"res")) {
                        std::for_each(comp_list.begin(), comp_list.end(), [&](const std::wstring& id) {
                            std::wstring full_id = L"resource.fso." + id;
                            auto pos = _component_map.find(full_id);
                            if (pos != _component_map.end()) {
                                // good, we have condition list for policy
                                exclude ? xmlpolicy._exclude_res_comp.push_back((*pos).second) : xmlpolicy._include_res_comp.push_back((*pos).second);
                            }
                        });
                    }
                    else if (0 == _wcsicmp(comp_type.c_str(), L"loc")) {
                        std::for_each(comp_list.begin(), comp_list.end(), [&](const std::wstring& id) {
                            std::wstring full_id = L"host." + id;
                            auto pos = _component_map.find(full_id);
                            if (pos != _component_map.end()) {
                                // good, we have condition list for policy
                                exclude ? xmlpolicy._exclude_host_comp.push_back((*pos).second) : xmlpolicy._include_host_comp.push_back((*pos).second);
                            }
                        });
                    }
                    else if (0 == _wcsicmp(comp_type.c_str(), L"env")) {
                        std::for_each(comp_list.begin(), comp_list.end(), [&](const std::wstring& id) {
                            std::wstring full_id = L"environment." + id;
                            auto pos = _component_map.find(full_id);
                            if (pos != _component_map.end()) {
                                // good, we have condition list for policy
                                exclude ? xmlpolicy._exclude_env_comp.push_back((*pos).second) : xmlpolicy._include_env_comp.push_back((*pos).second);
                            }
                        });
                    }
                    else {
                        // unknown type
                        continue;
                    }
                }
                else if (0 == _wcsicmp(current_child->name(), L"OBLIGATION")) {
                    // Get obligation
                    std::wstring ob_name;
                    std::map<std::wstring, std::wstring> ob_parameters;
                    
                    rapidxml::xml_attribute<wchar_t>* attr_ob_name = current_child->first_attribute(L"name", 0, false);
                    if (NULL == attr_ob_name)
                        continue;
                    ob_name = attr_ob_name->value();
                    std::transform(ob_name.begin(), ob_name.end(), ob_name.begin(), tolower);

                    rapidxml::xml_node<wchar_t>* param_node = current_child->first_node();
                    while (param_node) {
                        rapidxml::xml_node<wchar_t>* current_param = param_node;
                        param_node = param_node->next_sibling();
                        if (current_param->type() != rapidxml::node_type::node_element)
                            continue;
                        rapidxml::xml_attribute<wchar_t>* attr_obparam_name = current_param->first_attribute(L"name", 0, false);
                        rapidxml::xml_attribute<wchar_t>* attr_obparam_value = current_param->first_attribute(L"value", 0, false);
                        if (NULL == attr_obparam_name || NULL == attr_obparam_value)
                            continue;
                        if (NULL == attr_obparam_name->value() || NULL == attr_obparam_value->value())
                            continue;
                        std::wstring  param_name = attr_obparam_name->value();
                        std::transform(param_name.begin(), param_name.end(), param_name.begin(), tolower);
                        std::wstring  param_value = attr_obparam_value->value();
                        if (!param_name.empty() && !param_value.empty()) {
                            ob_parameters[param_name] = param_value;
                        }
                    }

                    // create obligation object
                    std::shared_ptr<obligation> sp_ob = std::shared_ptr<obligation>(new obligation(ob_name, ob_parameters));
                    // is this obligation already exist?
                    auto pos = _obligation_map.find(sp_ob->hash());
                    if (pos == _obligation_map.end()) {
                        // not exist, add a new one
                        _obligation_map[sp_ob->hash()] = sp_ob;
                    }
                    xmlpolicy._obligations.push_back(sp_ob->hash());
                }
                else {
                    // Unknown
                    ; // NOTHING
                }
            }

            _policy_map[xmlpolicy._id] = xmlpolicy;
        }
    }

private:
    PolicyMap       _policy_map;
    ComponentMap    _component_map;
    ConditionMap    _condition_map;
    ObligationMap   _obligation_map;
    UserGroupMap    _user_group_map;
    UserGroupInfo   _unknown_user_group_info;
    
    friend class policy_bundle::xml_parser;
};
//
//  class policy_bundle::xml_parser
//
policy_bundle::xml_parser::xml_parser() : policy_bundle::parser()
{
}

policy_bundle::xml_parser::~xml_parser()
{
}

policy_bundle policy_bundle::xml_parser::parse(const std::string& s /*utf8*/)
{
    std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
    return parse(ws);
}

policy_bundle policy_bundle::xml_parser::parse(const std::wstring& ws /*utf16*/)
{
    policy_bundle   bundle;
    xml_bundle      _xml_bundle;

    try {

        std::wstring            time_stamp;

        rapidxml::xml_document<wchar_t> doc;
        std::vector<wchar_t> buf(ws.begin(), ws.end());
        buf.push_back(0);
        doc.parse<0>(buf.data());

        rapidxml::xml_node<wchar_t>* root = doc.first_node(L"POLICYBUNDLE", 0, false);
        if (root == NULL) {
            throw std::exception("empty xml");
        }

        rapidxml::xml_attribute<wchar_t>* attr_time = root->first_attribute(L"timestamp", 0, false);
        if (attr_time == NULL) {
            throw std::exception("timestamp not exists");
        }
        time_stamp = attr_time->value();


        // get policy node
        rapidxml::xml_node<wchar_t>* policyset_node = root->first_node(L"POLICYSET", 0, false);
        if (NULL == policyset_node) {
            throw std::exception("<POLICYSET> not exist");
        }

        // get user group node
        rapidxml::xml_node<wchar_t>* usergroupmap_node = root->first_node(L"USERGROUPMAP", 0, false);
        if (NULL == usergroupmap_node) {
            throw std::exception("<USERGROUPMAP> not exist");
        }

        // get components node
        rapidxml::xml_node<wchar_t>* components_node = root->first_node(L"COMPONENTS", 0, false);
        if (NULL == components_node) {
            throw std::exception("<COMPONENTS> not exist");
        }

        // load all component sub nodes
        rapidxml::xml_node<wchar_t>* props_user_node = components_node->first_node(L"USERS", 0, false);
        if (NULL == props_user_node) {
            throw std::exception("<USERS> not exist");
        }
        rapidxml::xml_node<wchar_t>* props_app_node = components_node->first_node(L"APPLICATIONS", 0, false);
        if (NULL == props_app_node) {
            throw std::exception("<APPLICATIONS> not exist");
        }
        rapidxml::xml_node<wchar_t>* props_res_node = components_node->first_node(L"RESOURCES", 0, false);
        if (NULL == props_res_node) {
            throw std::exception("<RESOURCES> not exist");
        }
        rapidxml::xml_node<wchar_t>* props_loc_node = components_node->first_node(L"LOCATIONS", 0, false);
        if (NULL == props_loc_node) {
            throw std::exception("<LOCATIONS> not exist");
        }
        rapidxml::xml_node<wchar_t>* props_env_node = components_node->first_node(L"ENVS", 0, false);
        if (NULL == props_env_node) {
            throw std::exception("<ENVS> not exist");
        }

        // load user group map
        _xml_bundle.get_user_groups(usergroupmap_node);
        // load user properties
        _xml_bundle.get_components(props_user_node);
        _xml_bundle.get_components(props_app_node);
        _xml_bundle.get_components(props_res_node);
        _xml_bundle.get_components(props_loc_node);
        _xml_bundle.get_components(props_env_node);

        // load policy
        _xml_bundle.load_policies(policyset_node);

        // Good, now we have all the data
        // 1. build policy bundle object
        bundle._time = time_stamp;
        bundle._obligation_map = _xml_bundle._obligation_map;
        bundle._condition_map  = _xml_bundle._condition_map;
        bundle._user_group_map = _xml_bundle._user_group_map;
        bundle._unknown_user_group = _xml_bundle._unknown_user_group_info;
        std::for_each(_xml_bundle._policy_map.begin(), _xml_bundle._policy_map.end(), [&](const std::pair<std::wstring,xml_policy_object>& policy_object) {
            std::shared_ptr<policy> sp = std::shared_ptr<policy>(new policy(policy_object.second._id, policy_object.second._name, policy_object.second._rights));
            if (sp == NULL) {
                return;
            }
            // Initialize this policy object
            //   --> Build subject expression
            add_policy_conditions(bundle, sp->subject_expr(), policy_object.second._exclude_user_comp, true);
            add_policy_conditions(bundle, sp->subject_expr(), policy_object.second._include_user_comp, false);
            add_policy_conditions(bundle, sp->subject_expr(), policy_object.second._exclude_app_comp, true);
            add_policy_conditions(bundle, sp->subject_expr(), policy_object.second._include_app_comp, false);
            add_policy_conditions(bundle, sp->subject_expr(), policy_object.second._exclude_host_comp, true);
            add_policy_conditions(bundle, sp->subject_expr(), policy_object.second._include_host_comp, false);
            //   --> Build resource expression
            add_policy_conditions(bundle, sp->resource_expr(), policy_object.second._exclude_res_comp, true);
            add_policy_conditions(bundle, sp->resource_expr(), policy_object.second._include_res_comp, false);
            //   --> Build environment expression
            add_policy_conditions(bundle, sp->env_expr(), policy_object.second._exclude_env_comp, true);
            add_policy_conditions(bundle, sp->env_expr(), policy_object.second._include_env_comp, false);

            // add obligations
            std::for_each(policy_object.second._obligations.begin(), policy_object.second._obligations.end(), [&](const std::wstring& hash) {
                auto pos = bundle.obligation_map().find(hash);
                if (pos != bundle.obligation_map().end()) {
                    sp->obligations().push_back((*pos).second);
                }
            });

            // add this policy
            bundle.policy_map()[sp->id()] = sp;
        });
        // 2. save to local json file
    }
    catch (const nudf::CException& e) {
        throw e;
    }
    catch (const std::exception& e) {
        throw e;
    }
    catch (structured_exception& e) {
        std::wstring exception_msg = e.exception_message();
        LOGASSERT(exception_msg.c_str());
        e.dump();
        e.raise();
    }

    return std::move(bundle);
}

void policy_bundle::xml_parser::add_policy_conditions(policy_bundle& bundle, details::expression* expr, const std::vector<std::vector<std::wstring>>& comp_map, bool exclude)
{
    if (comp_map.empty()) {
        return;
    }

    if (expr->root() == NULL) {
        expr->attach(std::shared_ptr<details::expression::expression_node>(
                new details::expression::expression_node(NULL, details::expression::expression_node::op_and, false)
            ));
    }

    assert(NULL != expr->root());

    std::shared_ptr<details::expression::expression_node> node = std::shared_ptr<details::expression::expression_node>(
        new details::expression::expression_node(expr->root().get(), details::expression::expression_node::op_or, exclude)
        );
    if (NULL == node) {
        throw std::exception("fail to alloc expression_node");
    }

    std::for_each(comp_map.begin(), comp_map.end(), [&](const std::vector<std::wstring>& cond_hash_list) {
        if (cond_hash_list.empty()) {
            return;
        }
        std::shared_ptr<details::expression::expression_node> sub_node = std::shared_ptr<details::expression::expression_node>(
            new details::expression::expression_node(node.get(), details::expression::expression_node::op_and, exclude)
            );
        if (NULL == sub_node) {
            throw std::exception("fail to alloc expression_node");
        }

        std::for_each(cond_hash_list.begin(), cond_hash_list.end(), [&](const std::wstring& hash) {
            auto pos = bundle.condition_map().find(hash);
            if (pos != bundle.condition_map().end()) {
                std::shared_ptr<details::expression::condition_node> cond_node = std::shared_ptr<details::expression::condition_node>(
                    new details::expression::condition_node(node.get(), (*pos).second, false)
                    );
                sub_node->children().push_back(cond_node);
            }
        });

        // Add a component
        node->children().push_back(sub_node);
    });

    // Add this node :)
    expr->root()->children().push_back(node);
}

