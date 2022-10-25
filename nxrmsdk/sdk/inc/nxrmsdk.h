


#ifndef __NXRM_SDK_H__
#define __NXRM_SDK_H__



#ifdef NXRMSDK_EXPORTS
#define NXRMSDK_API __declspec(dllexport)
#else
#define NXRMSDK_API __declspec(dllimport)
#endif


//
//  Common
//

void* WINAPI RSEAllocEx(unsigned long size, bool init, unsigned char init_ch);
void* WINAPI RSEAlloc(unsigned long size);
void WINAPI RSEFree(void* mem);

//
//  Engine
//

HANDLE WINAPI RSECreate(const wchar_t* work_dir);
void WINAPI RSEDestroy(HANDLE engine_handle);
bool WINAPI RSEConfig(HANDLE engine_handle);
bool WINAPI RSEStart(HANDLE engine_handle);
bool WINAPI RSEStop(HANDLE engine_handle);




#endif
