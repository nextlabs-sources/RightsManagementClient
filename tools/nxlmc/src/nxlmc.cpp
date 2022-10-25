

#include <Windows.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <boost\algorithm\string.hpp>

#include <nudf\string.hpp>
#include <nudf\encoding.hpp>
#include <nudf\shared\nxlfmt.h>
#include <nudf\shared\keydef.h>
#include <nudf\web\json.hpp>
#include <nudf\crypto.hpp>
#include <nudf\nxlfile.hpp>
#include <nudf\shared\nxlfmt.h>

#include "nxlmc.hpp"



using namespace NX;


namespace NX {
class keystore
{
public:
    keystore() {}
    virtual ~keystore() {}

    inline bool empty() const noexcept { return _keys.empty(); }
    inline const std::vector<NX::NXL::nxl_key_pkg> keys() const noexcept { return _keys; }

    bool load_keys()
    {
        bool result = false;

        _rmc_root = find_rmc_root();
        if (_rmc_root.empty()) {
            _rmc_root = get_process_dir();
        }
        if (_rmc_root.empty()) {
            std::cout << "ERROR: configure file directory not found" << std::endl;
            return false;
        }

        std::wstring agent_profile = _rmc_root + L"\\conf\\agent_info.sjs";
        std::wstring key_profile = _rmc_root + L"\\conf\\agent_keys.sjs";

        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(agent_profile.c_str())) {
            std::cout << "ERROR: agent profile not found" << std::endl;
            return false;
        }
        if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(key_profile.c_str())) {
            std::cout << "ERROR: key profile not found" << std::endl;
            return false;
        }

        try {
            NX::secure_mem agent_key = load_agent_key(agent_profile);
            load_key_profile(key_profile, agent_key);
            result = true;
        }
        catch (std::exception& e) {
            result = false;
            std::cout << "ERROR: " << e.what() << std::endl;
        }

        return result;
    }

    std::vector<unsigned char> find_key(const NXRM_KEY_ID* key_id)
    {
        auto pos = std::find_if(_keys.begin(), _keys.end(), [&](const NX::NXL::nxl_key_pkg& pkg) -> bool {
            return (pkg.id().algorithm() == key_id->Algorithm
                    && pkg.id().size() == key_id->IdSize
                    && 0 == memcmp(pkg.id().id(), key_id->Id, key_id->IdSize));
        });
        if (pos == _keys.end()) {
            return std::vector<unsigned char>();
        }        
        return std::vector<unsigned char>((*pos).key().decrypt());
    }

    std::vector<unsigned char> find_key(const NX::NXL::nxl_key_id& key_id)
    {
        auto pos = std::find_if(_keys.begin(), _keys.end(), [&](const NX::NXL::nxl_key_pkg& pkg) -> bool {
            return (pkg.id().algorithm() == key_id.algorithm()
                    && pkg.id().size() == key_id.size()
                    && 0 == memcmp(pkg.id().id(), key_id.id(), key_id.size()));
        });
        if (pos == _keys.end()) {
            return std::vector<unsigned char>();
        }        
        return std::vector<unsigned char>((*pos).key().decrypt());
    }

    void dump_keys()
    {
        int index = 0;
        std::for_each(_keys.begin(), _keys.end(), [&](const NX::NXL::nxl_key_pkg& pkg) {

            std::string key_ring(pkg.id().id(), pkg.id().id() + 8);
            std::string key_hash = nudf::string::FromBytes<char>(pkg.id().id() + 8, 32);
            unsigned long time = *((unsigned long*)(pkg.id().id() + 40));
            nudf::time::CTime ct;
            ct.FromSecondsSince1970Jan1st(time);
            SYSTEMTIME st = { 0 };
            ct.ToSystemTime(&st);
            std::string key_time_hex;
            std::string key_time;
            sprintf_s(nudf::string::tempstr<char>(key_time_hex, 256), 256, "%08X", time);
            sprintf_s(nudf::string::tempstr<char>(key_time, 256), 256, "%04d-%02d-%02dT%02d:%02d:%02dZ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
            std::vector<unsigned char> vkey = pkg.key().decrypt();
            std::string key_value = nudf::string::FromBytes<char>(vkey.data(), (unsigned long)vkey.size());

            std::cout << "Key Package #" << index << std::endl;
            std::cout << "    Algorithm: " << ((pkg.id().algorithm() == NXL_ALGORITHM_AES128) ? "AES 128" : "AES 256") << std::endl;
            std::cout << "    Name: " << key_ring.c_str() << std::endl;
            std::cout << "    Hash: " << key_hash.c_str() << std::endl;
            std::cout << "    Time: " << key_time_hex << " (" << key_time << ")" << std::endl;
            std::cout << "    Key:  " << key_value << std::endl;
        });
    }

protected:
    std::string load_conf(const std::wstring& file, const std::vector<unsigned char>& key)
    {
        std::ifstream fs;

        if (key.size() != 16 && key.size() != 32) {
            throw std::invalid_argument("wrong key size");
        }

        fs.open(file, std::ifstream::binary);
        if (!fs.is_open() || !fs.good()) {
            throw std::exception("fail to open file");
        }

        // get length of file:
        fs.seekg(0, fs.end);
        size_t length = (size_t)fs.tellg();
        fs.seekg(0, fs.beg);

        if (0 == length) {
            return std::string();
        }

        if (0 != length%key.size()) {
            throw std::invalid_argument("file size is not aligned with key size");
        }

        std::vector<char> s;
        s.resize(length + 1, 0);
        fs.read(&s[0], length);
        if (fs.eof() || fs.bad() || fs.fail()) {
            throw std::exception("fail to read all the data");
        }

        nudf::crypto::CAesKeyBlob ak;
        ak.SetKey(&key[0], (ULONG)key.size());
        if (!nudf::crypto::AesDecrypt(ak, &s[0], (ULONG)length, 0)) {
            return std::string();
        }

        return &s[0];
    }
    std::wstring load_agent_id(const std::wstring& agent_profile)
    {
        static const secure_mem key_base(std::vector<unsigned char>({
            0x64, 0x31, 0xBA, 0xF1, 0xE3, 0xC5, 0x24, 0x1F,
            0x64, 0x31, 0xBA, 0xF1, 0xE3, 0xC5, 0x24, 0x1F,
            0x64, 0x31, 0xBA, 0xF1, 0xE3, 0xC5, 0x24, 0x1F,
            0x64, 0x31, 0xBA, 0xF1, 0xE3, 0xC5, 0x24, 0x1F
        }));

        std::wstring agent_id;

        try {
            std::string s = load_conf(agent_profile, key_base.decrypt());
            std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
            NX::web::json::value v = NX::web::json::value::parse(ws);
            agent_id = v[L"agent_id"].as_string();
            std::transform(agent_id.begin(), agent_id.end(), agent_id.begin(), tolower);
        }
        catch (std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            agent_id.clear();
        }

        return agent_id;
    }
    NX::secure_mem load_agent_key(const std::wstring& agent_profile)
    {
        static const std::vector<unsigned char> magic_key({
            0x1C, 0x2B, 0x33, 0x48, 0x56, 0x21, 0x1A, 0xCC,
            0x1C, 0x2B, 0x33, 0x48, 0x56, 0x21, 0x1A, 0xCC,
            0x1C, 0x2B, 0x33, 0x48, 0x56, 0x21, 0x1A, 0xCC,
            0x1C, 0x2B, 0x33, 0x48, 0x56, 0x21, 0x1A, 0xCC
        });

        std::wstring agent_id = load_agent_id(agent_profile);

        if (agent_id.empty()) {
            throw std::exception("agent id not exist");
        }

        std::vector<unsigned char> hash;
        if (!nudf::crypto::ToSha256(agent_id.c_str(), (unsigned long)(agent_id.length()*sizeof(wchar_t)), hash)) {
            throw std::exception("fail to calculate hash");
        }

        assert(32 == hash.size());
        nudf::crypto::CAesKeyBlob keyblob;
        keyblob.SetKey(&magic_key[0], (unsigned long)magic_key.size());
        if (!nudf::crypto::AesEncrypt(keyblob, &hash[0], (unsigned long)hash.size(), 0)) {
            throw std::exception("fail to encrypt hash");
        }

        return NX::secure_mem(hash);
    }
    void load_key(NX::NXL::nxl_key_pkg& pkg, const std::wstring& ring, const std::wstring& id, const std::wstring& key, const std::wstring& time)
    {
        std::string        vring(ring.begin(), ring.end());
        std::vector<UCHAR> vid;
        std::vector<UCHAR> vkey;
        FILETIME           ftime = { 0, 0 };

        long    key_time = 0;
        std::vector<unsigned char> key_id;

        if (!nudf::util::encoding::Base64Decode<wchar_t>(id, vid) || vid.size() != 32) {
            return;
        }
        if (vid.size() != 32) {
            return;
        }
        if (!nudf::util::encoding::Base64Decode<wchar_t>(key, vkey)) {
            return;
        }
        // Only support AES 256 in this release
        if (32 != vkey.size() && 16 != vkey.size()) {
            return;
        }
        if (!nudf::string::ToSystemTime<wchar_t>(time, &ftime, NULL)) {
            return;
        }

        nudf::time::CTime timestamp(&ftime);
        key_time = (long)timestamp.ToSecondsSince1970Jan1st();

        // good, now we have all the data, pu them to blob
        key_id.resize(8 + vid.size() + 4, 0);
        memcpy(&key_id[0], vring.c_str(), min(8, vring.length()));
        memcpy(&key_id[8], &vid[0], vid.size());
        memcpy(&key_id[8 + vid.size()], &key_time, sizeof(long));

        NX::NXL::nxl_key_id nk_id((32 == vkey.size()) ? NXRM_ALGORITHM_AES256 : NXRM_ALGORITHM_AES128, key_id);
        pkg = NX::NXL::nxl_key_pkg(nk_id, vkey);
    }
    void load_key_profile(const std::wstring& key_profile, const NX::secure_mem& agent_key)
    {
        std::string s = load_conf(key_profile, agent_key.decrypt());
        std::wstring ws = NX::utility::conversions::utf8_to_utf16(s);
        NX::web::json::value v = NX::web::json::value::parse(ws);
        std::for_each(v.as_array().begin(), v.as_array().end(), [&](const NX::web::json::value& key_obj) {
            try {
                NX::NXL::nxl_key_pkg pkg;
                load_key(pkg, key_obj.at(L"ring").as_string(), key_obj.at(L"id").as_string(), key_obj.at(L"key").as_string(), key_obj.at(L"time").as_string());
                if (!pkg.empty()) {
                    _keys.push_back(pkg);
                }
            }
            catch (std::exception& e) {
                UNREFERENCED_PARAMETER(e);
            }
        });
    }
    std::wstring find_rmc_root()
    {
        REGSAM  samDesired = KEY_READ;
        HKEY    hKey = NULL;
        DWORD   i = 0;
        WCHAR   wzGuid[MAX_PATH] = { 0 };
        std::wstring install_dir;
        
#ifdef _WIN64
        samDesired |= KEY_WOW64_64KEY;
#else
        samDesired |= KEY_WOW64_32KEY;
#endif
        if (ERROR_SUCCESS != ::RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 0, samDesired, &hKey)) {
            return std::wstring();
        }

        while (0 == RegEnumKeyW(hKey, i++, wzGuid, MAX_PATH - 1)) {

            HKEY    hProgramKey = NULL;
            WCHAR   wzData[1024] = { 0 };
            DWORD   cbData = 2048;
            DWORD   dwType = 0;

            if (ERROR_SUCCESS != ::RegOpenKeyExW(hKey, wzGuid, 0, samDesired, &hProgramKey) || L'\0' == wzGuid[0]) {
                continue;
            }

            // Get information from subkey
            if (ERROR_SUCCESS != RegQueryValueExW(hProgramKey, L"DisplayName", NULL, &dwType, (LPBYTE)wzData, &cbData) || REG_SZ != dwType || L'\0' == wzData[0]) {
                ::RegCloseKey(hProgramKey);
                hProgramKey = NULL;
                continue;
            }

            if (0 != _wcsicmp(wzData, L"NextLabs Rights Management")) {
                ::RegCloseKey(hProgramKey);
                hProgramKey = NULL;
                continue;
            }

            cbData = 2048;
            LONG lRet = RegQueryValueExW(hProgramKey, L"InstallLocation", NULL, &dwType, (LPBYTE)wzData, &cbData);
            if (ERROR_SUCCESS == lRet && REG_SZ == dwType && L'\0' != wzData[0]) {
                install_dir = wzData;
                // if the string ends with L'\\', remove it
                if (boost::algorithm::ends_with(install_dir, L"\\")) {
                    install_dir = install_dir.substr(0, install_dir.length() - 1);
                }
            }

            // Free key
            ::RegCloseKey(hProgramKey);
            hProgramKey = NULL;
            break;
        }

        // Free key
        ::RegCloseKey(hKey);
        hKey = NULL;
        return install_dir;
    }
    std::wstring get_process_dir()
    {
        std::wstring dir;
        std::wstring file;
        GetModuleFileNameW(NULL, nudf::string::tempstr<wchar_t>(file, MAX_PATH), MAX_PATH);
        auto pos = file.find_last_of(L'\\');
        if (pos != std::wstring::npos) {
            dir = file.substr(0, pos);
        }
        if (dir.empty()) {
            GetCurrentDirectoryW(MAX_PATH, nudf::string::tempstr<wchar_t>(dir, MAX_PATH));
        }
        return dir;
    }

private:
    std::wstring    _rmc_root;
    std::vector<NX::NXL::nxl_key_pkg> _keys;
};
}

keystore    _keystore;

int wmain(int argc, wchar_t** argv)
{
    nxlmc _mc;

    // initialize crypto library
    if (!nudf::crypto::Initialize()) {
        std::wcout << L"ERROR: fail to initialize crypto library" << std::endl;
        return -1;
    }
    
    if (!_keystore.load_keys()) {
        return -1;
    }
    if (_keystore.empty()) {
        std::wcout << L"ERROR: empty key store" << std::endl;
        return -1;
    }

    if (argc == 1) {
        // no parameters
        // enter standby mode
        _mc.run();
    }
    else {
        const wchar_t* command_line = GetCommandLineW();
        const wchar_t* pos = wcsstr(command_line, L".exe");
        if (pos == NULL) {
            std::wcout << L"ERROR: Invalid command (" << command_line << L")" << std::endl;
            return -1;
        }

        pos = wcschr(pos, L' ');
        if (pos == NULL) {
            std::wcout << L"ERROR: Invalid command (" << command_line << L")" << std::endl;
            return -1;
        }

        std::wstring command_str(pos + 1);
        boost::algorithm::trim(command_str);
        commandline cmdl(command_str);
        _mc.exec(cmdl);
    }

    return 0;
}





//
//  class commandline
//
commandline::commandline()
{
}

commandline::~commandline()
{
}

commandline::commandline(const std::wstring& line)
{
    std::wstring l(line);

    boost::algorithm::trim(l);
    if (l.empty()) {
        return;
    }

    // get command
    _command = get_next_part(l, L' ');

    while (!l.empty()) {
        std::wstring switcher = get_next_part(l, L' ');
        std::transform(switcher.begin(), switcher.end(), switcher.begin(), tolower);
        assert(!switcher.empty());

        // add this switcher
        std::vector<std::wstring>& item = _parameters[switcher];
        while (!l.empty()) {
            if (L'-' == l[0]) {
                break;
            }
            std::wstring value = get_next_part(l, L' ');
            if (!value.empty()) {
                item.push_back(value);
            }
        }
    }
}

std::wstring::size_type find_close_quote(std::wstring& line, std::wstring::size_type start_pos)
{
    std::wstring::size_type pos = start_pos;

    do {

        pos = line.find_first_of(L"\\\"", pos);
        if (pos == std::wstring::npos) {
            break;
        }
        if (line[pos] == L'\"') {
            break;
        }
        else {
            assert(line[pos] == L'\\');
            // ignore next character
            pos += 2;
            continue;
        }
    } while (true);

    return pos;
}

std::wstring commandline::get_next_part(std::wstring& line, const wchar_t sp)
{
    std::wstring part;
    size_t pos = 0;
    const std::wstring find_str({sp, L'\"'});
    
    do {
        
        pos = line.find_first_of(find_str.c_str(), pos);
        if (pos == std::wstring::npos) {
            part = line;
            line.clear();
            return part;
        }

        if (sp == line[pos]) {
            // good, there is no quote
            part = line.substr(0, pos);
            line = line.substr(pos + 1);
            boost::algorithm::trim_left(line);
            return part;
        }

        pos = find_close_quote(line, ++pos);
        if (pos == std::wstring::npos) {
            throw std::exception("bad format");
        }

        pos++;

    } while (pos < line.length());

    part = line;
    line.clear();
    return part;
}

commandline& commandline::operator = (const commandline& line)
{
    return *this;
}


//
//  class nxlmc
//
nxlmc::nxlmc()
{
}

nxlmc::~nxlmc()
{
}

void nxlmc::usage(const std::wstring& command)
{
    if (command.empty()) {
        std::cout << "nxlmc.exe <command> [parameters ...]" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "    quit/exit:  quit from this app" << std::endl;
        std::cout << "    dump-keys:  display all the keys belong to this agent" << std::endl;
        std::cout << "    create:  create a new NXL file" << std::endl;
        std::cout << "    decrypt: decrypt an NXL file" << std::endl;
        std::cout << "    verify: verify an existing NXL file" << std::endl;
        std::cout << "    show: show an existing NXL file's information" << std::endl;
        std::cout << "    section-resize: resize an existing section" << std::endl;
        std::cout << "    section-add: add a new section" << std::endl;
        std::cout << "    section-remove: remove an existing section" << std::endl;
        std::cout << "    attribute-add: add a new NXL attribute" << std::endl;
        std::cout << "    attribute-remove: remove an existing NXL attribute" << std::endl;
        std::cout << "    template-add: set descationary rights template" << std::endl;
        std::cout << "    template-remove: remove descationary rights template" << std::endl;
        std::cout << "    tag-add: add a new NXL attribute" << std::endl;
        std::cout << "    tag-remove: remove an existing NXL attribute" << std::endl;
        std::cout << "Use \"help <command>\" to view detail usage" << std::endl;
        return;
    }

    if (command == L"create") {
        std::cout << "create: create a new NXL file" << std::endl;
        std::cout << "    create [-i <file>] [-o <file>] [-s <section-name>:<section-size>]" << std::endl;
    }
    else if (command == L"decrypt") {
        std::cout << "decrypt: decrypt an NXL file" << std::endl;
        std::cout << "    decrypt -i <file> [-o <file>]" << std::endl;
    }
    else if (command == L"verify") {
        std::cout << "verify: verify an existing NXL file" << std::endl;
        std::cout << "    verify -i <file>" << std::endl;
    }
    else if (command == L"show") {
        std::cout << "show: show an existing NXL file's information" << std::endl;
        std::cout << "    show -i <file> [-all] [-header] [-section <name>]" << std::endl;
    }
    else if (command == L"section-resize") {
        std::cout << "section-resize: resize an existing section" << std::endl;
        std::cout << "    section-resize -i <file> -n <name> -s <size>" << std::endl;
    }
    else if (command == L"section-add") {
        std::cout << "section-add: add a new section" << std::endl;
        std::cout << "    section-add -i <file> -n <name> [-s <size>]" << std::endl;
    }
    else if (command == L"section-remove") {
        std::cout << "section-remove: remove an existing section" << std::endl;
        std::cout << "    section-remove -i <file> -n <name> [-n <name>, ...]" << std::endl;
    }
    else if (command == L"attribute-add") {
        std::cout << "attribute-add: add a new NXL attribute" << std::endl;
        std::cout << "    attribute-add -i <file> -a <name=value> [-a <name=value>, ...]" << std::endl;
    }
    else if (command == L"attribute-remove") {
        std::cout << "attribute-remove: remove an existing NXL attribute" << std::endl;
        std::cout << "    attribute-remove -i <file> -t <name> [-t <name>, ...]" << std::endl;
    }
    else if (command == L"template-add") {
        std::cout << "template-add: set descationary rights template" << std::endl;
        std::cout << "    template-add -i <file> -t <name=value> [-t <name=value>, ...]" << std::endl;
    }
    else if (command == L"template-remove") {
        std::cout << "template-remove: set descationary rights template" << std::endl;
        std::cout << "    template-remove -i <file> -t <name> [-t <name>, ...]" << std::endl;
    }
    else if (command == L"tag-add") {
        std::cout << "tag-add: add a new NXL tag" << std::endl;
        std::cout << "    tag-add -i <file> -t <name=value> [-t <name=value>, ...]" << std::endl;
    }
    else if (command == L"tag-remove") {
        std::cout << "tag-remove: remove an existing NXL tag" << std::endl;
        std::cout << "    tag-add -i <file> -t <name[=value]> [-t <name[=value]>, ...]" << std::endl;
        std::cout << "       \"-t name\" will remove all the tags with specified name" << std::endl;
        std::cout << "       \"-t name=value\" will only remove the tag with specified name and value" << std::endl;
    }
    else if (command == L"quit" || command == L"exit") {
        std::cout << "quit this application" << std::endl;
    }
    else {
        std::cout << "Unknown command" << std::endl;
    }
}

void nxlmc::run()
{
    do {

        std::wstring command_str;
        std::cout << "> ";
        std::wcin.getline((wchar_t*)nudf::string::tempstr<wchar_t>(command_str, 8192), 8192);

        boost::algorithm::trim(command_str);
        commandline cmdl(command_str);
        if (cmdl.command() == L"quit" || cmdl.command() == L"exit") {
            break;
        }

        // execute command
        exec(command_str);

    } while (true);
}

int nxlmc::exec(const commandline& cmdl)
{
    // get command
    if (cmdl.command().empty()) {
        return 0;
    }

    if (cmdl.command() == L"quit" || cmdl.command() == L"exit") {
        return 0;
    }
    else if (cmdl.command() == L"create") {
        return create(cmdl.parameters());
    }
    else if (cmdl.command() == L"decrypt") {
        return decrypt(cmdl.parameters());
    }
    else if (cmdl.command() == L"verify") {
        return verify(cmdl.parameters());
    }
    else if (cmdl.command() == L"show") {
        return show(cmdl.parameters());
    }
    else if (cmdl.command() == L"section-resize") {
        return section_resize(cmdl.parameters());
    }
    else if (cmdl.command() == L"section-add") {
        return section_add(cmdl.parameters());
    }
    else if (cmdl.command() == L"section-remove") {
        return section_remove(cmdl.parameters());
    }
    else if (cmdl.command() == L"attribute-add") {
        return attribute_add(cmdl.parameters());
    }
    else if (cmdl.command() == L"attribute-remove") {
        return attribute_remove(cmdl.parameters());
    }
    else if (cmdl.command() == L"template-add") {
        return template_add(cmdl.parameters());
    }
    else if (cmdl.command() == L"template-remove") {
        return template_remove(cmdl.parameters());
    }
    else if (cmdl.command() == L"template-set-remote") {
        return template_set_remote(cmdl.parameters());
    }
    else if (cmdl.command() == L"tag-add") {
        return tag_add(cmdl.parameters());
    }
    else if (cmdl.command() == L"tag-remove") {
        return tag_remove(cmdl.parameters());
    }
    else if (cmdl.command() == L"dump-keys") {
        _keystore.dump_keys();
        return 0;
    }
    else if (cmdl.command() == L"help") {
        std::wstring help_targets;
        if (!cmdl.parameters().empty()) {
            help_targets = (*cmdl.parameters().begin()).first;
        }
        usage(help_targets);
    }
    else {
        std::wcout << L"ERROR: Invalid command (" << cmdl.command() << L")" << std::endl;
        std::wcout << L"NXLMC support following commands (use \"nxlmc.exe help <command>\" to get more information)" << std::endl;
        std::wcout << L"    quit/exit:  quit from this app" << std::endl;
        std::wcout << L"    dump-keys:  display all the keys belong to this agent" << std::endl;
        std::wcout << L"    create:  create a new NXL file" << std::endl;
        std::wcout << L"    decrypt: decrypt an NXL file" << std::endl;
        std::wcout << L"    verify: verify an existing NXL file" << std::endl;
        std::wcout << L"    show: show an existing NXL file's information" << std::endl;
        std::wcout << L"    section-resize: resize an existing section" << std::endl;
        std::wcout << L"    section-add: add a new section" << std::endl;
        std::wcout << L"    section-remove: remove an existing section" << std::endl;
        std::wcout << L"    attribute-add: add a new NXL attribute" << std::endl;
        std::wcout << L"    attribute-remove: remove an existing NXL attribute" << std::endl;
        std::wcout << L"    template-add: set descationary rights template" << std::endl;
        std::wcout << L"    template-remove: remove descationary rights template" << std::endl;
        std::wcout << L"    tag-add: add a new NXL attribute" << std::endl;
        std::wcout << L"    tag-remove: remove an existing NXL attribute" << std::endl;
    }

    return 0;
}

int nxlmc::create(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;
    std::wstring in_file;
    std::wstring out_file;
    std::vector<std::pair<std::wstring, unsigned long>> sections = { { NXL_SECTION_ATTRIBUTESW, 2048 },{ NXL_SECTION_TEMPLATESW, 4096 },{ NXL_SECTION_TAGSW, 4096 } };

    auto pos = parameters.find(L"-i");
    if (pos != parameters.end()) {
        if (!(*pos).second.empty()) {
            in_file = (*pos).second[0];
        }
    }

    pos = parameters.find(L"-o");
    if (pos != parameters.end()) {
        if (!(*pos).second.empty()) {
            out_file = (*pos).second[0];
        }
    }

    pos = parameters.find(L"-s");
    if (pos != parameters.end()) {
        std::for_each((*pos).second.begin(), (*pos).second.end(), [&](const std::wstring& s) {

            auto sp = s.find_first_of(L':');
            std::wstring section_name;
            unsigned long section_size = 0;

            if (sp == std::wstring::npos) {
                section_name = s;
                section_size = 4096;
            }
            else {
                section_name = s.substr(0, sp);
                std::wstring numstr = s.substr(sp + 1);
                boost::algorithm::trim(numstr);
                if (numstr.empty()) {
                    section_size = 4096;
                }
                else {
                    wchar_t* pend = NULL;
                    section_size = std::wcstoul(numstr.c_str(), &pend, 0);
                    if (section_size == 0) {
                        section_size = 4096;
                    }
                }
            }

            auto section_pos = std::find_if(sections.begin(), sections.end(), [&](const std::pair<std::wstring, unsigned long>& it) -> bool {
                return (0 == _wcsicmp(section_name.c_str(), it.first.c_str()));
            });

            if (section_pos == sections.end()) {
                // new
                if (section_size == 0) {
                    section_size = 4096;
                }
                sections.push_back(std::pair<std::wstring, unsigned long>(section_name, section_size));
            }
            else {
                if (section_size != 0) {
                    (*section_pos).second = section_size;
                }
            }
        });
    }

    if (in_file.empty() && out_file.empty()) {
        std::wcout << L"ERROR: no file is specified" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }

    if (in_file.empty()) {
        std::wcout << L"Creating an empty NXL file ..." << std::endl;
        result = create_empty_file(out_file, sections);
    }
    else {
        std::wcout << L"Converting an existing file to NXL file ..." << std::endl;
        result = convert_file(in_file, out_file, sections);
    }

    return result;
}

int nxlmc::decrypt(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}

int nxlmc::verify(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;

    // get file
    auto pos = parameters.find(L"-i");
    if (pos == parameters.end()) {
        std::wcout << L"ERROR: no file is specified" << std::endl;
    }
    const std::wstring& in_file = (*pos).first;

    // check
    try {
        NX::NXL::nxl_header header;
        header.load(in_file);
        result = ERROR_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cout << L"ERROR: " << e.what() << std::endl;
        result = ERROR_INVALID_DATA;
    }

    return result;
}

int nxlmc::show(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;

    // get file
    auto pos = parameters.find(L"-i");
    if (pos == parameters.end()) {
        std::wcout << L"ERROR: no file is specified" << std::endl;
    }
    if((*pos).second.empty()) {
        std::wcout << L"ERROR: no file is specified" << std::endl;
    }
    const std::wstring& in_file = (*pos).second[0];

    bool show_header = false;
    bool show_attributes = false;
    bool show_templates = false;
    bool show_tags = false;

    if (parameters.end() != parameters.find(L"-all")) {
        show_header = true;
        show_attributes = true;
        show_templates = true;
        show_tags = true;
    }
    else {
        show_header = (parameters.end() != parameters.find(L"-h"));
        show_attributes = (parameters.end() != parameters.find(L"-a"));
        show_templates = (parameters.end() != parameters.find(L"-r"));
        show_tags = (parameters.end() != parameters.find(L"-t"));

        if (!show_header && !show_attributes && !show_templates && !show_tags) {
            // by default, show header
            show_header = true;
        }
    }

    // check
    try {
        
        NX::NXL::nxl_file file;
        file.open(in_file, _keystore.keys(), true);

        // print header information
        if (show_header) {

            const NX::NXL::nxl_header& header = file.header();

            std::wcout << L"-------------------------" << std::endl;
            std::wcout << L"*  NXL FILE HEADER" << std::endl;
            std::wcout << L"-------------------------" << std::endl;

            // signature
            std::wcout << L"SIGNATURE" << std::endl;
            std::wcout << L"  Code: NXLFMT!" << std::endl;
            std::wcout << L"  Message: " << header.message() << std::endl;

            // basic
            std::wstring content_offset_hex = nudf::string::FromUint<wchar_t>(header.content_offset());
            std::wcout << L"BASIC" << std::endl;
            std::wcout << L"  Thumbprint: " << header.thumbprint() << std::endl;
            std::wcout << L"  Flags: " << header.flags() << std::endl;
            std::wcout << L"  Alignment: " << header.alignment() << std::endl;
            std::wcout << L"  PointerOfContent: " << header.content_offset() << L" (" << content_offset_hex << L")" << std::endl;

            // crypto
            std::wstring content_size_hex = nudf::string::FromUint<wchar_t>((unsigned long)header.content_length());
            std::wcout << L"CRYPTO" << std::endl;
            std::wcout << L"  Algorithm: " << ((header.algorithm() == NXL_ALGORITHM_AES128) ? L"AES 128" : L"AES 256") << std::endl;
            std::wcout << L"  CBC Size: " << header.cbc_size() << std::endl;
            std::wcout << L"  Content Size: " << header.content_length() << L" (" << content_size_hex << L")" << std::endl;
            std::wcout << L"  Primary Key" << std::endl;
            if (header.primary_key_id().empty()) {
                throw std::exception("primary key doesn't exist");
            }
            else {
                std::wstring key_ring((const char*)(header.primary_key_id().id()), (const char*)(header.primary_key_id().id() + 8));
                std::vector<unsigned char> vkey_hash(header.primary_key_id().id() + 8, header.primary_key_id().id() + 40);
                const unsigned long ukey_time = *((unsigned long*)(header.primary_key_id().id() + 40));
                std::wstring key_hash = nudf::string::FromBytes<wchar_t>(vkey_hash.data(), (unsigned long)vkey_hash.size());
                nudf::time::CTime tm;
                tm.FromSecondsSince1970Jan1st(ukey_time);
                SYSTEMTIME st = { 0 };
                tm.ToSystemTime(&st);
                std::wstring key_time = nudf::string::FromSystemTime<wchar_t>(&st, true);
                std::wstring ukey_time_str;
                swprintf_s(nudf::string::tempstr<wchar_t>(ukey_time_str, 16), 16, L"%08X", ukey_time);
                std::wcout << L"    - algorithm: " << (header.is_primary_key_aes128() ? L"AES 128" : L"AES 256") << std::endl;
                std::wcout << L"    - key-ring: " << key_ring << std::endl;
                std::wcout << L"    - key-hash: " << key_hash << std::endl;
                std::wcout << L"    - key-time: " << ukey_time_str << L" (" << key_time << L")" << std::endl;
            }
            std::wcout << L"  Recovery Key" << std::endl;
            if (header.recovery_key_id().empty()) {
                std::wcout << L"    - N/A" << std::endl;
            }
            else {
                std::wstring key_ring((const char*)(header.recovery_key_id().id()), (const char*)(header.recovery_key_id().id() + 8));
                std::vector<unsigned char> vkey_hash(header.recovery_key_id().id() + 8, header.recovery_key_id().id() + 40);
                const unsigned long ukey_time = *((unsigned long*)(header.recovery_key_id().id() + 40));
                std::wstring key_hash = nudf::string::FromBytes<wchar_t>(vkey_hash.data(), (unsigned long)vkey_hash.size());
                nudf::time::CTime tm;
                tm.FromSecondsSince1970Jan1st(ukey_time);
                SYSTEMTIME st = { 0 };
                tm.ToSystemTime(&st);
                std::wstring key_time = nudf::string::FromSystemTime<wchar_t>(&st, true);
                std::wstring ukey_time_str;
                swprintf_s(nudf::string::tempstr<wchar_t>(ukey_time_str, 16), 16, L"%08X", ukey_time);
                std::wcout << L"    - algorithm: " << (header.is_recovery_key_aes128() ? L"AES 128" : L"AES 256") << std::endl;
                std::wcout << L"    - key-ring: " << key_ring << std::endl;
                std::wcout << L"    - key-hash: " << key_hash << std::endl;
                std::wcout << L"    - key-time: " << ukey_time_str << L" (" << key_time << L")" << std::endl;
            }

            // sections
            std::vector<unsigned char> primary_key = _keystore.find_key(header.primary_key_id());
            std::vector<unsigned char> content_key;
            if (!primary_key.empty()) {
                content_key = header.decrypt_content_key(primary_key);
            }
            std::wstring section_table_checksum = nudf::string::FromBytes<wchar_t>(header.section_table_checksum().data(), (unsigned long)header.section_table_checksum().size());
            const unsigned long cal_checksum = header.calc_sections_checksum();
            std::wstring section_table_checksum_calced = nudf::string::FromUint<wchar_t>(cal_checksum);
            std::wcout << L"SECTIONS" << std::endl;
            std::wcout << L"  Count: " << header.sections().size() << std::endl;
            std::wcout << L"  Checksum (encrypted): " << section_table_checksum << std::endl;
            if (!content_key.empty()) {
                const unsigned long dec_checksum = header.decrypt_sections_checksum(content_key);
                std::wstring section_table_checksum_plain = nudf::string::FromUint<wchar_t>(dec_checksum);
                if (dec_checksum == cal_checksum) {
                    std::wcout << L"  Checksum (verified): " << section_table_checksum_calced << std::endl;
                }
                else {
                    std::wcout << L"  Checksum (NOT MATCH): (calculated: " << section_table_checksum_calced << L") v.s. (decrypted: " << section_table_checksum_plain << L")" << std::endl;
                }
            }
            else {
                std::wcout << L"  Checksum (not verified): " << section_table_checksum_calced << std::endl;
            }

            int i = 0;
            unsigned long offset = 0;
            std::for_each(header.sections().begin(), header.sections().end(), [&](const NX::NXL::nxl_section& s) {
                std::wstring section_checksum = nudf::string::FromUint<wchar_t>(s.checksum());
                std::wstring section_offset = nudf::string::FromUint<wchar_t>(s.offset());
                std::wcout << L"  Section #" << i++ << std::endl;
                std::wcout << L"    - name: " << s.name() << std::endl;
                std::wcout << L"    - size: " << s.size() << std::endl;
                std::wcout << L"    - offset: " << s.offset() << L" (" << section_offset << L")" << std::endl;
                std::wcout << L"    - checksum: " << section_checksum << std::endl;
            });
        }


        // print nxl attributes information
        if (show_attributes) {
            bool validated = false;
            const NX::NXL::nxl_file::AttributeMapType& attributes = file.get_nxl_attributes(&validated);
            std::wcout << L"-------------------------" << std::endl;
            std::wcout << L"*  NXL FILE ATTRIBUTES  (" << (validated ? L"verified)" : L"wrong checksum)") << std::endl;
            std::wcout << L"-------------------------" << std::endl;
            std::for_each(attributes.begin(), attributes.end(), [&](const std::pair<std::wstring,std::wstring>& attr) {
                std::wcout << L"    " << attr.first << L": " << attr.second << std::endl;
            });
        }


        // print nxl rights template information
        if (show_templates) {
            bool validated = false;
            bool remote = file.is_remote_eval(&validated);
            std::wcout << L"-------------------------" << std::endl;
            std::wcout << L"*  NXL FILE RIGHTS TEMPLATE  (" << (validated ? L"verified)" : L"wrong checksum)") << std::endl;
            std::wcout << L"-------------------------" << std::endl;
            std::wcout << L"    remote evaluation: " << (remote ? L"yes" : L"no") << std::endl;
        }


        // print nxl tag information
        if (show_tags) {
            bool validated = false;
            const NX::NXL::nxl_file::TagMapType& tags = file.get_nxl_tags(&validated);
            std::wcout << L"-------------------------" << std::endl;
            std::wcout << L"*  NXL FILE TAGS  (" << (validated ? L"verified)":L"wrong checksum)") << std::endl;
            std::wcout << L"-------------------------" << std::endl;
            std::for_each(tags.begin(), tags.end(), [&](const std::pair<std::wstring,std::set<std::wstring>>& tag) {
                std::for_each(tag.second.begin(), tag.second.end(), [&](const std::wstring& value) {
                    std::wcout << L"    " << tag.first << L": " << value << std::endl;
                });
            });
        }

        result = ERROR_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cout << L"ERROR: " << e.what() << std::endl;
        result = ERROR_INVALID_DATA;
    }

    return result;
}

int nxlmc::section_resize(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;

    // get file
    auto pos = parameters.find(L"-i");
    if (pos == parameters.end()) {
        std::wcout << L"ERROR: file name is not specified" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }
    if ((*pos).second.empty()) {
        std::wcout << L"ERROR: file name is not specified" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }
    const std::wstring& in_file = (*pos).second[0];

    pos = parameters.find(L"-n");
    if (pos == parameters.end()) {
        std::wcout << L"ERROR: section name is not specified" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }
    if ((*pos).second.empty()) {
        std::wcout << L"ERROR: section name is not specified" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }
    const std::wstring& section_name = (*pos).second[0];

    pos = parameters.find(L"-s");
    if (pos == parameters.end()) {
        std::wcout << L"ERROR: section size is not specified" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }
    if ((*pos).second.empty()) {
        std::wcout << L"ERROR: section size is not specified" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }
    long section_size = _wtoi((*pos).second[0].c_str());

    // check
    try {

        // open target file
        NX::NXL::nxl_file nf;
        nf.open(in_file, _keystore.keys(), false);

        // 
        result = ERROR_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cout << L"ERROR: " << e.what() << std::endl;
        result = ERROR_INVALID_DATA;
    }

    if (0 == section_size) {
        // delete section
    }
    else {
        if (0 != (section_size%NXL_PAGE_SIZE)) {
            std::wcout << L"ERROR: section size is not aligned with NXL page size (4096)" << std::endl;
            return ERROR_INVALID_PARAMETER;
        }
    }

    return result;
}

int nxlmc::section_add(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}

int nxlmc::section_remove(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}

int nxlmc::attribute_add(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}

int nxlmc::attribute_remove(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}

int nxlmc::template_add(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}

int nxlmc::template_remove(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}

int nxlmc::template_set_remote(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;

    // get file
    auto pos = parameters.find(L"-i");
    if (pos == parameters.end()) {
        std::wcout << L"ERROR: file name is not specified" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }
    if ((*pos).second.empty()) {
        std::wcout << L"ERROR: file name is not specified" << std::endl;
        return ERROR_INVALID_PARAMETER;
    }
    const std::wstring& in_file = (*pos).second[0];

    bool set = (parameters.end() == parameters.find(L"-local"));

    // check
    try {

        // open target file
        NX::NXL::nxl_file nf;
        nf.open(in_file, _keystore.keys(), false);

        nf.set_remote_eval(set);

        // 
        result = ERROR_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cout << L"ERROR: " << e.what() << std::endl;
        result = ERROR_INVALID_DATA;
    }
    return result;
}

int nxlmc::tag_add(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}

int nxlmc::tag_remove(const std::map<std::wstring, std::vector<std::wstring>>& parameters)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}

//
//
//
int nxlmc::create_empty_file(const std::wstring& file, const std::vector<std::pair<std::wstring, unsigned long>>& sections)
{
    int result = ERROR_INVALID_FUNCTION;

    NX::NXL::nxl_header header;

    assert(!_keystore.empty());

    return result;
}

int nxlmc::convert_file(const std::wstring& in_file, const std::wstring& out_file, const std::vector<std::pair<std::wstring, unsigned long>>& sections)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}

int nxlmc::decrypt_file(const std::wstring& in_file, const std::wstring& out_file)
{
    int result = ERROR_INVALID_FUNCTION;
    return result;
}