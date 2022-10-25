#pragma once
#pragma pack(push, 8)

#include <comdef.h>

namespace AddInDesignerObjects {

	//
	// Forward references and typedefs
	//

	struct __declspec(uuid("ac0714f2-3d04-11d1-ae7d-00a0c90f26f4"))
		/* LIBID */ __AddInDesignerObjects;
	enum ext_ConnectMode;
	enum ext_DisconnectMode;
	struct __declspec(uuid("ac0714f3-3d04-11d1-ae7d-00a0c90f26f4"))
		/* dual interface */ IAddinDesigner;
	struct __declspec(uuid("ac0714f4-3d04-11d1-ae7d-00a0c90f26f4"))
		/* dual interface */ IAddinInstance;
	struct __declspec(uuid("b65ad801-abaf-11d0-bb8b-00a0c90f2744"))
		/* dual interface */ _IDTExtensibility2;
	struct /* coclass */ AddinDesigner;
	struct /* coclass */ AddinInstance;
	typedef struct _IDTExtensibility2 IDTExtensibility2;

	//
	// Smart pointer typedef declarations
	//

	_COM_SMARTPTR_TYPEDEF(IAddinDesigner, __uuidof(IAddinDesigner));
	_COM_SMARTPTR_TYPEDEF(IAddinInstance, __uuidof(IAddinInstance));
	_COM_SMARTPTR_TYPEDEF(_IDTExtensibility2, __uuidof(_IDTExtensibility2));

	//
	// Type library items
	//

	enum __declspec(uuid("289e9af1-4973-11d1-ae81-00a0c90f26f4"))
		ext_ConnectMode
	{
		ext_cm_AfterStartup = 0,
		ext_cm_Startup = 1,
		ext_cm_External = 2,
		ext_cm_CommandLine = 3
	};

	enum __declspec(uuid("289e9af2-4973-11d1-ae81-00a0c90f26f4"))
		ext_DisconnectMode
	{
		ext_dm_HostShutdown = 0,
		ext_dm_UserClosed = 1
	};

	struct __declspec(uuid("ac0714f3-3d04-11d1-ae7d-00a0c90f26f4"))
IAddinDesigner : IDispatch
	{};

	struct __declspec(uuid("ac0714f4-3d04-11d1-ae7d-00a0c90f26f4"))
IAddinInstance : IDispatch
	{};

	struct __declspec(uuid("b65ad801-abaf-11d0-bb8b-00a0c90f2744"))
_IDTExtensibility2 : IDispatch
	{
		//
		// Raw methods provided by interface
		//

		virtual HRESULT __stdcall OnConnection (
			/*[in]*/ IDispatch * Application,
			/*[in]*/ enum ext_ConnectMode ConnectMode,
			/*[in]*/ IDispatch * AddInInst,
			/*[in]*/ SAFEARRAY * * custom ) = 0;
		virtual HRESULT __stdcall OnDisconnection (
			/*[in]*/ enum ext_DisconnectMode RemoveMode,
			/*[in]*/ SAFEARRAY * * custom ) = 0;
		virtual HRESULT __stdcall OnAddInsUpdate (
			/*[in]*/ SAFEARRAY * * custom ) = 0;
		virtual HRESULT __stdcall OnStartupComplete (
			/*[in]*/ SAFEARRAY * * custom ) = 0;
		virtual HRESULT __stdcall OnBeginShutdown (
			/*[in]*/ SAFEARRAY * * custom ) = 0;
	};

	struct __declspec(uuid("ac0714f6-3d04-11d1-ae7d-00a0c90f26f4"))
		AddinDesigner;
	// [ default ] interface IAddinDesigner
	// [ default, source ] interface _IDTExtensibility2

	struct __declspec(uuid("ac0714f7-3d04-11d1-ae7d-00a0c90f26f4"))
		AddinInstance;
	// [ default ] interface IAddinInstance
	// [ default, source ] interface _IDTExtensibility2

} // namespace AddInDesignerObjects

#pragma pack(pop)
