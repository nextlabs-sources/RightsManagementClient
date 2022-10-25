

#ifndef __NXREST_XSTYPE_KEY_PROFILE_HPP__
#define __NXREST_XSTYPE_KEY_PROFILE_HPP__

#include <Windows.h>
#include <string>
#include <list>
#include <vector>
#include <memory>

#include <nudf\shared\keydef.h>
#include <nudf\crypto.hpp>
#include <nudf\string.hpp>


namespace nxrmrest {
namespace schema {
namespace type {

        

class CKey
{
public:
    CKey()
    {
        _timestamp.dwHighDateTime = 0;
        _timestamp.dwLowDateTime  = 0;
    }

    virtual ~CKey()
    {
    }

    inline const std::vector<UCHAR>& GetId() const throw() {return _id;}
    inline void SetId(const std::vector<UCHAR>& id) throw() {_id = id;}
    inline void SetId(const std::wstring& id) throw() {nudf::string::ToBytes<wchar_t>(id, _id);}
    inline const std::vector<UCHAR>& GetKey() const throw() {return _key;}
    inline void SetKey(const std::vector<UCHAR>& key) throw() {_key = key;}
    inline void SetKey(const std::wstring& key) throw() {nudf::string::ToBytes<wchar_t>(key, _key);}
    inline const FILETIME& GetTimestamp() const throw() {return _timestamp;}
    inline void SetId(const FILETIME& timestamp) throw() {_timestamp = timestamp;}

    CKey& operator = (const CKey& key) throw()
    {
        if(this != &key) {
            _id = key.GetId();
            _key = key.GetKey();
            _timestamp = key.GetTimestamp();
        }
        return *this;
    }

    bool operator == (const CKey& key) throw()
    {
        return (_id == key.GetId());
    }

    void Clear() throw()
    {
        _id.clear();
        _key.clear();
        _timestamp.dwHighDateTime = 0;
        _timestamp.dwLowDateTime  = 0;
    }

private:
    std::vector<UCHAR>  _id;
    std::vector<UCHAR>  _key;
    FILETIME            _timestamp;
};

class CKeyProfile
{
public:
    CKeyProfile() : _checksum(0)
    {
    }
    virtual ~CKeyProfile()
    {
    }

    inline PCNXRM_KEY_BLOB GetKeyBlob() const throw() {_keys.empty() ? NULL : ((PCNXRM_KEY_BLOB)&_keys[0]);}
    inline ULONG GetKeyCount() const throw() {return (ULONG)(_keys.size()/sizeof(NXRM_KEY_BLOB));}
    inline ULONG GetKeySize() const throw() {return (ULONG)_keys.size();}
    inline ULONGLONG GetChecksum() const throw() {return _checksum;}

    void SetKeys(_In_ const std::list<CKey>& keys)
    {
        Clear();
        if(!keys.empty()) {
            _keys.resize((keys.size() * sizeof(NXRM_KEY_BLOB)), 0);
            PNXRM_KEY_BLOB key = ((PNXRM_KEY_BLOB)&_keys[0]);
            for(std::list<CKey>::const_iterator it=keys.begin(); it!=keys.end(); ++it) {
                key->KeKeyId.Algorithm = ((*it).GetKey().size() == 32) ? NXRM_ALGORITHM_AES256 : NXRM_ALGORITHM_AES128;
                key->KeKeyId.IdSize    = (ULONG)((*it).GetId().size());
                memcpy(key->KeKeyId.Id,  &((*it).GetId()[0]), key->KeKeyId.IdSize);
                memcpy(key->Key,  &((*it).GetId()[0]), key->KeKeyId.IdSize);
            }
            _checksum = nudf::crypto::ToCrc64(0, &_keys[0], (ULONG)_keys.size());
        }
    }

    CKeyProfile& operator = (const CKeyProfile& profile)
    {
        if(this != &profile) {
            Clear();
            if(0 != profile.GetKeySize()) {
                _keys.resize(profile.GetKeySize(), 0);
                memcpy(&_keys[0], profile.GetKeyBlob(), profile.GetKeySize());
                _checksum = nudf::crypto::ToCrc64(0, &_keys[0], (ULONG)_keys.size());
            }
        }
        return *this;
    }

    void Clear() throw()
    {
        _keys.clear();
        _checksum = 0;
    }


private:
    std::vector<UCHAR>  _keys;
    ULONGLONG           _checksum;
};


}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_KEY_PROFILE_HPP__