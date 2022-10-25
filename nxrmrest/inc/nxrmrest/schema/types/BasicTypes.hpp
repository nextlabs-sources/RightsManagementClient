

#ifndef __NXREST_XSTYPE_BASIC_TYPES_HPP__
#define __NXREST_XSTYPE_BASIC_TYPES_HPP__



namespace nxrmrest {
namespace schema {
namespace type {


typedef enum _TIMEUNIT {
    Days = 0,
    Hours,
    Minutes,
    Seconds,
    Milliseconds
} TIMEUNIT;

class CTimeWithUnit
{
public:
    CTimeWithUnit() : _value(0), _unit(Seconds)
    {
    }
    CTimeWithUnit(_In_ __int64 value, _In_ TIMEUNIT unit) : _value(value), _unit(unit)
    {
    }
    CTimeWithUnit(_In_ __int64 value, _In_ const std::wstring& unit) : _value(value), _unit(Seconds)
    {
        SetUnitByName(unit);
    }

    ~CTimeWithUnit()
    {
    }

    inline __int64 GetValue() const throw() {return _value;}
    inline void SetValue(__int64 value) throw() {_value = value;}
    inline TIMEUNIT GetUnit() const throw() {return _unit;}
    inline LPCWSTR GetUnitName() const throw()
    {
        switch(_unit)
        {
        case Days: return L"Days";
        case Hours: return L"Hours";
        case Minutes: return L"Minutes";
        case Seconds: return L"Seconds";
        case Milliseconds:
        default:
            return L"Milliseconds";
        }
    }
    inline void SetUnit(TIMEUNIT unit) throw() {_unit = unit;}
    inline void SetUnitByName(const std::wstring& unit) throw()
    {
        if(0 == _wcsicmp(unit.c_str(), L"Days")) {
            _unit = Days;
        }
        else if(0 == _wcsicmp(unit.c_str(), L"Hours")) {
            _unit = Hours;
        }
        else if(0 == _wcsicmp(unit.c_str(), L"Minutes")) {
            _unit = Minutes;
        }
        else if(0 == _wcsicmp(unit.c_str(), L"Seconds")) {
            _unit = Seconds;
        }
        else {
            _unit = Milliseconds;
        }
    }

    CTimeWithUnit& operator = (const CTimeWithUnit t) throw()
    {
        if(this != &t) {
            _value = t.GetValue();
            _unit = t.GetUnit();
        }
        return *this;
    }

    __int64 ToMilliseconds() const throw()
    {
        switch(_unit)
        {
        case Days: return (_value * 86400000); // (_value * 24 * 60 * 60 * 1000);
        case Hours: return (_value * 3600000); // (_value * 1000 * 60 * 60);
        case Minutes: return (_value * 60000); // (_value * 1000 * 60);
        case Seconds: return (_value * 1000);
        case Milliseconds:
        default:
            return (_value);
        }
    }

private:
    __int64  _value;
    TIMEUNIT _unit;
};


typedef enum _SIZEUNIT {
    BYTES = 0,
    KB,
    MB,
    GB
} SIZEUNIT;

class CSizeWithUnit
{
public:
    CSizeWithUnit() : _value(0), _unit(MB)
    {
    }
    CSizeWithUnit(_In_ unsigned __int64 value, _In_ SIZEUNIT unit) : _value(value), _unit(unit)
    {
    }
    CSizeWithUnit(_In_ unsigned __int64 value, _In_ const std::wstring& unit) : _value(value), _unit(MB)
    {
        SetUnitByName(unit);
    }

    ~CSizeWithUnit()
    {
    }

    inline unsigned __int64 GetValue() const throw() {return _value;}
    inline void SetValue(unsigned __int64 value) throw() {_value = value;}
    inline SIZEUNIT GetUnit() const throw() {return _unit;}
    inline LPCWSTR GetUnitName() const throw()
    {
        switch(_unit)
        {
        case BYTES: return L"BYTES";
        case KB: return L"KB";
        case GB: return L"GB";
        case MB:
        default:
            return L"MB";
        }
    }
    inline void SetUnit(SIZEUNIT unit) throw() {_unit = unit;}
    inline void SetUnitByName(const std::wstring& unit) throw()
    {
        if(0 == _wcsicmp(unit.c_str(), L"BYTES")) {
            _unit = BYTES;
        }
        else if(0 == _wcsicmp(unit.c_str(), L"KB")) {
            _unit = KB;
        }
        else if(0 == _wcsicmp(unit.c_str(), L"GB")) {
            _unit = GB;
        }
        else {
            _unit = MB;
        }
    }

    CSizeWithUnit& operator = (const CSizeWithUnit s) throw()
    {
        if(this != &s) {
            _value = s.GetValue();
            _unit = s.GetUnit();
        }
        return *this;
    }

    unsigned __int64 ToBytes() const throw()
    {
        switch(_unit)
        {
        case GB: return (_value * 1073741824);  // (_value * 1024 * 1024 * 1024);
        case MB: return (_value * 1048576);     // (_value * 1024 * 1024);
        case KB: return (_value * 1024);
        case BYTES:
        default:
            return (_value);
        }
    }

private:
    unsigned __int64  _value;
    SIZEUNIT _unit;
};

}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_BASIC_TYPES_HPP__