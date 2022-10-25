

#include <Windows.h>
#include <assert.h>

#include <nudf\secure.hpp>
#include <nudf\asyncpipe.hpp>


using namespace NX;


static const std::wstring pipe_name_prefix(L"\\\\.\\pipe\\");
const unsigned long NX::async_pipe::server::default_buffer_size = 4096;

namespace NX {
namespace async_pipe {
namespace details {


class instance
{
    OVERLAPPED                  _overlap;
    HANDLE                      _pipe;
    async_pipe::server*         _server;
    unsigned long               _bytes_available;
    std::vector<unsigned char>  _buffer;

public:
    instance() : _pipe(INVALID_HANDLE_VALUE), _server(NULL), _bytes_available(0)
    {
        _buffer.resize(NX::async_pipe::server::default_buffer_size, 0);
        memset(&_overlap, 0, sizeof(_overlap));
    }

    instance(HANDLE pipe, async_pipe::server* server, unsigned long buffer_size) : _pipe(pipe), _server(server), _bytes_available(0)
    {
        _buffer.resize(buffer_size, 0);
        memset(&_overlap, 0, sizeof(_overlap));
    }

    ~instance()
    {
        if (INVALID_HANDLE_VALUE != _pipe) {
            CloseHandle(_pipe);
            _pipe = INVALID_HANDLE_VALUE;
        }
        _buffer.clear();
    }

    inline unsigned long bytes_available() const noexcept { return _bytes_available; }
    inline void set_bytes_available(unsigned long size) noexcept
    {
        _bytes_available = size;
        // make sure data is swiped
        if (_bytes_available < (unsigned long)_buffer.size()) {
            memset(&_buffer[_bytes_available], 0, ((unsigned long)_buffer.size() - _bytes_available));
        }
    }

    inline HANDLE pipe() { return _pipe; }
    inline async_pipe::server* server() { return _server; }
    inline unsigned char* buffer() { return _buffer.empty() ? NULL : (&_buffer[0]); }
    inline unsigned long buffer_size() const { return (unsigned long)_buffer.size(); }
};


}   // namespace NX::async_pipe::details
}   // namespace NX::async_pipe
}   // namespace NX


//
//  class NX::async_pipe::server
//

async_pipe::server::server() : _buffer_size(4096), _timeout(3000), _shutting_down(false)
{
    memset(&_overlap, 0, sizeof(_overlap));
    _overlap.hEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
}

async_pipe::server::server(unsigned long buffer_size, unsigned long timeout) : _buffer_size(buffer_size), _timeout(timeout), _shutting_down(false)
{
    memset(&_overlap, 0, sizeof(_overlap));
    _overlap.hEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
}

async_pipe::server::~server()
{
    if (NULL != _overlap.hEvent) {
        CloseHandle(_overlap.hEvent);
    }
    memset(&_overlap, 0, sizeof(_overlap));
}

void async_pipe::server::listen(const std::wstring& port)
{
    _name = pipe_name_prefix + port;

    if (NULL == _overlap.hEvent) {
        throw std::exception("invalid connection event");
    }

    _listen_thread = std::thread(async_pipe::server::main_worker, this);
}

void async_pipe::server::shutdown()
{
    _shutting_down = true;
    SetEvent(_overlap.hEvent);
    if (_listen_thread.joinable()) {
        _listen_thread.join();
    }
    _shutting_down = false;
}

HANDLE async_pipe::server::listen_to_pipe(bool* pending) noexcept
{
    HANDLE          pipe = INVALID_HANDLE_VALUE;

    static const unsigned long def_open_mode = PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED;
    static const unsigned long def_pipe_mode = PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT;
    static NX::sa_everyone  sa(GENERIC_READ|GENERIC_WRITE);

    try {

        pipe = ::CreateNamedPipeW(_name.c_str(), def_open_mode, def_pipe_mode, PIPE_UNLIMITED_INSTANCES, _buffer_size, _buffer_size, _timeout, sa);
        if (pipe == INVALID_HANDLE_VALUE) {
            throw std::exception("fail to create main pipe");
        }

        // Overlapped ConnectNamedPipe should return zero. 
        if (0 != ::ConnectNamedPipe(pipe, &_overlap)) {
            throw std::exception("fail to connect to main pipe");
        }


        unsigned long last_error = GetLastError();
        if (ERROR_IO_PENDING == last_error) {
            *pending = true;
        }
        else if (ERROR_PIPE_CONNECTED == last_error) {
            if (!SetEvent(_overlap.hEvent)) {
                throw std::exception("fail to set connect event");
            }
        }
        else {
            // Unknown Error Code
            throw std::exception("unknown error");
        }
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        if (pipe != INVALID_HANDLE_VALUE) {
            CloseHandle(pipe);
            pipe = INVALID_HANDLE_VALUE;
        }
    }

    return pipe;
}

void async_pipe::server::accept(HANDLE pipe, OVERLAPPED* overlap, bool pending) noexcept
{
    NX::async_pipe::details::instance* inst = NULL;

    try {

        if (pending) {
            unsigned long overlap_result = 0;
            if (!::GetOverlappedResult(pipe, overlap, &overlap_result, FALSE)) {
                throw std::exception("fail to get overlap result");
            }
        }

        // Allocate storage for this instance.
        inst = new NX::async_pipe::details::instance(pipe, this, _buffer_size);
        if (NULL == inst) {
            throw std::exception("fail to allocate instance");
        }
        pipe = INVALID_HANDLE_VALUE;

        // beging with read data from client
        read(inst);
        inst = NULL;    // inst ownership has been transfered to completion routine
    }
    catch (std::exception& e) {
        UNREFERENCED_PARAMETER(e);
        if (INVALID_HANDLE_VALUE != pipe) {
            CloseHandle(pipe);
        }
        if (NULL != inst) {
            delete inst;
            inst = NULL;
        }
    }
}

void async_pipe::server::read(void* context) noexcept
{
    NX::async_pipe::details::instance* inst = (NX::async_pipe::details::instance*)context;

    assert(NULL != inst);
    if (NULL == context) {
        return;
    }

    if (!ReadFileEx(inst->pipe(),
                    inst->buffer(),
                    inst->buffer_size(),
                    (LPOVERLAPPED)inst,
                    // LAMBDA is a READ completion routine
                    static_cast<LPOVERLAPPED_COMPLETION_ROUTINE>([](unsigned long error, unsigned long bytes_read, LPOVERLAPPED overlap)->void {

                        NX::async_pipe::details::instance* inst_read = (NX::async_pipe::details::instance*)overlap;
                        bool write_response = false;

                        if ((error == 0) && (bytes_read != 0)) {
                            inst_read->set_bytes_available(bytes_read);
                            assert(0 != inst_read->bytes_available());
                            inst_read->server()->on_read(inst_read->buffer(), &bytes_read, &write_response);
                            if (write_response) {
                                // make sure the data available in buffer is not zero
                                assert(0 != bytes_read);
                                // set available data size in the instance
                                inst_read->set_bytes_available(bytes_read);
                            }
                        }
                        else {
                            // error happened or no data from client
                            inst_read->set_bytes_available(0);
                            write_response = false;
                        }

                        if (write_response) {

                            assert(0 != inst_read->bytes_available());

                            // a response is required?
                            if (!::WriteFileEx( inst_read->pipe(),
                                                inst_read->buffer(),
                                                inst_read->bytes_available(),
                                                (LPOVERLAPPED)inst_read,
                                                // LAMBDA is a write completion routine
                                                static_cast<LPOVERLAPPED_COMPLETION_ROUTINE>([](unsigned long error, unsigned long bytes_written, LPOVERLAPPED overlap)->void {

                                                    NX::async_pipe::details::instance* inst_write = (NX::async_pipe::details::instance*)overlap;

                                                    if ((error == 0) && (bytes_written != 0)) {
                                                        // data has been sent
                                                        // check, make sure all the data has been send
                                                        assert(inst_write->bytes_available() == bytes_written);
                                                        // clear buffer by reset available data size to zero
                                                        inst_write->set_bytes_available(0);
                                                        // read more data from client
                                                        inst_write->server()->read(inst_write);
                                                    }
                                                    else {
                                                        // error happened in previous WRITE
                                                        delete inst_write;
                                                        inst_write = NULL;
                                                    }
                                                })
                                )) {
                                // sigh, error happens when write response
                                // we need to stop here (in read completion routine)
                                write_response = false;
                            }
                        }

                        // check again
                        if (!write_response) {
                            // no response is required (there are 3 cases):
                            //      1. the server()->on_read() set this to true, normally it means client doesn't require a response
                            //      2. error happened in previous READ
                            //      3. fail to write a response
                            delete inst_read;
                            inst_read = NULL;
                        }
                    })
        )) {

        // fail to read ? release this instance
        delete inst;
        inst = NULL;
    }
}

void async_pipe::server::on_read(unsigned char* data, unsigned long* size, bool* write_response)
{
    *size = 0;
    *write_response = false;
}

void async_pipe::server::main_worker(async_pipe::server* serv) noexcept
{
    unsigned long   result = 0;
    bool            pending = FALSE;
    HANDLE          pipe = INVALID_HANDLE_VALUE;


    do {

        unsigned wait_result = 0;
        
        // if not listening, try to listen
        if (INVALID_HANDLE_VALUE == pipe) {
            pipe = serv->listen_to_pipe(&pending);
        }

        // if pipe is not valid, exit
        if (pipe == INVALID_HANDLE_VALUE) {
            return;
        }

        // wait for event or APC
        wait_result = WaitForSingleObjectEx(serv->_overlap.hEvent, INFINITE, TRUE);

        // check if server is shutting down
        if (serv->_shutting_down) {
            // shutting down
            (VOID)DisconnectNamedPipe(pipe);
            (VOID)CloseHandle(pipe);
            pipe = INVALID_HANDLE_VALUE;
            return;
        }

        // server is active, handle event
        if (WAIT_OBJECT_0 == wait_result) {
            // new client connected
            serv->accept(pipe, &serv->_overlap, pending);
            pipe = INVALID_HANDLE_VALUE;    // ownership has been transfered to accept()
            continue;
        }
        else if (WAIT_IO_COMPLETION == wait_result) {
            // APC
            continue;
        }
        else {
            // somthing bad happen
            return;
        }

    } while (!serv->_shutting_down);
}



//
//  class NX::async_pipe::client
//

async_pipe::client::client() : _pipe(INVALID_HANDLE_VALUE), _pipe_mode(0)
{
    memset(&_overlap, 0, sizeof(_overlap));
    _overlap.hEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
    _buffer.resize(NX::async_pipe::server::default_buffer_size, 0);
}

async_pipe::client::client(unsigned long buffer_size) : _pipe(INVALID_HANDLE_VALUE), _pipe_mode(0)
{
    memset(&_overlap, 0, sizeof(_overlap));
    _overlap.hEvent = ::CreateEventW(NULL, FALSE, FALSE, NULL);
    _buffer.resize(buffer_size, 0);
}

async_pipe::client::~client()
{
    disconnect();
    if (NULL != _overlap.hEvent) {
        CloseHandle(_overlap.hEvent);
    }
    memset(&_overlap, 0, sizeof(_overlap));
}

bool async_pipe::client::connect(const std::wstring& name, unsigned long timeout) noexcept
{
    DWORD dwMode = 0;
    std::wstring pipe_name = pipe_name_prefix + name;

    // support async i/o
    _pipe = CreateFileW(pipe_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
    if (INVALID_HANDLE_VALUE == _pipe) {
        // failed, and the cause is not pipe busy
        if (GetLastError() != ERROR_PIPE_BUSY) {
            return false;
        }

        // pipe is busy, wait a while
        if (!::WaitNamedPipeW(pipe_name.c_str(), timeout)) {
            // timeout? return false
            SetLastError(ERROR_TIMEOUT);
            return false;
        }

        // Try again
        _pipe = CreateFileW(pipe_name.c_str(), GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
        if (INVALID_HANDLE_VALUE == _pipe) {
            if (GetLastError() == ERROR_PIPE_BUSY) {
                // sorry, we already wait, let it fail this time
                SetLastError(ERROR_TIMEOUT);
            }
            return false;
        }
    }

    // pipe has been opened
    assert(INVALID_HANDLE_VALUE != _pipe);

    unsigned long mode = PIPE_READMODE_MESSAGE;
    if (!::SetNamedPipeHandleState(_pipe, &mode, NULL, NULL)) {
        CloseHandle(_pipe);
        _pipe = INVALID_HANDLE_VALUE;
        return false;
    }

    // good, connected
    return true;
}

void async_pipe::client::disconnect()
{
    if (INVALID_HANDLE_VALUE != _pipe) {
        CloseHandle(_pipe);
        _pipe = INVALID_HANDLE_VALUE;
    }
}

bool async_pipe::client::read(std::vector<unsigned char>& data, unsigned long timeout)
{
    data.clear();
    
    _overlap.InternalHigh = 0;
    if(!::ReadFileEx(_pipe,
                     &_buffer[0],
                     (unsigned long)_buffer.size(),
                     &_overlap,
                     // LAMBDA is a READ completion routine
                     static_cast<LPOVERLAPPED_COMPLETION_ROUTINE>([](unsigned long error, unsigned long bytes_read, LPOVERLAPPED overlap)->void {
                        assert(NULL != overlap);
                        ::SetEvent(overlap->hEvent);
                     }))) {
        // fail to call async ReadFileEx
        return false;
    }

    unsigned long wait_result = ::WaitForSingleObjectEx(_overlap.hEvent, timeout, TRUE);
    if (WAIT_OBJECT_0 == wait_result || WAIT_IO_COMPLETION == wait_result) {
        // read succeed
        ::ResetEvent(_overlap.hEvent);
        if (0 == _overlap.InternalHigh) {
            return false;
        }

        data.resize(_overlap.InternalHigh, 0);
        memcpy(&data[0], _buffer.data(), _overlap.InternalHigh);
        return true;
    }
    else if (WAIT_TIMEOUT == wait_result) {
        // timeout
        SetLastError(ERROR_TIMEOUT);
        return false;
    }
    else {
        // somthing bad happen
        return false;
    }

    // Should not reach here
    __assume(0);
    return false;
}

bool async_pipe::client::write(const std::vector<unsigned char>& data, unsigned long timeout)
{
    if (data.size() > _buffer.size()) {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return false;
    }

    memcpy(&_buffer[0], data.data(), data.size());
    
    if(!::WriteFileEx(_pipe,
                      &_buffer[0],
                      (unsigned long)data.size(),
                      &_overlap, //(),
                      // LAMBDA is a WRITE completion routine
                      static_cast<LPOVERLAPPED_COMPLETION_ROUTINE>([](unsigned long error, unsigned long bytes_written, LPOVERLAPPED overlap)->void {
                         assert(NULL != overlap);
                         assert(bytes_written == overlap->InternalHigh);
                         // cleanup data
                         ::SetEvent(overlap->hEvent);
                      }))) {
        // fail to call async WriteFileEx
        return false;
    }

    unsigned long wait_result = ::WaitForSingleObjectEx(_overlap.hEvent, timeout, TRUE);
    if (WAIT_OBJECT_0 == wait_result || WAIT_IO_COMPLETION == wait_result) {
        // write succeed
        ::ResetEvent(_overlap.hEvent);
        return true;
    }
    else if (WAIT_TIMEOUT == wait_result) {
        // timeout
        SetLastError(ERROR_TIMEOUT);
        return false;
    }
    else {
        // somthing bad happen
        return false;
    }

    // Should not reach here
    __assume(0);
    return false;
}
