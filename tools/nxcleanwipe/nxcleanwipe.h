#ifndef __NXCLEANWIPE_H__



#include <Windows.h>
#include <map>



// Program exit code
#define EXIT_INVALID_PARAMETER          (EXIT_FAILURE + 1)
#define EXIT_PLATFORM_NOT_SUPPORTED     (EXIT_FAILURE + 2)



// Abbreviations
typedef std::wstring wstr;



namespace nxcleanwipe {
    class VarStore {
    public:
        VarStore(void);
        wstr Lookup(wstr var);
        bool Resolve(wstr &str, bool hideError=false);
    private:
        std::map<const wstr, wstr> m_varMap;
    };

    class SvcWiper {
    public:
        SvcWiper(void);
        ~SvcWiper();
        bool Delete(const wchar_t *svcName);
    private:
        SC_HANDLE m_schSCM;
    };

    class AsmWiper {
    public:
        bool Delete(const wchar_t *asmFullPath, bool isX64);
    };

    class RegWiper {
    public:
        bool DeleteValue(HKEY key, const wchar_t *subKey, const wchar_t *value);
        bool DeleteTree(HKEY key, const wchar_t *subKey);
        bool DeleteKeyIfEmpty(HKEY key, const wchar_t *subKey);
    };

    class FileAssocWiper {
    public:
        bool DeleteExt(const wchar_t *ext);
        bool DeleteType(const wchar_t *type);
    };

    class FontWiper {
    public:
        bool Delete(const wchar_t *fontName);
    };

    class FileWiper {
    public:
        bool Delete(const wchar_t *fileFullPath, bool ignoreMissing=false);
    };

    class DirWiper {
    public:
        bool Delete(const wchar_t *dirFullPath, bool ignoreMissing=false);
        bool DeleteNoRecurse(const wchar_t *dirFullPath);
    };

} // namespace nxcleanwipe



#endif // __NXCLEANWIPE_H__
