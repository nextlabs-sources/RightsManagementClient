

#ifndef __NXREST_XSTYPE_HWINFO_HPP__
#define __NXREST_XSTYPE_HWINFO_HPP__

#include <string>
#include <vector>

namespace nxrmrest {
namespace schema {
namespace type {


class CCpuInfo
{
public:
    CCpuInfo() : _physnum(0), _cores(0), _logicalprocessors(0)
    {
    }

    virtual ~CCpuInfo()
    {
    }
    
    inline const std::wstring& GetBrand() const throw() {return _brand;}
    inline void SetBrand(const std::wstring& brand) throw() {_brand = brand;}
    inline const std::wstring& GetArch() const throw() {return _arch;}
    inline void SetArch(const std::wstring& arch) throw() {_arch = arch;}
    inline int GetPhysNum() const throw() {return _physnum;}
    inline void SetPhysNum(int num) throw() {_physnum = num;}
    inline int GetCores() const throw() {return _cores;}
    inline void SetCores(int cores) throw() {_cores = cores;}
    inline int GetLogicalProcessors() const throw() {return _logicalprocessors;}
    inline void SetLogicalProcessors(int processors) throw() {_logicalprocessors = processors;}

    CCpuInfo& operator = (const CCpuInfo& info) throw()
    {
        if(this != & info) {
            _brand = info.GetBrand();
            _arch = info.GetArch();
            _physnum = info.GetPhysNum();
            _cores = info.GetCores();
            _logicalprocessors = info.GetLogicalProcessors();
        }
        return *this;
    }

private:
    std::wstring    _brand;
    std::wstring    _arch;
    int             _physnum;
    int             _cores;
    int             _logicalprocessors;
};

class CMemInfo
{
public:
    CMemInfo() : _total(0), _total_vm(0), _l1_count(0), _l1_size(0), _l2_count(0), _l2_size(0), _l3_count(0), _l3_size(0)
    {
    }

    virtual ~CMemInfo()
    {
    }

    inline int GetTotal() const throw() {return _total;}
    inline void SetTotal(int total) throw() {_total = total;}
    inline int GetTotalVm() const throw() {return _total_vm;}
    inline void SetTotalVm(int total_vm) throw() {_total_vm = total_vm;}
    inline int GetL1Count() const throw() {return _l1_count;}
    inline void SetL1Count(int count) throw() {_l1_count = count;}
    inline int GetL1Size() const throw() {return _l1_size;}
    inline void SetL1Size(int size) throw() {_l1_size = size;}
    inline int GetL2Count() const throw() {return _l2_count;}
    inline void SetL2Count(int count) throw() {_l2_count = count;}
    inline int GetL2Size() const throw() {return _l2_size;}
    inline void SetL2Size(int size) throw() {_l2_size = size;}
    inline int GetL3Count() const throw() {return _l3_count;}
    inline void SetL3Count(int count) throw() {_l3_count = count;}
    inline int GetL3Size() const throw() {return _l3_size;}
    inline void SetL3Size(int size) throw() {_l3_size = size;}

    CMemInfo& operator = (const CMemInfo& info) throw()
    {
        if(this != & info) {
            _total = info.GetTotal();
            _total_vm = info.GetTotalVm();
            _l1_count = info.GetL1Count();
            _l1_size = info.GetL1Size();
            _l2_count = info.GetL2Count();
            _l2_size = info.GetL2Size();
            _l3_count = info.GetL3Count();
            _l3_size = info.GetL3Size();
        }
        return *this;
    }

private:
    int     _total;
    int     _total_vm;
    int     _l1_count;
    int     _l1_size;
    int     _l2_count;
    int     _l2_size;
    int     _l3_count;
    int     _l3_size;
};
    
class CHwInfo
{
public:
    CHwInfo()
    {
    }

    virtual ~CHwInfo()
    {
    }

    inline const CCpuInfo& GetCpuInfo() const throw() {return _cpu;}
    inline void SetCpuInfo(const CCpuInfo& info) throw() {_cpu=info;}
    inline const CMemInfo& GetMemInfo() const throw() {return _memory;}
    inline void SetMemInfo(const CMemInfo& info) throw() {_memory=info;}
    inline const std::wstring& GetDeviceId() const throw() {return _deviceId;}
    inline void SetDeviceId(const std::wstring& id) throw() {_deviceId=id;}
    inline const std::vector<std::wstring>& GetMacAddresses() const throw() {return _macaddrs;}
    inline void SetMacAddresses(const std::vector<std::wstring>& macaddrs) throw() {_macaddrs=macaddrs;}
    
    CHwInfo& operator = (const CHwInfo& info)
    {
        if(this != &info) {
            _cpu = info.GetCpuInfo();
            _memory = info.GetMemInfo();
            _deviceId = info.GetDeviceId();
            _macaddrs = info.GetMacAddresses();
        }
        return *this;
    }

private:
    CCpuInfo     _cpu;
    CMemInfo     _memory;
    std::wstring _deviceId;
    std::vector<std::wstring> _macaddrs;
};


}   // namespace nxrmrest::schema::type
}   // namespace nxrmrest::schema
}   // namespace nxrmrest


#endif  // __NXREST_XSTYPE_HWINFO_HPP__