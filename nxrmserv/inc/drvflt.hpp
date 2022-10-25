
#ifndef __NX_DRV_FLT_H__
#define __NX_DRV_FLT_H__


#include <string>
#include <list>
#include <memory>
#include <thread>

#include <nudf\thread.hpp>

namespace NX {
namespace drv {
    


class serv_flt
{
public:
    class flt_request
    {
    public:
        flt_request() : _type(0), _context(NULL)
        {
        }
        flt_request(unsigned long type, void* msg, unsigned long length, void* msg_context);
        ~flt_request() {}

        inline void* request() const noexcept { return (void*)(_request.empty()? NULL : _request.data()); }
        inline void* context() const noexcept { return _context; }
        inline unsigned long type() const noexcept { return _type; }
        inline bool empty() const noexcept { return _request.empty(); }


    private:
        flt_request& operator = (const flt_request& other)
        {
            assert(FALSE);
            return *this;
        }

    private:
        std::vector<unsigned char> _request;
        //void*           _request;
        unsigned long   _type;
        void*           _context;
    };

public:
    serv_flt();
    ~serv_flt();

    bool start(const void* key_blob, unsigned long size) noexcept;
    void stop() noexcept;

    inline bool started() const noexcept { return _started; }

    unsigned long set_saveas_forecast(unsigned long process_id, const wchar_t* source, const wchar_t* target) noexcept;
    unsigned long set_policy_changed() noexcept;
    unsigned long set_keys(const void* key_blob, unsigned long size) noexcept;

    static void worker(serv_flt* serv) noexcept;
    static unsigned long __stdcall drv_callback(unsigned long type, void* msg, unsigned long length, void* msg_context, void* user_context);
    static BOOL __stdcall check_log_accept(unsigned long level);
    static LONG __stdcall write_log(const wchar_t* msg);

protected:
    // Handle request
    void on_check_rights(flt_request* request);
    void on_block_notification(flt_request* request);
    void on_key_error_notification(flt_request* request);
    void on_purge_cache_notification(flt_request* request);
    void on_process_notification(flt_request* request);

    bool load_file_attributes(unsigned long session_id,
                              const std::wstring& file,
                              std::map<std::wstring, std::wstring>& attributes,
                              std::wstring& templates,
                              std::multimap<std::wstring, std::wstring>& tags
                              );

private:
    std::vector<std::thread>                _threads;
    std::list<std::shared_ptr<flt_request>> _list;
    CRITICAL_SECTION                        _list_lock;
    HANDLE                                  _fltman;
    HANDLE                                  _stop_event;
    HANDLE                                  _request_event;
    bool                                    _started;
};
       
}   // namespace NX::drv
}   // namespace NX


#endif  // #ifndef __NX_DRV_FLT_H__