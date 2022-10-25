

#include <windows.h>
#include <assert.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>

#include <nudf\shared\logdef.h>
#include <nudf\string.hpp>
#include <nudf\convert.hpp>

#include "dbglog.hpp"
#include "nxlogdb.hpp"
#include "nxlogdbfile.hpp"


using namespace NXLOGDB;


static const unsigned char DB_MAGIC_CODE[8] = { 'N', 'X', 'L', 'O', 'G', 'D', 'B', 0xFB };

//
//
//

bits_map::bits_map() : _size(0)
{
}

bits_map::bits_map(size_t bits_size) : _size(bits_size)
{
    _map.resize((bits_size + 31) / 32, 0);
}

bits_map::~bits_map()
{
}

void bits_map::resize(size_t bits_size)
{
    const unsigned long count = (unsigned long)((bits_size + 31) / 32);
    _size = bits_size;
    _map.resize(count, 0);
}

void bits_map::clear()
{
    _size = 0;
    _map.clear();
}

void bits_map::attach(size_t bits_size, const std::vector<unsigned long>& buffer)
{
    const unsigned long count = (unsigned long)((bits_size + 31) / 32);
    _size = bits_size;
    assert(count == (unsigned long)buffer.size());
    _map.clear();
    _map = buffer;
}

bool bits_map::is_bit_on(_In_ unsigned long bit) const noexcept
{
    if (bit >= size()) {
        return false;
    }
    return check_bit(_map[bit / 32], bit % 32);
}

bool bits_map::operator [](_In_ unsigned long bit) const noexcept
{
    return is_bit_on(bit);
}

void bits_map::set_bit(_In_ unsigned long bit) noexcept
{
    if (bit < size()) {
        set_bit(_map[bit / 32], bit % 32);
    }
}

void bits_map::clear_bit(_In_ unsigned long bit) noexcept
{
    if (bit < size()) {
        clear_bit(_map[bit / 32], bit % 32);
    }
}

void bits_map::clear_all()
{
    if (!_map.empty()) {
        memset(_map.data(), 0, _map.size()*sizeof(unsigned long));
    }
}

bits_map& bits_map::operator = (_In_ unsigned long data)
{
    _map[0] = data;
    return *this;
}

bits_map& bits_map::operator = (_In_ int data)
{
    _map[0] = (unsigned long)data;
    return *this;
}

unsigned long bits_map::reserve_first_free_bit(unsigned long start_pos)
{
    for (int i = (int)(start_pos / 32); i < (int)_map.size(); i++) {
        if (MAX_ULONG != _map[i]) {
            unsigned long bit = find_free_bit(_map[i]);
            assert(bit < 32);
            bit += i * 32;
            if (bit < size()) {
                set_bit(bit);
                return bit;
            }
            else {
                return 0;
            }
        }
    }
    return 0;
}


static const unsigned long bits_mask_map[32] = {
    0x00000001, 0x00000002, 0x00000004, 0x00000008, 0x00000010, 0x00000020, 0x00000040, 0x00000080,
    0x00000100, 0x00000200, 0x00000400, 0x00000800, 0x00001000, 0x00002000, 0x00004000, 0x00008000,
    0x00010000, 0x00020000, 0x00040000, 0x00080000, 0x00100000, 0x00200000, 0x00400000, 0x00800000,
    0x01000000, 0x02000000, 0x04000000, 0x08000000, 0x10000000, 0x20000000, 0x40000000, 0x80000000
};

unsigned long bits_map::find_free_bit(unsigned long value) const noexcept
{
    for (unsigned long i = 0; i < 32; i++) {
        if (!check_bit(value, i)) {
            return i;
        }
    }
    return MAX_ULONG;
}

bool bits_map::check_bit(unsigned long value, unsigned long bit) const noexcept
{
    assert(bit < 32);
    return (0 != (value & bits_mask_map[bit]));
}

void bits_map::set_bit(unsigned long& value, unsigned long bit) noexcept
{
    assert(bit < 32);
    value |= bits_mask_map[bit];
}

void bits_map::clear_bit(unsigned long& value, unsigned long bit) noexcept
{
    assert(bit < 32);
    value &= (~bits_mask_map[bit]);
}



//
//
//

logdb_record_map::logdb_record_map()
{
}

logdb_record_map::~logdb_record_map()
{
}

void logdb_record_map::resize(size_t size)
{
    _records.resize(size, 0);
}

void logdb_record_map::attach(const std::vector<unsigned long>& buffer)
{
    _records.clear();
    _records = buffer;
}

unsigned long logdb_record_map::get_record_sector_id(size_t record_id) const
{
    return ((record_id < _records.size()) ? _records[record_id] : MAX_ULONG);
}


class log_exporter
{
public:
    log_exporter(const std::wstring& file) : _h(INVALID_HANDLE_VALUE)
    {
        open(file);
    }
    virtual ~log_exporter()
    {
        close();
    }

    inline bool is_opened() const { return (INVALID_HANDLE_VALUE != _h); }

    void export_header()
    {
        static const std::wstring line1(L"*********************************");
        static const std::wstring line2(L"*        ACTIVITY LOG           *");
        static const std::wstring line3(L"*********************************");

        write_file(line1.c_str(), line1.length() * sizeof(wchar_t));
        write_line_ending();
        write_file(line2.c_str(), line2.length() * sizeof(wchar_t));
        write_line_ending();
        write_file(line3.c_str(), line3.length() * sizeof(wchar_t));
        write_line_ending();
    }

    void export_conf(const db_conf& conf)
    {
        static const std::wstring line1(L"[CONFIGURATION]");
        static const std::wstring line2(L"[RECORDS]");
        write_line_ending();
        write_file(line1.c_str(), line1.length() * sizeof(wchar_t));
        write_line_ending();
        
        std::wstring line;

        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    Description: %S", conf.get_description().c_str());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB Size: %d MB", conf.get_db_size() / 1048576);
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB Flags: %08X", conf.get_db_flags());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB Block Size: %d", conf.get_block_size());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB Sector Size: %d", conf.get_sector_size());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB Sector data Size: %d", conf.get_sector_data_size());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB Sector fields Size: %d", conf.get_fields_size());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB record map offset: 0x%08X", conf.get_record_map_offset());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB record map size: 0x%08X", conf.get_record_map_size());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB record start id: 0x%08X", conf.get_record_start_id());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB record count: 0x%08X", conf.get_record_count());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB record max count: 0x%08X", conf.get_max_record_count());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB sector map offset: 0x%08X", conf.get_sector_map_offset());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB sector map size: 0x%08X", conf.get_sector_map_size());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB sector count: 0x%08X", conf.get_sector_start_offset());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"    DB sector start offset: 0x%08X", conf.get_max_record_count());
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();

        const NXLOGDB::record_layout& layout = conf.get_record_layout();
        for (int i = 0; i < (int)layout.get_fields_def().size(); i++) {
            swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"      > Field #%d", i);
            write_file(line.c_str(), line.length() * sizeof(wchar_t));
            write_line_ending();
            swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"          Name:  %S", layout.get_fields_def()[i].get_field_name().c_str());
            write_file(line.c_str(), line.length() * sizeof(wchar_t));
            write_line_ending();
            swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"          Type:  %s", layout.get_fields_def()[i].get_field_type_name());
            write_file(line.c_str(), line.length() * sizeof(wchar_t));
            write_line_ending();
            swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"          Flags: %08X", layout.get_fields_def()[i].get_field_flags());
            write_file(line.c_str(), line.length() * sizeof(wchar_t));
            write_line_ending();
            swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"          Size:  %d", layout.get_fields_def()[i].get_field_size());
            write_file(line.c_str(), line.length() * sizeof(wchar_t));
            write_line_ending();
        }

        write_line_ending();
        write_file(line2.c_str(), line2.length() * sizeof(wchar_t));
        write_line_ending();
        write_line_ending();
    }

    void export_record(unsigned long id, const db_record& record, const record_layout& layout)
    {
        std::wstring line;

        write_line_ending();

        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"  Record #%d", id);
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();

        for (int i = 0; i < (int)layout.get_fields_def().size(); i++) {
            // write name
            swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"      %S: ", layout.get_fields_def()[i].get_field_name().c_str());
            write_file(line.c_str(), line.length() * sizeof(wchar_t));
            // write value
            switch (record.get_fields()[i].get_type())
            {
            case FIELD_INTEGER:
                swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"%I64d", record.get_fields()[i].as_int64());
                break;

            case FIELD_UNSIGNED_INTEGER:
                {
                    unsigned __int64 u = record.get_fields()[i].as_uint64();
                    const bool is_time = boost::algorithm::icontains(layout.get_fields_def()[i].get_field_name(), L"time");
                    if (is_time) {
                        FILETIME ft = { 0, 0 };
                        SYSTEMTIME st = { 0 };
                        ft.dwHighDateTime = (unsigned long)(u >> 32);
                        ft.dwLowDateTime = (unsigned long)u;
                        FileTimeToLocalFileTime(&ft, &ft);
                        FileTimeToSystemTime(&ft, &st);
                        swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"%04d-%02d-%02d %02d:%02d:%02d.%03d",
                            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
                    }
                    else {
                        if (u > 0xFFFFFFFF) {
                            swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"0x%08X%08x", (unsigned long)(u >> 32), (unsigned long)u);
                        }
                        else {
                            swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, L"0x%08X", (unsigned long)u);
                        }
                    }
                }
                break;

            case FIELD_DOUBLE:
                break;

            case FIELD_BOOL:
                swprintf_s(nudf::string::tempstr<wchar_t>(line, MAX_PATH), MAX_PATH, record.get_fields()[i].as_boolean() ? L"true" : L"false");
                break;

            case FIELD_CHAR:
                line = record.get_fields()[i].as_string();
                break;

            default:
                line = L"Bad Data";
                break;
            }
            write_file(line.c_str(), line.length() * sizeof(wchar_t));
            write_line_ending();
        }

        const std::vector<unsigned char>& opaque_data = record.get_opaque_data();
        if (!opaque_data.empty()) {
            line = L"      Opaque Data: ";
            line += nudf::util::convert::Utf8ToUtf16(std::string(opaque_data.begin(), opaque_data.end()));
        }
        else {
            line = L"      Opaque Data: ";
        }
        write_file(line.c_str(), line.length() * sizeof(wchar_t));
        write_line_ending();
    }

private:
    bool open(const std::wstring& file)
    {
        _h = ::CreateFileW(file.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (INVALID_HANDLE_VALUE == _h) {
            return false;
        }
        if (!write_file_bom()) {
            CloseHandle(_h);
            _h = INVALID_HANDLE_VALUE;
            return false;
        }
        return true;
    }
    void close()
    {
        if (INVALID_HANDLE_VALUE != _h) {
            CloseHandle(_h);
            _h = INVALID_HANDLE_VALUE;
        }
    }

    inline bool write_file_bom()
    {
        static const unsigned short BOM_UTF16 = 0xFEFF;
        return write_file(&BOM_UTF16, sizeof(BOM_UTF16));
    }

    inline bool write_line_ending()
    {
        static const wchar_t file_endl[2] = { L'\r', L'\n' };
        return write_file(file_endl, sizeof(file_endl));
    }

    inline bool write_file(const void* data, size_t size)
    {
        unsigned long written = 0;
        assert(INVALID_HANDLE_VALUE != _h);
        return (WriteFile(_h, data, (unsigned long)size, &written, NULL) && (unsigned long)size == written);
    }


private:
    HANDLE _h;
};


//
//
//

logdb_file::logdb_file() : _h(INVALID_HANDLE_VALUE)
{
}

logdb_file::~logdb_file()
{
}

void logdb_file::create(const std::wstring& file, const db_conf& conf)
{
    exclusive_locker lock_db(&_db_lock);

    if (opened()) {
        DBGLOG(LOGERROR, L"db file has been opened already");
        throw std::exception("db already opened");
    }

    if (!NXLOGDB::db_conf_validate(conf)) {
        DBGLOG(LOGERROR, L"invalid db configuration");
        throw std::exception("invalid db configuration");
    }


    _h = ::CreateFile(file.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH, NULL);
    if (INVALID_HANDLE_VALUE == _h) {
        DBGLOG(LOGERROR, L"fail to create db file (%s), error is %d", file.c_str(), GetLastError());
        throw std::exception("fail to create db file");
    }

    try {

        _db_conf = conf;

        // build map
        _sector_map.resize(_db_conf.get_sector_count());
        _record_map.resize(_db_conf.get_sector_count());

        // set file size
        if (!set_file_size(_db_conf.get_sector_start_offset())) {
            DBGLOG(LOGERROR, L"fail to initialize db file size (%s), error is %d", file.c_str(), GetLastError());
            throw std::exception("fail to create db file");
        }

        // save header
        DBHEADER header = { 0 };
        write_conf(&header);
        if (!write_file(0, &header, (unsigned long)sizeof(header))) {
            DBGLOG(LOGERROR, L"ERROR: fail to save initial db file header (%s), error is %d", file.c_str(), GetLastError());
            throw std::exception("fail to create db file");
        }

        // allocate first page
        if (!allocate_block()) {
            DBGLOG(LOGERROR, L"ERROR: fail to allocate first page in db file (%s), error is %d", file.c_str(), GetLastError());
            throw std::exception("fail to create db file");
        }

        // always mark first sector is used in sector map
        _sector_map.set_bit(0);
        const unsigned long first_block = _sector_map.get_block(0);
        write_file(_db_conf.get_sector_map_offset(), &first_block, (unsigned long)sizeof(unsigned long));
    }
    catch (const std::exception& e) {
        close();
        ::DeleteFileW(file.c_str());
        throw e;
    }
}

void logdb_file::open(const std::wstring& file, bool read_only)
{
    exclusive_locker lock_db(&_db_lock);

    if (opened()) {
        DBGLOG(LOGERROR, L"db file has been opened already");
        throw std::exception("db already opened");
    }

    unsigned long access_flags = read_only ? GENERIC_READ : (GENERIC_READ | GENERIC_WRITE);
    unsigned long file_flags = read_only ? FILE_ATTRIBUTE_NORMAL : (FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH);

    _h = ::CreateFile(file.c_str(), access_flags, FILE_SHARE_READ, NULL, OPEN_EXISTING, file_flags, NULL);
    if (INVALID_HANDLE_VALUE == _h) {
        DBGLOG(LOGERROR, L"fail to open db file (%s), error is %d", file.c_str(), GetLastError());
        throw std::exception("fail to open db file");
    }

    try {
        
        // load header
        DBHEADER header = { 0 };
        if (!read_file(0, &header, sizeof(header))) {
            DBGLOG(LOGERROR, L"fail to read header from db file (%s), error is %d", file.c_str(), GetLastError());
            throw std::exception("fail to read header from db file");
        }

        read_conf(&header);
        if (_db_conf.empty()) {
            DBGLOG(LOGERROR, L"invalid db header");
            throw std::exception("invalid db header");
        }

        // build sector map
        std::vector<unsigned long>  map_buffer;
        map_buffer.resize(_db_conf.get_sector_map_size() / sizeof(unsigned long));
        if (!read_file(_db_conf.get_sector_map_offset(), map_buffer.data(), _db_conf.get_sector_map_size())) {
            DBGLOG(LOGERROR, L"fail to read sector map from db file (%s), error is %d", file.c_str(), GetLastError());
            throw std::exception("fail to read sector map from db file");
        }
        _sector_map.attach(_db_conf.get_sector_count(), map_buffer);
        
        // build record map
        map_buffer.clear();
        map_buffer.resize(_db_conf.get_record_map_size() / sizeof(unsigned long));
        if (!read_file(_db_conf.get_record_map_offset(), map_buffer.data(), _db_conf.get_record_map_size())) {
            DBGLOG(LOGERROR, L"fail to read record map from db file (%s), error is %d", file.c_str(), GetLastError());
            throw std::exception("fail to read record map from db file");
        }
        _record_map.attach(map_buffer);
    }
    catch (const std::exception& e) {
        close();
        throw e;
    }
}

void logdb_file::close()
{
    exclusive_locker lock_db(&_db_lock);

    if (opened()) {
        CloseHandle(_h);
        _h = INVALID_HANDLE_VALUE;
        _db_conf.clear();
        _sector_map.clear();
        _record_map.clear();
    }
}

void logdb_file::export_log(const std::wstring& file) const
{
    // prepare export file
    log_exporter exporter(file);
    if (!exporter.is_opened()) {
        DBGLOG(LOGERROR, L"Fail to open db log export file (%d)", GetLastError());
        return;
    }

    exporter.export_header();

    shared_locker lock_db(&_db_lock);
    exporter.export_conf(get_db_conf());
    const unsigned long max_record = get_db_conf().get_record_count();
    for (unsigned long i = 0; i < max_record; i++) {
        try {
            const NXLOGDB::db_record& record = read_record_safe(i);
            exporter.export_record(i, record, get_db_conf().get_record_layout());
        }
        catch (const std::exception& e) {
            UNREFERENCED_PARAMETER(e);
            DBGLOG(LOGERROR, L"Fail to open db log export file (%d)", GetLastError());
        }
    }
}

NXLOGDB::db_record logdb_file::read_record(unsigned long record_id) const
{
    shared_locker lock_db(&_db_lock);
    return read_record_safe(record_id);
}

NXLOGDB::db_record logdb_file::read_record_safe(unsigned long record_id) const
{
    if (get_db_conf().record_empty() || record_id >= get_db_conf().get_record_count()) {
        DBGLOG(LOGERROR, L"record #%d not found", record_id);
        throw std::exception("record not found");
    }

    const unsigned long real_record_id = (get_db_conf().get_record_start_id() + record_id) % get_db_conf().get_record_count();
    const unsigned long sector_id = _record_map.get_record_sector_id(real_record_id);
    if (0 == sector_id) {
        DBGLOG(LOGERROR, L"record #%d is empty", record_id);
        throw std::exception("record is empty");
    }

    const std::vector<unsigned char>& data = inter_read_record(sector_id);
    const unsigned char* p = data.data();

    if ((get_db_conf().get_fields_size() + 4) > (unsigned long)data.size()) {
        DBGLOG(LOGERROR, L"Not read all the fields of record #%d (read: %d, expected: %d)", record_id, data.size(), get_db_conf().get_fields_size() + 4);
        throw std::exception("not read all the fields of record");
    }

    const unsigned long opaque_data_size = *((unsigned long*)(p + get_db_conf().get_fields_size()));

    if ((get_db_conf().get_fields_size() + 4 + opaque_data_size) > (unsigned long)data.size()) {
        DBGLOG(LOGERROR, L"Not read all the fields of record #%d (read: %d, expected: %d)", record_id, data.size(), get_db_conf().get_fields_size() + 4 + opaque_data_size);
        throw std::exception("not read all the data of record");
    }

    // get fields first
    std::vector<NXLOGDB::field_value> fields;
    std::for_each(get_db_conf().get_record_layout().get_fields_def().begin(), get_db_conf().get_record_layout().get_fields_def().end(), [&](const NXLOGDB::field_definition& field) {
        switch (field.get_field_type())
        {
        case FIELD_INTEGER:
            fields.push_back(NXLOGDB::field_value(*((__int64*)p)));
            p += field.get_field_size();
            break;

        case FIELD_UNSIGNED_INTEGER:
            fields.push_back(NXLOGDB::field_value(*((unsigned __int64*)p)));
            p += field.get_field_size();
            break;

        case FIELD_DOUBLE:
            fields.push_back(NXLOGDB::field_value(*((double*)p)));
            p += field.get_field_size();
            break;

        case FIELD_BOOL:
            fields.push_back(NXLOGDB::field_value(*((bool*)p)));
            p += field.get_field_size();
            break;

        case FIELD_CHAR:
        {
            const wchar_t* s = (const wchar_t*)p;
            fields.push_back(NXLOGDB::field_value(std::wstring(s, s + field.get_field_size())));
            p += field.get_field_data_size();
        }
        break;

        default:
            assert(false);  // should never reach here
            throw std::exception("bad field data");
            break;
        }
    });

    // get opaque data
    p += sizeof(unsigned long);
    return NXLOGDB::db_record(record_id, fields, std::vector<unsigned char>(p, p + opaque_data_size));
}

unsigned long logdb_file::free_record(unsigned long record_count)
{
    exclusive_locker lock_db(&_db_lock);

    unsigned long bytes_freed = 0;

    if (record_count > _db_conf.get_record_count()) {
        record_count = _db_conf.get_record_count();
    }

    while (0 != record_count) {
        bytes_freed += inter_pop_record();
    }

    return bytes_freed;
}

bool logdb_file::push_record(const NXLOGDB::db_record& record)
{
    exclusive_locker lock_db(&_db_lock);

    if (_db_conf.record_full()) {
        // full? remove first one
        inter_pop_record();
    }

    const unsigned long next_reord_id = (get_db_conf().get_record_start_id() + get_db_conf().get_record_count()) % get_db_conf().get_max_record_count();
    const std::vector<unsigned char>& buf = record.serialize(get_db_conf().get_record_layout());

    unsigned long record_start_sector_id = 0;

    // allocate sectors list
    do {

        record_start_sector_id = allocate_sector_list((unsigned long)buf.size());
        if (0 == record_start_sector_id) {
            unsigned long bytes_freed = 0;
            while (bytes_freed <= (unsigned long)buf.size()) {
                bytes_freed += inter_pop_record();
            }
        }

    } while (0 == record_start_sector_id);

    if (!inter_write_record(record_start_sector_id, buf)) {
        inter_free_sector(record_start_sector_id);
        DBGLOG(LOGERROR, L"fail to write record #%d (start sector id = %d)", next_reord_id, record_start_sector_id);
        return false;
    }

    _record_map.get_records()[next_reord_id] = record_start_sector_id;
    write_record_sector_id(next_reord_id, record_start_sector_id);
    _db_conf.set_record_count(get_db_conf().get_record_count() + 1);
    write_record_count(get_db_conf().get_record_count());

    return true;
}

// return bytes freed
unsigned long logdb_file::pop_record()
{
    exclusive_locker lock_db(&_db_lock);
    return inter_pop_record();
}

// return bytes freed
unsigned long logdb_file::inter_pop_record()
{
    if (_db_conf.record_empty()) {
        return 0;
    }

    const unsigned long first_record_id = _db_conf.get_record_start_id();
    const unsigned long record_start_sector_id = _record_map.get_record_sector_id(first_record_id);
    assert(0 != record_start_sector_id);

    // update db information
    unsigned long new_first_record_id = (first_record_id + 1) % _db_conf.get_max_record_count();
    _db_conf.set_record_count(_db_conf.get_record_count() - 1);
    _db_conf.set_record_start_id(new_first_record_id);
    write_record_start_id(new_first_record_id);
    write_record_count(_db_conf.get_record_count());

    return (0 != record_start_sector_id) ? inter_free_sector(record_start_sector_id) : 0;
}

bool logdb_file::write_record_count(unsigned long count)
{
    if (count >= get_db_conf().get_max_record_count()) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DBGLOG(LOGERROR, L"record (%d) exceed max count (%d)", count, get_db_conf().get_max_record_count());
        return false;
    }
    const unsigned long offset = FIELD_OFFSET(DBHEADER, record_count);
    return write_file(offset, &count, sizeof(count));
}

bool logdb_file::write_record_start_id(unsigned long record_id)
{
    if (record_id >= get_db_conf().get_max_record_count()) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DBGLOG(LOGERROR, L"record id (%d) exceed max count (%d)", record_id, get_db_conf().get_max_record_count());
        return false;
    }
    const unsigned long offset = FIELD_OFFSET(DBHEADER, record_start_id);
    return write_file(offset, &record_id, sizeof(record_id));
}

bool logdb_file::write_record_sector_id(unsigned long record_id, unsigned long sector_id)
{
    if (record_id >= get_db_conf().get_max_record_count()) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DBGLOG(LOGERROR, L"record id (%d) exceed max count (%d)", record_id, get_db_conf().get_max_record_count());
        return false;
    }
    if (sector_id >= get_db_conf().get_sector_count() || 0 == sector_id) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DBGLOG(LOGERROR, L"sector id (%d) exceed max count (%d) or sector is zero", sector_id, get_db_conf().get_sector_count());
        return false;
    }

    const unsigned long record_offset = get_db_conf().get_record_map_offset() + record_id * sizeof(unsigned long);
    return write_file(record_offset, &sector_id, sizeof(sector_id));
}

bool logdb_file::write_sector_next_id(unsigned long sector_id, unsigned long next_sector_id)
{
    if (sector_id >= get_db_conf().get_sector_count() || next_sector_id >= get_db_conf().get_sector_count()) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DBGLOG(LOGERROR, L"sector id (%d, %d) exceed max count (%d)", sector_id, next_sector_id, get_db_conf().get_sector_count());
        return false;
    }

    return write_file(get_sector_offset(sector_id), &next_sector_id, sizeof(next_sector_id));
}

unsigned long logdb_file::read_sector_next_id(unsigned long sector_id)
{
    unsigned long next_sector_id = 0;

    if (sector_id >= get_db_conf().get_sector_count()) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DBGLOG(LOGERROR, L"sector id (%d) exceed max count (%d) or sector is zero", sector_id, get_db_conf().get_sector_count());
        return 0;
    }

    if (!read_file(get_sector_offset(sector_id), &next_sector_id, sizeof(next_sector_id))) {
        return 0;
    }
    if (next_sector_id >= get_db_conf().get_sector_count()) {
        return 0;
    }

    return next_sector_id;
}

bool logdb_file::write_sector_map_block(unsigned long block_id, unsigned long block_value)
{
    if (block_id >= _sector_map.block_count()) {
        SetLastError(ERROR_INVALID_PARAMETER);
        DBGLOG(LOGERROR, L"sector map block id (%d) exceed max count (%d)", block_id, _sector_map.block_count());
        return false;
    }

    const unsigned long offset = get_db_conf().get_sector_map_offset() + block_id * sizeof(unsigned long);
    return write_file(offset, &block_value, sizeof(block_value));
}

unsigned long logdb_file::inter_free_sector(unsigned long sector_id)
{
    std::vector<unsigned long> sector_list;
    unsigned long bytes_freed = 0;

    do {
        sector_list.push_back(sector_id);
        sector_id = get_next_sector(sector_id);
    } while (0 != sector_id);

    std::map<unsigned long, unsigned long> dirty_map_blocks;

    std::for_each(sector_list.begin(), sector_list.end(), [&](const unsigned long& id) {
        write_sector_next_id(sector_id, 0);
        _sector_map.clear_bit(id);
        bytes_freed += _db_conf.get_sector_data_size();
        const unsigned long block_id = _sector_map.bit_id_to_block_id(id);
        dirty_map_blocks[block_id] = _sector_map.get_block(block_id);
    });

    // update sector map
    std::for_each(dirty_map_blocks.begin(), dirty_map_blocks.end(), [&](const std::pair<unsigned long, unsigned long>& block) {
        write_sector_map_block(block.first, block.second);
    });

    return bytes_freed;
}

unsigned long logdb_file::get_next_sector(unsigned long sector_id) const
{
    if (sector_id >= get_db_conf().get_sector_count()) {
        return 0;
    }

    unsigned long next_sector_id = 0;
    if (read_file(get_sector_offset(sector_id), &next_sector_id, sizeof(next_sector_id))) {
        return (next_sector_id < get_db_conf().get_sector_count()) ? next_sector_id : 0;
    }

    return 0;
}

unsigned long logdb_file::get_required_sector_number(unsigned long data_size)
{
    const unsigned long sector_available_size = get_db_conf().get_sector_size() - (unsigned long)sizeof(unsigned long);
    return ((data_size + sector_available_size - 1) / sector_available_size);
}

unsigned long logdb_file::allocate_sector_list(unsigned long record_size)
{
    const unsigned long sector_count = get_required_sector_number(record_size);
    std::vector<unsigned long>  sector_ids;

    try {

        unsigned long start_pos = 0;
        for (int i = 0; i < (int)sector_count; i++) {
            unsigned long sector_id = _sector_map.reserve_first_free_bit(start_pos);
            if (sector_id == 0) {
                throw std::exception("not enough space");
            }
            start_pos = sector_id + 1;
            sector_ids.push_back(sector_id);
        }

        std::map<unsigned long, unsigned long> dirty_map_blocks;

        std::for_each(sector_ids.begin(), sector_ids.end(), [&](const unsigned long& id) {
            const unsigned long block_id = _sector_map.bit_id_to_block_id(id);
            dirty_map_blocks[block_id] = _sector_map.get_block(block_id);
        });

        // update sector map
        std::for_each(dirty_map_blocks.begin(), dirty_map_blocks.end(), [&](const std::pair<unsigned long, unsigned long>& block) {
            const unsigned long block_offset = get_db_conf().get_sector_map_offset() + (unsigned long)(block.first * sizeof(unsigned long));
            write_file(block_offset, &block.second, (unsigned long)sizeof(unsigned long));
        });
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        std::for_each(sector_ids.begin(), sector_ids.end(), [&](const unsigned long& id) {
            _sector_map.clear_bit(id);
        });
        sector_ids.clear();
    }

    return sector_ids.empty() ? 0 : sector_ids[0];
}

bool logdb_file::allocate_block() noexcept
{
    const unsigned long current_file_size = GetFileSize(_h, NULL);
    const unsigned long current_data_size = current_file_size - get_db_conf().get_sector_start_offset();
    const unsigned long new_file_size = current_file_size + get_db_conf().get_block_size();

    if (current_data_size >= get_db_conf().get_db_size()) {
        DBGLOG(LOGCRITICAL, L"DB reach max size (%d)", get_db_conf().get_db_size());
        return false;
    }

    if (!IS_ALIGNED(current_data_size, get_db_conf().get_block_size())) {
        assert(false);
        DBGLOG(LOGCRITICAL, L"Data size (%d) is not aligned with page size (%d)", current_data_size, get_db_conf().get_block_size());
        return false;
    }

    if (!set_file_size(new_file_size)) {
        DBGLOG(LOGCRITICAL, L"Fail to set file size (%d), error is %d", new_file_size, GetLastError());
        return false;
    }

    return true;
}

std::vector<unsigned char> logdb_file::inter_read_record(unsigned long sector_id) const
{
    std::vector<unsigned char> data;
    
    std::vector<unsigned char> temp_buf;
    temp_buf.resize(get_db_conf().get_sector_data_size(), 0);
    while (sector_id != 0) {

        const unsigned long offset = get_sector_offset(sector_id) + (unsigned long)sizeof(unsigned long);
        read_file(offset, temp_buf.data(), get_db_conf().get_sector_data_size());
        std::for_each(temp_buf.begin(), temp_buf.end(), [&](unsigned char ch) {
            data.push_back(ch);
        });

        sector_id = get_next_sector(sector_id);
    }

    return std::move(data);
}

bool logdb_file::inter_write_record(unsigned long sector_id, const std::vector<unsigned char>& data)
{
    const unsigned char* p = data.data();
    unsigned long size = (unsigned long)data.size();

    while (sector_id != 0 && size != 0) {

        unsigned long bytes_to_write = min(get_db_conf().get_sector_data_size(), size);
        unsigned long offset = get_sector_offset(sector_id) + (unsigned long)sizeof(unsigned long);

        write_file(offset, p, bytes_to_write);
        size -= bytes_to_write;
        p += bytes_to_write;
        sector_id = get_next_sector(sector_id);

        if (bytes_to_write < get_db_conf().get_sector_data_size()) {
            // empty left area
            //  --> this must be last sector
            assert(0 == size);
            offset += bytes_to_write;
            bytes_to_write = get_db_conf().get_sector_data_size() - bytes_to_write;
            std::vector<unsigned char> zero_buf;
            zero_buf.resize(bytes_to_write, 0);
            write_file(offset, zero_buf.data(), bytes_to_write);
        }
    }

    return true;
}

bool logdb_file::read_file(unsigned long offset, void* buf, unsigned long size)  const noexcept
{
    unsigned long bytes_read = 0;
    return (INVALID_SET_FILE_POINTER != SetFilePointer(_h, offset, NULL, FILE_BEGIN) && ReadFile(_h, buf, size, &bytes_read, NULL) && size == bytes_read);
}

bool logdb_file::write_file(unsigned long offset, const void* buf, const unsigned long size) noexcept
{
    unsigned long bytes_written = 0;
    return (INVALID_SET_FILE_POINTER != SetFilePointer(_h, offset, NULL, FILE_BEGIN) && WriteFile(_h, buf, size, &bytes_written, NULL) && size == bytes_written);
}

bool logdb_file::set_file_size(unsigned long size) noexcept
{
    return (INVALID_SET_FILE_POINTER != SetFilePointer(_h, size, NULL, FILE_BEGIN) && SetEndOfFile(_h));
}

void logdb_file::read_conf(const DBHEADER* header)
{
    // check magic
    if (0 != memcmp(&header->magic, DB_MAGIC_CODE, sizeof(unsigned __int64))) {
        return;
    }

    _db_conf.set_description(header->description);
    _db_conf.set_db_size(header->db_size);
    _db_conf.set_db_flags(header->db_flags);
    _db_conf.set_block_size(header->block_size);
    _db_conf.set_sector_size(header->sector_size);
    _db_conf.set_sector_data_size(header->sector_size - sizeof(unsigned long));
    _db_conf.set_fields_size(header->fields_size);
    _db_conf.set_record_map_offset(header->record_map_offset);
    _db_conf.set_record_map_size(header->record_map_size);
    _db_conf.set_record_start_id(header->record_start_id);
    _db_conf.set_record_count(header->record_count);
    _db_conf.set_sector_map_offset(header->sector_map_offset);
    _db_conf.set_sector_map_size(header->sector_map_size);
    _db_conf.set_sector_count(header->sector_count);
    _db_conf.set_sector_start_offset(header->sector_start_offset);
    std::vector<field_definition> fields_def;
    for (int i = 0; i < (int)header->fields_count; i++) {
        fields_def.push_back(NXLOGDB::field_definition(
            header->fields[i].name,
            (NXLOGDB::LOGDB_FIELD_TYPE)header->fields[i].type,
            header->fields[i].size,
            header->fields[i].flags
            ));
    }
    _db_conf.set_record_layout(NXLOGDB::record_layout(fields_def));

    if (!NXLOGDB::db_conf_validate(_db_conf)) {
        _db_conf.clear();
        DBGLOG(LOGERROR, L"Invalid logdb_info");
    }
}

void logdb_file::write_conf(DBHEADER* header) const
{
    memset(header, 0, sizeof(DBHEADER));
    memcpy(&header->magic, DB_MAGIC_CODE, sizeof(unsigned __int64));
    memcpy(header->description, _db_conf.get_description().c_str(), min(MAX_DESCRIPTION_SIZE - 1, _db_conf.get_description().length()));
    header->db_size = _db_conf.get_db_size();
    header->db_flags = _db_conf.get_db_flags();
    header->block_size = _db_conf.get_block_size();
    header->sector_size = _db_conf.get_sector_size();
    header->fields_size = _db_conf.get_fields_size();
    header->record_map_offset = _db_conf.get_record_map_offset();
    header->record_map_size = _db_conf.get_record_map_size();
    header->record_start_id = _db_conf.get_record_start_id();
    header->record_count = _db_conf.get_record_count();
    header->sector_map_offset = _db_conf.get_sector_map_offset();
    header->sector_map_size = _db_conf.get_sector_map_size();
    header->sector_count = _db_conf.get_sector_count();
    header->sector_start_offset = _db_conf.get_sector_start_offset();
    header->fields_count = (unsigned long)_db_conf.get_record_layout().get_fields_def().size();
    for (int i = 0; i < (int)header->fields_count; i++) {
        const std::string& name = _db_conf.get_record_layout().get_fields_def()[i].get_field_name();
        memcpy(header->fields[i].name, name.c_str(), min(MAX_FIELD_NAME_SIZE - 1, name.length()));
        header->fields[i].type = _db_conf.get_record_layout().get_fields_def()[i].get_field_type();
        header->fields[i].size = _db_conf.get_record_layout().get_fields_def()[i].get_field_size();
        header->fields[i].flags = _db_conf.get_record_layout().get_fields_def()[i].get_field_flags();
    }
}