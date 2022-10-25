
#pragma once
#ifndef __NX_FUNCTION_DLL_HPP__
#define __NX_FUNCTION_DLL_HPP__

#include <string>
#include <vector>
#include <algorithm>


namespace NX {


class function_item
{
public:
    function_item();
    function_item(unsigned int id, const std::string& name = std::string());
    virtual ~function_item();

    inline bool valid() const { return (-1 == _id); }
    inline bool empty() const { return (!valid() && nullptr != _fp); }
    inline unsigned int get_id() const { return _id; }
    inline const std::string& get_name() const { return _name; }
    inline void* get_function_pointer() const { return _fp; }

    function_item& operator = (const function_item& other);
    bool load(void* h);
    void clear();

private:
    unsigned int    _id;
    std::string     _name;
    void*           _fp;
};

template<unsigned long N>
class dll_instance
{
public:
    dll_instance() : _h(NULL)
    {
    }

    dll_instance(const std::vector<function_item>& items) : _h(NULL), _functions(items)
    {
        //assert(N == (unsigned long)items.size());
    }

    ~dll_instance()
    {
        unload();
    }

    virtual void load(const std::wstring& dll_file)
    {
        _h = ::LoadLibraryW(dll_file.c_str());
        if (NULL != _h) {
            std::for_each(_functions.begin(), _functions.end(), [&](function_item& item) {
                item.load(_h);
            });
        }
    }

    virtual void unload()
    {
        std::for_each(_functions.begin(), _functions.end(), [&](function_item& item) {
            item.clear();
        });
        if (NULL != _h) {
            FreeLibrary(_h);
            _h = NULL;
        }
    }

    inline bool is_loaded() const { return (NULL != _h); }
    inline void* operator[](size_t index) const
    {
        return ((index < _functions.size()) ? _functions[index].get_function_pointer() : nullptr);
    }

private:
    HMODULE _h;
    std::vector<function_item> _functions;
};

#define EXECUTE(TYPE, INSTANCE, ID, ...)     (reinterpret_cast<TYPE>((INSTANCE)[ID]))(__VA_ARGS__)


}


#endif