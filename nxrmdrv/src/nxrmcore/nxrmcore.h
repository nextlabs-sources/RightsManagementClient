// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the NXRMCORE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// NXRMCORE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef NXRMCORE_EXPORTS
#define NXRMCORE_API __declspec(dllexport)
#else
#define NXRMCORE_API __declspec(dllimport)
#endif

BOOL init_rm_section_safe(void);