
#pragma once
#ifndef __NXLOGDB_HPP__
#define __NXLOGDB_HPP__


#ifdef NXLOGDB_EXPORTS
#define NXLOGDB_API __declspec(dllexport)
#else
#ifdef NXLOGDB_STATIC
#define NXLOGDB_API
#else
#define NXLOGDB_API __declspec(dllimport)
#endif
#endif

#include <assert.h>

#include <string>
#include <vector>
#include <memory>


extern NXLOGDB_API const unsigned long DB_HEADER_SIZE;

namespace NXLOGDB {


typedef bool(__stdcall *DBG_LOG_ACCEPT)(unsigned long level);
typedef void(__stdcall *DBG_LOG)(unsigned long level, const wchar_t* message);
typedef bool(__stdcall *OPAQUE_DATA_QUERY)(_In_opt_ const wchar_t* s);

#define ROUND_TO_SIZE(Length, Alignment)        ((ULONG_PTR)(((ULONG_PTR)Length + ((ULONG_PTR)Alignment - 1)) & ~((ULONG_PTR)Alignment - 1)))
#define IS_ALIGNED(Pointer, Alignment)          ((((ULONG_PTR) (Pointer)) & ((Alignment) - 1)) == 0)

#define ONE_MEGA_BYTES          1048576             // 0x100000
#define ONE_GIGA_BYTES          1073741824          // 0x40000000

typedef enum LOGDB_FIELD_TYPE {
    FIELD_UNKNOWN = 0,
    FIELD_UNSIGNED_INTEGER,
    FIELD_INTEGER,
    FIELD_DOUBLE,
    FIELD_BOOL,
    FIELD_CHAR
} LOGDB_FIELD_TYPE;


typedef enum LOGDB_BLOCK_SIZE {
    BLOCK_1MB  = 1048576
} LOGDB_BLOCK_SIZE;

typedef enum LOGDB_SECTOR_SIZE {
    SECTOR_512 = 512,
    SECTOR_1024 = 1024,
    SECTOR_2048 = 2048,
    SECTOR_4096 = 4096
} LOGDB_SECTOR_SIZE;

#define FIELD_INTEGER_SIZE  sizeof(__int64)
#define FIELD_DOUBLE_SIZE   sizeof(double)
#define FIELD_BOOL_SIZE     sizeof(bool)

// define field flags
#define FIELD_FLAG_SEQUENTIAL   0x00000001


// define CONSTs
#define MAX_ULONG               0xFFFFFFFF
#define MAX_FIELDS_NUMBER       56
#define MAX_FIELD_NAME_SIZE     20
#define MAX_DESCRIPTION_SIZE    120

NXLOGDB_API void __stdcall SetDbgLogRoutine(DBG_LOG_ACCEPT pAccept, DBG_LOG pLog);


class field_definition
{
public:
    field_definition::field_definition()
        : _type(FIELD_UNKNOWN), _size(0), _flags(0)
    {
    }

    field_definition::field_definition(const field_definition& other)
        : _name(other.get_field_name()), _type(other.get_field_type()), _size(other.get_field_size()), _flags(other.get_field_flags())
    {
    }

    field_definition::field_definition(const std::string& field_name, const LOGDB_FIELD_TYPE field_type, unsigned long field_size, unsigned long flags)
        : _name(field_name), _type(field_type), _size(field_size), _flags(flags)
    {
        if (_name.length() >= MAX_FIELD_NAME_SIZE) {
            _name = _name.substr(0, MAX_FIELD_NAME_SIZE - 1);
        }
    }

    field_definition::~field_definition()
    {
    }

    field_definition& field_definition::operator = (const field_definition& other)
    {
        if (this != &other) {
            _name = other.get_field_name();
            _type = other.get_field_type();
            _size = other.get_field_size();
            _flags = other.get_field_flags();
        }
        return *this;
    }

    inline const std::string& get_field_name() const { return _name; }
    inline LOGDB_FIELD_TYPE get_field_type() const { return _type; }
    inline unsigned long get_field_flags() const { return _flags; }
    inline unsigned long get_field_size() const { return _size; }
    inline unsigned long get_field_data_size() const { return (NXLOGDB::FIELD_CHAR == _type) ? (2 * _size) : _size; }
    inline const wchar_t* get_field_type_name() const
    {
        switch (_type)
        {
        case NXLOGDB::FIELD_UNSIGNED_INTEGER:
            return L"Unsigned Integer";
        case NXLOGDB::FIELD_INTEGER:
            return L"Integer";
        case NXLOGDB::FIELD_DOUBLE:
            return L"Double";
        case NXLOGDB::FIELD_BOOL:
            return L"Boolean";
        case NXLOGDB::FIELD_CHAR:
            return L"Char";
        case NXLOGDB::FIELD_UNKNOWN:
        default:
            return L"Unknown";
        }
    }


private:
    std::string      _name;
    LOGDB_FIELD_TYPE _type;
    unsigned long    _flags;
    unsigned long    _size;
};

class record_layout
{
public:
    record_layout() : _fields_size(0), _opaque_data_offset(0), _opaque_data_size_offset(0) {}
    record_layout(const std::vector<field_definition>& db_fields_def) : _fields(db_fields_def), _fields_size(0), _opaque_data_offset(0), _opaque_data_size_offset(0)
    {
        calculate_size();
    }
    ~record_layout()
    {
    }

    inline bool empty() const { return _fields.empty(); }
    inline const std::vector<field_definition>& get_fields_def() const { return _fields; }
    inline const std::vector<unsigned long>& get_fields_offset() const { return _fields_offset; }
    inline unsigned long get_fields_size() const { return _fields_size; }
    inline unsigned long get_opaque_data_size_offset() const { return _opaque_data_size_offset; }
    inline unsigned long get_opaque_data_offset() const { return _opaque_data_offset; }

    record_layout& operator = (const record_layout& other)
    {
        if (this != &other) {
            _fields = other.get_fields_def();
            _fields_offset = other.get_fields_offset();
            _fields_size = other.get_fields_size();
            _opaque_data_size_offset = other.get_opaque_data_size_offset();
            _opaque_data_offset = other.get_opaque_data_offset();
        }
        return *this;
    }

    void clear()
    {
        _fields.clear();
        _fields_offset.clear();
        _fields_size = 0;
        _opaque_data_size_offset = 0;
        _opaque_data_offset = 0;
    }


private:
    void calculate_size()
    {
        unsigned long offset = 0;
        _fields_offset.clear();
        std::for_each(_fields.begin(), _fields.end(), [&](const field_definition& field) {
            _fields_offset.push_back(offset);
            offset += field.get_field_data_size();
        });
        _fields_size = offset;
        _opaque_data_size_offset = _fields_size;
        _opaque_data_offset = _opaque_data_size_offset + (unsigned long)sizeof(unsigned long);
    }

private:
    std::vector<field_definition>  _fields;
    std::vector<unsigned long>  _fields_offset;
    unsigned long   _fields_size;
    unsigned long   _opaque_data_size_offset;
    unsigned long   _opaque_data_offset;
};

class field_value
{
    typedef union _UVALUE {
        long                _l;
        unsigned long       _ul;
        __int64             _ll;
        unsigned __int64    _ull;
        double              _f;
        bool                _b;
    } UVALUE, *PUVALUE;

public:
    field_value() : _type(FIELD_UNKNOWN) {}
    explicit field_value(int v) : _type(FIELD_INTEGER) { set_uvalue(v); }
    explicit field_value(long v) : _type(FIELD_INTEGER) { set_uvalue((int)v); }
    explicit field_value(__int64 v) : _type(FIELD_INTEGER) { set_uvalue(v); }
    explicit field_value(unsigned int v) : _type(FIELD_UNSIGNED_INTEGER) { set_uvalue(v); }
    explicit field_value(unsigned long v) : _type(FIELD_UNSIGNED_INTEGER) { set_uvalue((unsigned int)v); }
    explicit field_value(unsigned __int64 v) : _type(FIELD_UNSIGNED_INTEGER) { set_uvalue(v); }
    explicit field_value(double v) : _type(FIELD_DOUBLE) { set_uvalue(v); }
    explicit field_value(bool v) : _type(FIELD_BOOL) { set_uvalue(v); }
    explicit field_value(const std::wstring& v, unsigned long count = -1) : _type(FIELD_CHAR)
    {
        if ((unsigned long)-1 != count) {
            set_string(v.substr(0, count));
        }
        else {
            set_string(v);
        }
    }
    virtual ~field_value() {}


    inline field_value& operator = (const field_value& other)
    {
        if (this != &other) {
            _type = other.get_type();
            _value = other.get_value();
        }
        return *this;
    }

    inline void clear()
    {
        _type = FIELD_UNKNOWN;
        _value.clear();
    }

    std::vector<unsigned char> serialize(unsigned long field_size) const
    {
        std::vector<unsigned char> buf;

        if (empty()) {
            return buf;
        }

        switch (_type)
        {
        case FIELD_UNSIGNED_INTEGER:
        case FIELD_INTEGER:
            assert(field_size == sizeof(__int64));
            buf.resize(field_size, 0);
            memcpy(buf.data(), &get_uvalue()._ull, sizeof(__int64));
            break;
        case FIELD_DOUBLE:
            assert(field_size == sizeof(double));
            buf.resize(field_size, 0);
            memcpy(buf.data(), &get_uvalue()._f, sizeof(double));
            break;
        case FIELD_BOOL:
            assert(field_size == sizeof(bool));
            buf.resize(field_size, 0);
            memcpy(buf.data(), &get_uvalue()._b, sizeof(bool));
            break;
        case FIELD_CHAR:
            assert(field_size >= (_value.size() / sizeof(wchar_t)));
            buf.resize(field_size * sizeof(wchar_t), 0);
            if (!_value.empty()) {
                memcpy(buf.data(), _value.data(), _value.size());
            }
            break;
        default:
            break;
        }

        return std::move(buf);
    }

    inline bool empty() const { return _value.empty(); }
    inline LOGDB_FIELD_TYPE get_type() const { return _type; }
    inline const std::vector<unsigned char>& get_value() const { return _value; }

    inline bool is_number() const { return (_type == FIELD_UNSIGNED_INTEGER || _type == FIELD_INTEGER || _type == FIELD_DOUBLE); }
    inline bool is_signed_number() const { return (_type == FIELD_INTEGER || _type == FIELD_DOUBLE); }
    inline bool is_integer() const { return (_type == FIELD_UNSIGNED_INTEGER || _type == FIELD_INTEGER); }
    inline bool is_decimal() const { return (_type == FIELD_DOUBLE); }
    inline bool is_boolean() const { return (_type == FIELD_BOOL); }
    inline bool is_string() const { return (_type == FIELD_CHAR); }

    int field_value::as_int() const
    {
        switch (get_type())
        {
        case FIELD_INTEGER:
        case FIELD_UNSIGNED_INTEGER:
            return get_uvalue()._l;
        case FIELD_DOUBLE:
            return (int)(get_uvalue()._f);
        case FIELD_BOOL:
            return get_uvalue()._b ? 1 : 0;
        default:
            throw std::exception("bad type");
        }
        // should never reach here
        __assume(0);
    }

    __int64 field_value::as_int64() const
    {
        switch (get_type())
        {
        case FIELD_INTEGER:
        case FIELD_UNSIGNED_INTEGER:
            return get_uvalue()._ll;
        case FIELD_DOUBLE:
            return (__int64)(get_uvalue()._f);
        case FIELD_BOOL:
            return get_uvalue()._b ? 1 : 0;
        default:
            throw std::exception("bad type");
        }
        // should never reach here
        __assume(0);
    }

    unsigned int field_value::as_uint() const
    {
        switch (get_type())
        {
        case FIELD_INTEGER:
        case FIELD_UNSIGNED_INTEGER:
            return get_uvalue()._ul;
        case FIELD_DOUBLE:
            return (unsigned int)(get_uvalue()._f);
        case FIELD_BOOL:
            return get_uvalue()._b ? 1 : 0;
        default:
            throw std::exception("bad type");
        }
        // should never reach here
        __assume(0);
    }

    unsigned __int64 field_value::as_uint64() const
    {
        switch (get_type())
        {
        case FIELD_INTEGER:
        case FIELD_UNSIGNED_INTEGER:
            return get_uvalue()._ull;
        case FIELD_DOUBLE:
            return (unsigned __int64)(get_uvalue()._f);
        case FIELD_BOOL:
            return get_uvalue()._b ? 1 : 0;
        default:
            throw std::exception("bad type");
        }
        // should never reach here
        __assume(0);
    }

    double field_value::as_double() const
    {
        switch (get_type())
        {
        case FIELD_INTEGER:
        case FIELD_UNSIGNED_INTEGER:
            return get_uvalue()._ul;
            return (get_uvalue()._ll * 1.0);
        case FIELD_DOUBLE:
            return get_uvalue()._f;
        case FIELD_BOOL:
            return get_uvalue()._b ? 1.0 : 0.0;
        default:
            throw std::exception("bad type");
        }
        // should never reach here
        __assume(0);
    }

    bool field_value::as_boolean() const
    {
        switch (get_type())
        {
        case FIELD_INTEGER:
        case FIELD_UNSIGNED_INTEGER:
        case FIELD_DOUBLE:
            return (get_uvalue()._ull == 0) ? false : true;
        case FIELD_BOOL:
            return get_uvalue()._b;
        default:
            throw std::exception("bad type");
        }
        // should never reach here
        __assume(0);
    }

    std::wstring field_value::as_string() const
    {
        if (FIELD_CHAR != get_type()) {
            throw std::exception("bad type");
        }

        const size_t count = get_value().size() / sizeof(wchar_t);

        if (0 == count) {
            return std::wstring();
        }

        const wchar_t* p = (const wchar_t*)get_value().data();
        return std::wstring(p, p + count);
    }

    bool field_value::operator == (const field_value& other) const
    {
        return (0 == compare(other));
    }

    bool field_value::operator > (const field_value& other) const
    {
        return (0 < compare(other));
    }

    bool field_value::operator >= (const field_value& other) const
    {
        return (0 <= compare(other));
    }

    bool field_value::operator < (const field_value& other) const
    {
        return (0 > compare(other));
    }

    bool field_value::operator <= (const field_value& other) const
    {
        return (0 >= compare(other));
    }

    bool field_value::match(const field_value& other) const
    {
        if (is_string() && other.is_string()) {
            // regex match
            return false;
        }
        return false;
    }

private:
    inline const UVALUE& get_uvalue() const
    {
        if (_value.size() != sizeof(UVALUE)) {
            throw std::exception("bad type");
        }
        return *((const UVALUE*)_value.data());
    }

    inline UVALUE& get_uvalue()
    {
        if (_value.size() != sizeof(UVALUE)) {
            throw std::exception("bad type");
        }
        return *((UVALUE*)_value.data());
    }

    inline int double_compare(double x, double y) const
    {
        double maxXY = max(std::fabs(x), std::fabs(y));
        return (std::fabs(x - y) <= std::numeric_limits<double>::epsilon()*maxXY) ? 0 : (x > y ? 1 : -1);
    }

    int field_value::compare(const field_value& other) const
    {
        if (is_string()) {
            assert(other.is_string());
            const std::wstring& s1 = as_string();
            const std::wstring& s2 = other.as_string();
            return _wcsicmp(s1.c_str(), s2.c_str());
        }
        else {
            if (is_signed_number()) {
                return (as_int64() == other.as_int64()) ? 0 : ((as_int64() > other.as_int64()) ? 1 : -1);
            }
            else {
                return (as_uint64() == other.as_uint64()) ? 0 : ((as_uint64() > other.as_uint64()) ? 1 : -1);
            }
        }
    }

    inline void set_uvalue(int v)
    {
        _value.resize(sizeof(UVALUE), 0);
        get_uvalue()._l = v;
    }

    inline void set_uvalue(__int64 v)
    {
        _value.resize(sizeof(UVALUE), 0);
        get_uvalue()._ll = v;
    }

    inline void set_uvalue(unsigned int v)
    {
        _value.resize(sizeof(UVALUE), 0);
        get_uvalue()._ul = v;
    }

    inline void set_uvalue(unsigned __int64 v)
    {
        _value.resize(sizeof(UVALUE), 0);
        get_uvalue()._ull = v;
    }

    inline void set_uvalue(double v)
    {
        _value.resize(sizeof(UVALUE), 0);
        get_uvalue()._f = v;
    }

    inline void set_uvalue(bool v)
    {
        _value.resize(sizeof(UVALUE), 0);
        get_uvalue()._b = v;
    }

    inline void set_string(const std::wstring& s)
    {
        if (s.empty()) {
            _value.clear();
        }
        else {
            _value.resize(s.length() * sizeof(wchar_t), 0);
            memcpy(_value.data(), s.c_str(), s.length() * sizeof(wchar_t));
        }
    }

private:
    LOGDB_FIELD_TYPE    _type;
    std::vector<unsigned char>  _value;
};

class db_record
{
public:
    db_record() : _id(0xFFFFFFFF)
    {
    }

    explicit db_record(unsigned long id, const std::vector<field_value>& fields, const std::vector<unsigned char>& extra_data)
        : _id(id), _fields(fields), _opaque(extra_data)
    {
    }

    ~db_record()
    {
    }

    db_record& operator = (const db_record& other)
    {
        if (this != &other) {
            _id = other.get_id();
            _fields = other.get_fields();
            _opaque = other.get_opaque_data();
        }
        return *this;
    }

    inline bool empty() const { return _fields.empty(); }
    inline unsigned long get_id() const { return _id; }
    inline const std::vector<field_value>& get_fields() const { return _fields; }
    inline const std::vector<unsigned char>& get_opaque_data() const { return _opaque; }
    inline void clear() { _id = 0xFFFFFFFF; _fields.clear(); _opaque.clear(); }

    std::vector<unsigned char> serialize(const record_layout& layout) const
    {
        std::vector<unsigned char> buf;
        assert(!layout.empty());
        if (!empty()) {
            assert(this->get_fields().size() == layout.get_fields_def().size());
            for (int i = 0; i < (int)layout.get_fields_def().size(); i++) {
                const NXLOGDB::field_value& value = get_fields()[i];
                const NXLOGDB::field_definition& def = layout.get_fields_def()[i];
                assert(value.get_type() == def.get_field_type());
                const std::vector<unsigned char>& field_buf = value.serialize(def.get_field_size());
                std::for_each(field_buf.begin(), field_buf.end(), [&](const unsigned char& ch) {
                    buf.push_back(ch);
                });
            }
            const unsigned long opaque_data_size = (unsigned long)_opaque.size();
            const unsigned char* p = (const unsigned char*)&opaque_data_size;
            for (int i = 0; i < 4; i++) {
                buf.push_back(p[i]);
            }
            std::for_each(_opaque.begin(), _opaque.end(), [&](const unsigned char& ch) {
                buf.push_back(ch);
            });
        }
        return std::move(buf);
    }

private:
    unsigned long   _id;
    std::vector<field_value>  _fields;
    std::vector<unsigned char>  _opaque;
};

class db_query_condition
{
public:
    db_query_condition() {}
    virtual ~db_query_condition() {}
    virtual bool compare(const field_value& value) const noexcept = 0;

private:
    db_query_condition& operator = (const db_query_condition& other) { return *this; }
};

class group_condition : public db_query_condition
{
public:
    typedef enum LOGIC_OPERATOR {
        AND = 0,
        OR
    } LOGIC_OPERATOR;

    group_condition() : _logic_operator(group_condition::AND)
    {
    }

    group_condition(const std::vector<std::shared_ptr<db_query_condition>>& conditions, LOGIC_OPERATOR logic_operator)
        : _conditions(conditions), _logic_operator(logic_operator)
    {
    }

    virtual ~group_condition()
    {
    }

    virtual bool group_condition::compare(const NXLOGDB::field_value& value) const noexcept
    {
        if (_conditions.empty()) {
            return true;
        }

        bool result = false;

        if (is_or()) {
            // any succeed
            auto pos = std::find_if(_conditions.begin(), _conditions.end(), [&](const std::shared_ptr<db_query_condition>& sp) -> bool {
                return sp->compare(value);
            });
            result = (pos != _conditions.end());
        }
        else {
            // all succeed
            auto pos = std::find_if_not(_conditions.begin(), _conditions.end(), [&](const std::shared_ptr<db_query_condition>& sp) -> bool {
                return sp->compare(value);
            });
            result = (pos == _conditions.end());
        }

        return result;
    }

private:
    inline bool is_and() const { return (AND == _logic_operator); }
    inline bool is_or() const { return (OR == _logic_operator); }
    // no copy allowed
    group_condition& operator = (const group_condition& other) { return *this; }


private:
    LOGIC_OPERATOR    _logic_operator;
    std::vector<std::shared_ptr<db_query_condition>>    _conditions;
};

class value_condition : public db_query_condition
{
public:
    typedef enum _COMPARE_METHOD {
        EqualTo = 0,
        GreaterThan,
        GreaterThanOrEqualTo,
        LessThan,
        LessThanOrEqualTo,
        Like
    } COMPARE_METHOD;

    value_condition() : _field_id(MAX_ULONG), _compare_method(value_condition::EqualTo)
    {
    }

    value_condition(unsigned long field_id, COMPARE_METHOD method, const field_value& value)
        : _field_id(field_id), _compare_method(method), _compare_value(value)
    {
    }

    virtual ~value_condition()
    {
    }

    virtual bool compare(const field_value& value) const noexcept
    {
        bool result = false;

        if (value.get_type() != get_compare_value().get_type()) {
            return result;
        }

        try {

            switch (get_compare_method())
            {
            case EqualTo:
                result = (value == get_compare_value());
                break;
            case GreaterThan:
                result = (value > get_compare_value());
                break;
            case GreaterThanOrEqualTo:
                result = (value >= get_compare_value());
                break;
            case LessThan:
                result = (value < get_compare_value());
                break;
            case LessThanOrEqualTo:
                result = (value <= get_compare_value());
                break;
            case Like:
                result = (value.match(get_compare_value()));
                break;
            default:
                break;
            }
        }
        catch (...) {
            result = false;
        }

        return result;
    }

    inline unsigned long get_field_id() const { return _field_id; }
    inline COMPARE_METHOD get_compare_method() const { return _compare_method; }
    inline const field_value& get_compare_value() const { return _compare_value; }

private:
    // no copy allowed
    value_condition& operator = (const value_condition& other) { return *this; }

private:
    unsigned long   _field_id;
    COMPARE_METHOD  _compare_method;
    field_value   _compare_value;
};

class db_conf
{
public:
    db_conf() :
        _db_size(0),
        _db_flags(0),
        _block_size(0),
        _sector_size(0),
        _sector_data_size(0),
        _fields_size(0),
        _record_map_offset(0),
        _record_map_size(0),
        _record_count(0),
        _max_record_count(0),
        _sector_map_offset(0),
        _sector_map_size(0),
        _sector_count(0),
        _sector_start_offset(0)
    {
    }

    db_conf(const std::string& description, unsigned long db_size, unsigned long db_flags, unsigned long sector_size, unsigned long block_size, const NXLOGDB::record_layout& layout) :
        _description(description.length() >= MAX_DESCRIPTION_SIZE ? description.substr(0, MAX_DESCRIPTION_SIZE - 1) : description),
        _db_size(db_size * 1048576/*ONE_MEGA_BYTES*/),
        _db_flags(db_flags),
        _block_size(block_size),
        _sector_size(sector_size),
        _sector_data_size(sector_size - sizeof(unsigned long)),
        _record_layout(layout),
        _fields_size(layout.get_fields_size()),
        _record_map_offset(0),
        _record_map_size(0),
        _record_start_id(0),
        _record_count(0),
        _sector_map_offset(0),
        _sector_map_size(0),
        _sector_count(0),
        _sector_start_offset(0)
    {
        try {
            initialize();
        }
        catch (...) {
            clear();
        }
    }

    virtual ~db_conf()
    {
    }

    void clear()
    {
        _record_layout.clear();
        _description.clear();
        _db_size = 0;
        _db_flags = 0;
        _block_size = 0;
        _sector_size = 0;
        _fields_size = 0;
        _record_map_offset = 0;
        _record_map_size = 0;
        _record_start_id = 0;
        _record_count = 0;
        _sector_map_offset = 0;
        _sector_map_size = 0;
        _sector_count = 0;
        _sector_start_offset = 0;
    }
    db_conf& operator = (const db_conf& other)
    {
        if (this != &other) {
            _record_layout = other.get_record_layout();
            _description = other.get_description();
            _db_size = other.get_db_size();
            _db_flags = other.get_db_flags();
            _block_size = other.get_block_size();
            _sector_size = other.get_sector_size();
            _sector_data_size = other.get_sector_data_size();
            _fields_size = other.get_fields_size();
            _record_map_offset = other.get_record_map_offset();
            _record_map_size = other.get_record_map_size();
            _record_start_id = other.get_record_start_id();
            _record_count = other.get_record_count();
            _sector_map_offset = other.get_sector_map_offset();
            _sector_map_size = other.get_sector_map_size();
            _sector_count = other.get_sector_count();
            _sector_start_offset = other.get_sector_start_offset();
        }
        return *this;
    }


    inline bool empty() const { return (0 == _db_size); }
    inline bool record_empty() const { return (_record_count == 0); }
    inline bool record_full() const { return (_record_count >= get_max_record_count()); }

    inline const std::string& get_description() const { return _description; }
    inline unsigned long get_db_size() const { return _db_size; }
    inline unsigned long get_db_flags() const { return _db_flags; }
    inline unsigned long get_block_size() const { return _block_size; }
    inline unsigned long get_sector_size() const { return _sector_size; }
    inline unsigned long get_sector_data_size() const { return _sector_data_size; }
    inline unsigned long get_fields_size() const { return _fields_size; }
    inline unsigned long get_record_map_offset() const { return _record_map_offset; }
    inline unsigned long get_record_map_size() const { return _record_map_size; }
    inline unsigned long get_record_start_id() const { return _record_start_id; }
    inline unsigned long get_record_count() const { return _record_count; }
    inline unsigned long get_max_record_count() const { return (_sector_count - 1); }
    inline unsigned long get_sector_map_offset() const { return _sector_map_offset; }
    inline unsigned long get_sector_map_size() const { return _sector_map_size; }
    inline unsigned long get_sector_count() const { return _sector_count; }
    inline unsigned long get_sector_start_offset() const { return _sector_start_offset; }
    inline const NXLOGDB::record_layout& get_record_layout() const { return _record_layout; }

    inline void set_description(const std::string& description) { _description = description.length() >= MAX_DESCRIPTION_SIZE ? description.substr(0, MAX_DESCRIPTION_SIZE - 1) : description; }
    inline void set_db_size(unsigned long db_size) { _db_size = db_size; }
    inline void set_db_flags(unsigned long db_flags) { _db_flags = db_flags; }
    inline void set_block_size(unsigned long block_size) { _block_size = block_size; }
    inline void set_sector_size(unsigned long sector_size) { _sector_size = sector_size; }
    inline void set_sector_data_size(unsigned long sector_data_size) { _sector_data_size = sector_data_size; }
    inline void set_fields_size(unsigned long fields_size) { _fields_size = fields_size; }
    inline void set_record_map_offset(unsigned long record_map_offset) { _record_map_offset = record_map_offset; }
    inline void set_record_map_size(unsigned long record_map_size) { _record_map_size = record_map_size; }
    inline void set_record_start_id(unsigned long id) { _record_start_id = id; }
    inline void set_record_count(unsigned long count) { _record_count = count; }
    inline void set_sector_map_offset(unsigned long sector_map_offset) { _sector_map_offset = sector_map_offset; }
    inline void set_sector_map_size(unsigned long sector_map_size) { _sector_map_size = sector_map_size; }
    inline void set_sector_count(unsigned long sector_count) { _sector_count = sector_count; }
    inline void set_sector_start_offset(unsigned long sector_start_offset) { _sector_start_offset = sector_start_offset; }
    inline void set_record_layout(const NXLOGDB::record_layout& layout) { _record_layout = layout; }

private:
    void initialize()
    {
        assert(0 != _db_size);
        assert(0 != _block_size);
        assert(0 != _sector_size);
        assert(!_record_layout.empty());

        if (0 == _db_size || 0 == _block_size || 0 == _sector_size || _record_layout.empty() || 0 == _fields_size) {
            throw std::exception("bad parameter");
        }

        // sanity check
        if (_db_size > 0x40000000 /*ONE_GIGA_BYTES*/) {
            throw std::exception("max_data size is greater than 1 GB");
        }

        if (_sector_size != SECTOR_512 && _sector_size != SECTOR_1024 && _sector_size != SECTOR_2048 && _sector_size != SECTOR_4096) {
            throw std::exception("invalid sector size");
        }

        if (_record_layout.get_fields_def().size() > MAX_FIELDS_NUMBER) {
            throw std::exception("too many fields, maximum fields number is 56");
        }

        // good, now calculate other offset & size
        _sector_count = _db_size / _sector_size;

        _record_map_size = _sector_count * sizeof(unsigned long);
        _sector_map_size = ((_sector_count + 31) / 32) * sizeof(unsigned long);

        _record_map_offset = DB_HEADER_SIZE;
        _sector_map_offset = _record_map_offset + _record_map_size;
        _sector_start_offset = _sector_map_offset + _sector_map_size;
        _sector_start_offset = ROUND_TO_SIZE(_sector_start_offset, _sector_size);
    }

private:
    std::string     _description;
    unsigned long   _db_size;
    unsigned long   _db_flags;
    unsigned long   _block_size;
    unsigned long   _sector_size;
    unsigned long   _sector_data_size;
    unsigned long   _fields_size;
    unsigned long   _record_map_offset;
    unsigned long   _record_map_size;
    unsigned long   _record_start_id;
    unsigned long   _record_count;
    unsigned long   _sector_map_offset;
    unsigned long   _sector_map_size;
    unsigned long   _sector_count;
    unsigned long   _sector_start_offset;
    NXLOGDB::record_layout  _record_layout;

    unsigned long _max_record_count;
};

NXLOGDB_API bool __stdcall db_conf_validate(const db_conf& conf);

class logdb_file;
// This class is exported from the nxlogdb.dll
class NXLOGDB_API nxlogdb
{
public:
    nxlogdb();
    virtual ~nxlogdb();
    
    typedef enum QUERY_DIRECTION {
        ASCENDING = 0,
        DESCENDING
    } QUERY_DIRECTION;

    void create(const std::wstring& file, const db_conf& conf);
    void open(const std::wstring& file, bool read_only = false);
    void close();
    const db_conf& get_conf() const;
    
    // query fixed size fields only
    // it is faster
    std::vector<db_record> query(std::shared_ptr<db_query_condition> query_condition,
        unsigned long start_pos = 0,
        bool descending = true,
        unsigned long count_to_get = MAX_ULONG);
    std::vector<db_record> query(std::shared_ptr<db_query_condition> query_condition,
        OPAQUE_DATA_QUERY opaque_query,
        unsigned long start_pos = 0,
        bool descending = true,
        unsigned long count_to_get = MAX_ULONG);
    unsigned long get_record_count() const;

    // record manipulate
    db_record read_record(unsigned long record_id);
    void push_record(const db_record& record);
    void pop_record();

    // dump
    void export_log(const std::wstring& file) const;

    inline bool opened() const { return (INVALID_HANDLE_VALUE != _h); }
    
private:
    HANDLE          _h;
    logdb_file*     _dbfile;
};


}   // namespace NXLOGDB

#endif  // __NXLOGDB_HPP__