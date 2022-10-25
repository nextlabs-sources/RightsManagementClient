#pragma once

#ifdef __cplusplus
extern "C"{
#endif

	#ifndef _AMD64_

		BOOL install_hook_x86(
			PVOID original_call,
			PVOID *trampoline_call,
			PVOID new_call);

		BOOL remove_hook_x86(PVOID trampoline_call);

		BOOL install_com_hook_x86(
			PVOID	*call_in_vtbl_addr,
			PVOID	new_call);

		BOOL remove_com_hook_x86(
			PVOID	*call_in_vtbl_addr,
			PVOID	old_call);

		BOOL install_export_hook_x86(
			PVOID module_base,
			PVOID original_call, 
			PVOID new_call);

		BOOL remove_export_hook_x86(
			PVOID module_base,
			PVOID new_call, 
			PVOID original_call);

		#define install_hook		install_hook_x86
		#define remove_hook			remove_hook_x86
		#define install_com_hook	install_com_hook_x86
		#define remove_com_hook		remove_com_hook_x86
		#define install_export_hook	install_export_hook_x86
		#define remove_export_hook	remove_export_hook_x86
		#define	install_hook_ex		install_hook_x86
		#define	remove_hook_ex		remove_hook_x86 

	#else

		BOOL install_hook_x64(
			PVOID original_call,
			PVOID *trampoline_call,
			PVOID new_call);
		
		BOOL remove_hook_x64(PVOID trampoline_call);

		BOOL install_com_hook_x64(
			PVOID	*call_in_vtbl_addr,
			PVOID	new_call);

		BOOL remove_com_hook_x64(
			PVOID	*call_in_vtbl_addr,
			PVOID	old_call);

		BOOL install_export_hook_x64(
			PVOID module_base,
			PVOID original_call, 
			PVOID new_call);

		BOOL remove_export_hook_x64(
			PVOID module_base,
			PVOID new_call, 
			PVOID original_call);

		#define install_hook		install_hook_x64
		#define remove_hook			remove_hook_x64
		#define install_com_hook	install_com_hook_x64
		#define remove_com_hook		remove_com_hook_x64
		#define install_export_hook	install_export_hook_x64
		#define remove_export_hook	remove_export_hook_x64
		#define	install_hook_ex		install_hook_ex_x64
		#define	remove_hook_ex		remove_hook_ex_x64 
		
		BOOL install_hook_ex_x64(
			PVOID original_call,
			PVOID *trampoline_call,
			PVOID new_call);

		BOOL remove_hook_ex_x64(PVOID trampoline_call);

	#endif

#ifdef __cplusplus
}
#endif