
#include <ntifs.h>
#include <fltkernel.h>

#include <nkdf/basic.h>
#include <nkdf/fs/fltconn.h>



typedef struct _NKFLT_PORT_OBJECT {
    const BOOLEAN   Server;
    const ULONG     Id;
    PFLT_PORT       Port;
} NKFLT_PORT_OBJECT, *PNKFLT_PORT_OBJECT;
typedef const NKFLT_PORT_OBJECT* PCNKFLT_PORT_OBJECT;

typedef struct _NKFLT_PORTS_CONTEXT {
    const ULONG         Count;
    KSPIN_LOCK          Lock;
    ULONG               ConnectBits;
    ULONG               IdleBits;
    RTL_BITMAP          ConnectMap;
    RTL_BITMAP          IdleMap;
    NKFLT_PORT_OBJECT   Ports[1];
} NKFLT_PORTS_CONTEXT, *PNKFLT_PORTS_CONTEXT;


typedef struct _NKFLT_CONN_CONTEXT {
    PFLT_FILTER             Filter;
    ULONG                   Flags;
    PFLT_PORT               ControlPort;
    PNKFLT_PORTS_CONTEXT    ServerPortsCtx;
    PNKFLT_PORTS_CONTEXT    ClientPortsCtx;
    NKFLT_CONN_MSGDISPATCH_ROUTINE MsgDispatch;
} NKFLT_CONN_CONTEXT, *PNKFLT_CONN_CONTEXT;


NKFLT_CONN_CONTEXT  NkFltConnCtx = {
    NULL,
    0,
    NULL,
    NULL,
    NULL,
    NULL
};



//
//  Local Routines
//
NTSTATUS
NkFltClientConnect (
                  _In_ PFLT_PORT ClientPort,
                  _In_ PVOID ServerPortCookie,
                  _In_ PVOID ConnectionContext,
                  _In_ ULONG SizeOfContext,
                  _Out_ PVOID *ConnectionPortCookie
                  );


VOID
NkFltClientDisconnect (
                     _In_ PVOID ConnectionCookie
                     );


_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltClientMessageDispatch (
                          _In_ PVOID PortCookie,
                          _In_opt_ PVOID InputBuffer,
                          _In_ ULONG InputBufferLength,
                          _Out_opt_ PVOID OutputBuffer,
                          _In_ ULONG OutputBufferLength,
                          _Out_ PULONG ReturnOutputBufferLength
                          );

_Check_return_
PNKFLT_PORTS_CONTEXT
InterAllocPortsContext(
                       _In_ ULONG Count,
                       _In_ BOOLEAN Server
                       );

VOID
InterFreePortsContext(
                      _In_ PNKFLT_PORTS_CONTEXT Context
                      );


//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkFltConnStart)
#pragma alloc_text(PAGE, NkFltConnShutdown)
#pragma alloc_text(PAGE, NkFltClientMessageDispatch)
#endif


__forceinline
ULONG
GetPortsContextSize(
                    _In_ ULONG Count
                    )
{
    return (ULONG)((0 == Count) ? 0 : (sizeof(NKFLT_PORTS_CONTEXT) + sizeof(PFLT_PORT) * (Count - 1)));
}

__forceinline
NTSTATUS
DefaultMsgDisptach(
                   _In_opt_ PVOID InputBuffer,
                   _In_ ULONG InputBufferLength,
                   _Out_opt_ PVOID OutputBuffer,
                   _In_ ULONG OutputBufferLength,
                   _Out_ PULONG ReturnOutputBufferLength
                   )
{
    // By default, we deny all requests
    return STATUS_INVALID_DEVICE_REQUEST;
}



_Check_return_
_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltConnStart(
               _In_ PFLT_FILTER Filter,
               _In_ PCUNICODE_STRING Name,
               _In_ ULONG ServerConn,    // Driver works as a server when use this port: it handles application's request
               _In_ ULONG ClientConn,    // Driver works as a client when use this port: it sends request to application and gets reply
               _In_opt_ NKFLT_CONN_MSGDISPATCH_ROUTINE MsgDispatchRoutine
               )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    PSECURITY_DESCRIPTOR    SecurityDescriptor = NULL;


    PAGED_CODE();


    // Sanity check
    if (ServerConn > NKFLT_MAX_CONN) {
        return STATUS_INVALID_PARAMETER;
    }
    if (ClientConn > NKFLT_MAX_CONN) {
        return STATUS_INVALID_PARAMETER;
    }
    if (0 == ClientConn && 0 == ServerConn) {
        return STATUS_INVALID_PARAMETER;
    }
    if (0 != ServerConn && NULL == MsgDispatchRoutine) {
        return STATUS_INVALID_PARAMETER;
    }

    if (NULL != NkFltConnCtx.Filter) {
        return STATUS_INVALID_DEVICE_STATE;
    }

    try {

        NkFltConnCtx.Filter = Filter;
        NkFltConnCtx.MsgDispatch = (NULL == MsgDispatchRoutine) ? DefaultMsgDisptach : MsgDispatchRoutine;
        NkFltConnCtx.ServerPortsCtx = InterAllocPortsContext(ServerConn, TRUE);
        if (0 != ServerConn && NULL == NkFltConnCtx.ServerPortsCtx) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }
        NkFltConnCtx.ClientPortsCtx = InterAllocPortsContext(ClientConn, FALSE);
        if (0 != ClientConn && NULL == NkFltConnCtx.ClientPortsCtx) {
            try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
        }

        //  Build security descriptor
        Status = FltBuildDefaultSecurityDescriptor(&SecurityDescriptor, FLT_PORT_ALL_ACCESS);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        //  Grant unconditional access to everyone
#pragma prefast(suppress:6248, "Setting a SECURITY_DESCRIPTOR's DACL to NULL will result in an unprotected object")
        Status = RtlSetDaclSecurityDescriptor(SecurityDescriptor, TRUE, NULL, FALSE);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        InitializeObjectAttributes( &ObjectAttributes,
                                    (PUNICODE_STRING)Name,
                                    OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    SecurityDescriptor);
        Status = FltCreateCommunicationPort(Filter,
                                            &NkFltConnCtx.ControlPort,
                                            &ObjectAttributes,
                                            NULL,
                                            NkFltClientConnect,
                                            NkFltClientDisconnect,
                                            NkFltClientMessageDispatch,
                                            1 + ServerConn + ClientConn);
        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }


try_exit: NOTHING;
    }
    finally {

        if (NULL != SecurityDescriptor) {
            FltFreeSecurityDescriptor(SecurityDescriptor);
            SecurityDescriptor = NULL;
        }

        if (!NT_SUCCESS(Status)) {
            if (NULL != NkFltConnCtx.ControlPort) {
                FltCloseCommunicationPort(NkFltConnCtx.ControlPort);
            }
            if (NULL != NkFltConnCtx.ServerPortsCtx) {
                InterFreePortsContext(NkFltConnCtx.ServerPortsCtx);
            }
            if (NULL != NkFltConnCtx.ClientPortsCtx) {
                InterFreePortsContext(NkFltConnCtx.ClientPortsCtx);
            }
            RtlZeroMemory(&NkFltConnCtx, sizeof(NkFltConnCtx));
        }
    }


    return Status;
}

_IRQL_requires_(PASSIVE_LEVEL)
VOID
NkFltConnShutdown(
                  )
{
    PAGED_CODE();

    if (NULL != NkFltConnCtx.ControlPort) {
        FltCloseCommunicationPort(NkFltConnCtx.ControlPort);
    }
    if (NULL != NkFltConnCtx.ServerPortsCtx) {
        InterFreePortsContext(NkFltConnCtx.ServerPortsCtx);
    }
    if (NULL != NkFltConnCtx.ClientPortsCtx) {
        InterFreePortsContext(NkFltConnCtx.ClientPortsCtx);
    }
    RtlZeroMemory(&NkFltConnCtx, sizeof(NkFltConnCtx));
}


_Check_return_
NTSTATUS
NkFltConnQuery(
               _In_ PVOID SenderBuffer,
               _In_ ULONG SenderBufferLength,
               _Out_opt_ PVOID ReplyBuffer,
               _Inout_ PULONG ReplyLength,
               _In_opt_ PLARGE_INTEGER Timeout
               )
{

    NTSTATUS            Status = STATUS_UNSUCCESSFUL;
    KIRQL               OldIrql;
    PNKFLT_PORT_OBJECT  PortObject = NULL;
    ULONG               PortId = 0xFFFFFFFF;
    PVOID               FltReply = NULL;
    ULONG               FltReplyLength = 0;
    const ULONG         ReplyBufferSize = 0;


    if (NULL == NkFltConnCtx.Filter || NULL == NkFltConnCtx.ClientPortsCtx) {
        return STATUS_INVALID_DEVICE_STATE;
    }
    if (NULL == SenderBuffer || NULL == ReplyLength) {
        return STATUS_INVALID_PARAMETER;
    }

    *((PULONG)(&ReplyBufferSize)) = *ReplyLength;
    *ReplyLength = 0;
    if (NULL != ReplyBuffer && 0 != ReplyBufferSize) {
        RtlZeroMemory(ReplyBuffer, ReplyBufferSize);
    }

    // try to get a valid connection from list
    KeAcquireSpinLock(&NkFltConnCtx.ClientPortsCtx->Lock, &OldIrql);
    PortId = RtlFindSetBitsAndClear(&NkFltConnCtx.ClientPortsCtx->IdleMap, 1, 0);
    KeReleaseSpinLock(&NkFltConnCtx.ClientPortsCtx->Lock, OldIrql);

    if (0xFFFFFFFF == PortId) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ASSERT(PortId < NkFltConnCtx.ClientPortsCtx->Count);
    PortObject = &NkFltConnCtx.ClientPortsCtx->Ports[PortId];

    try {


        if (NULL != ReplyBuffer) {
            FltReplyLength = ROUND_TO_SIZE(sizeof(FILTER_REPLY_HEADER) + ReplyBufferSize, 8);
            FltReply = ExAllocatePoolWithTag(NonPagedPool, FltReplyLength, TAG_TEMP);
            if (NULL == FltReply) {
                try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
            }
        }

        // Check again to make sure the port is still available
        if (NULL == PortObject->Port) {
            try_return(Status = STATUS_PORT_DISCONNECTED);
        }

        // Now we have a valid connection, send a request to server
        Status = FltSendMessage(NkFltConnCtx.Filter,
                                &PortObject->Port,
                                SenderBuffer,
                                SenderBufferLength,
                                FltReply,
                                &FltReplyLength,
                                Timeout);
        if (NT_SUCCESS(Status)) {

            // Returned Length
            if (NULL != FltReply) {
                PUCHAR Data = ((PUCHAR)FltReply) + sizeof(FILTER_REPLY_HEADER);
                ULONG  Length = min((FltReplyLength - sizeof(FILTER_REPLY_HEADER)), ReplyBufferSize);
                *ReplyLength = Length;
                RtlCopyMemory(ReplyBuffer, Data, Length);
            }
        }

try_exit: NOTHING;
    }
    finally {

        if (NULL != FltReply) {
            ExFreePool(FltReply);
            FltReply = NULL;
        }

        if (NULL != PortObject) {

            KeAcquireSpinLock(&NkFltConnCtx.ClientPortsCtx->Lock, &OldIrql);
            if (STATUS_PORT_DISCONNECTED == Status) {
                RtlClearBit(&NkFltConnCtx.ClientPortsCtx->ConnectMap, PortObject->Id);
                RtlClearBit(&NkFltConnCtx.ClientPortsCtx->IdleMap, PortObject->Id);
            }
            else {
                RtlSetBit(&NkFltConnCtx.ClientPortsCtx->IdleMap, PortObject->Id);
            }
            KeReleaseSpinLock(&NkFltConnCtx.ClientPortsCtx->Lock, OldIrql);

            PortObject = NULL;
        }
    }

    return Status;
}



_Check_return_
PNKFLT_PORTS_CONTEXT
InterAllocPortsContext(
                       _In_ ULONG Count,
                       _In_ BOOLEAN Server
                       )
{
    PNKFLT_PORTS_CONTEXT Context = NULL;
    ULONG   Size = 0;
    ULONG   i = 0;

    if (0 == Count) {
        return NULL;
    }

    Size = GetPortsContextSize(Count);
    ASSERT(0 != Size);
    Context = ExAllocatePoolWithTag(NonPagedPool, Size, TAG_FLTCONN);
    if (NULL == Context) {
        return NULL;
    }

    RtlZeroMemory(Context, Size);
    *((PULONG)Context->Count) = Count;
    KeInitializeSpinLock(&Context->Lock);
    Context->ConnectBits = 0;
    Context->IdleBits = 0;
    RtlInitializeBitMap(&Context->ConnectMap, &Context->ConnectBits, Count);
    RtlInitializeBitMap(&Context->IdleMap, &Context->IdleBits, Count);
    for (i = 0; i < Count; i++) {
        *((PBOOLEAN)(&Context->Ports[i].Server)) = Server;
        *((PULONG)(&Context->Ports[i].Id)) = i;
    }
    return Context;
}

VOID
InterFreePortsContext(
                      _In_ PNKFLT_PORTS_CONTEXT Context
                      )
{
    ULONG   i = 0;
    KIRQL   OldIrql;

    KeAcquireSpinLock(&Context->Lock, &OldIrql);
    // Clear idle bits
    RtlClearAllBits(&Context->ConnectMap);
    // Clear connection bits
    RtlClearAllBits(&Context->IdleMap);
    // Close all client ports
    for (i = 0; i<Context->Count; i++) {
        if (NULL != Context->Ports[i].Port) {
            FltCloseClientPort(NkFltConnCtx.Filter, &Context->Ports[i].Port);
            Context->Ports[i].Port = NULL;
        }
    }
    *((PULONG)(&Context->Count)) = 0;
    KeReleaseSpinLock(&Context->Lock, OldIrql);
}

NTSTATUS
NkFltClientConnect (
                  _In_ PFLT_PORT ClientPort,
                  _In_ PVOID ServerPortCookie,
                  _In_ PVOID ConnectionContext,
                  _In_ ULONG SizeOfContext,
                  _Out_ PVOID *ConnectionPortCookie
                  )
{
    NTSTATUS    Status = STATUS_UNSUCCESSFUL;
    BOOLEAN     ConnToCtrl = (NULL == ConnectionContext) ? TRUE : (*((PBOOLEAN)ConnectionContext));
    ULONG       PortId = 0xFFFFFFFF;
    KIRQL       OldIrql;


    UNREFERENCED_PARAMETER(ServerPortCookie);
    UNREFERENCED_PARAMETER(SizeOfContext);


    if (NULL == NkFltConnCtx.Filter || NULL == NkFltConnCtx.ControlPort) {
        return STATUS_INVALID_DEVICE_STATE;
    }


    try {
        
        if (ConnToCtrl) {
        
            if (NULL == NkFltConnCtx.ServerPortsCtx) {
                try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
            }
            KeAcquireSpinLock(&NkFltConnCtx.ServerPortsCtx->Lock, &OldIrql);
            PortId = RtlFindClearBitsAndSet(&NkFltConnCtx.ServerPortsCtx->ConnectMap, 1, 0);
            if (0xFFFFFFFF != PortId) {

                ASSERT(PortId < NkFltConnCtx.ServerPortsCtx->Count);

                // Connect
                NkFltConnCtx.ServerPortsCtx->Ports[PortId].Port = ClientPort;
                *ConnectionPortCookie = &NkFltConnCtx.ServerPortsCtx->Ports[PortId];
                Status = STATUS_SUCCESS;
            }
            else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
            KeReleaseSpinLock(&NkFltConnCtx.ServerPortsCtx->Lock, OldIrql);
        }
        else {

            if (NULL == NkFltConnCtx.ClientPortsCtx) {
                try_return(Status = STATUS_INSUFFICIENT_RESOURCES);
            }
            KeAcquireSpinLock(&NkFltConnCtx.ClientPortsCtx->Lock, &OldIrql);
            PortId = RtlFindClearBitsAndSet(&NkFltConnCtx.ClientPortsCtx->ConnectMap, 1, 0);
            if (0xFFFFFFFF != PortId) {

                ASSERT(PortId < NkFltConnCtx.ClientPortsCtx->Count);

                // Connect
                NkFltConnCtx.ClientPortsCtx->Ports[PortId].Port = ClientPort;
                *ConnectionPortCookie = &NkFltConnCtx.ClientPortsCtx->Ports[PortId];
                // Connected, it is in idle status
                RtlSetBit(&NkFltConnCtx.ClientPortsCtx->IdleMap, PortId);
                Status = STATUS_SUCCESS;
            }
            else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
            KeReleaseSpinLock(&NkFltConnCtx.ClientPortsCtx->Lock, OldIrql);
        }

try_exit: NOTHING;
    }
    finally {

        NOTHING; // Do nothing
    }

    return Status;
}

VOID
NkFltClientDisconnect (
                     _In_ PVOID ConnectionCookie
                     )
{
    PNKFLT_PORT_OBJECT  PortObject = NULL;
    PFLT_PORT           FltPort = NULL;
    KIRQL               OldIrql;


    if (NULL == NkFltConnCtx.Filter) {
        return;
    }

    ASSERT(NULL != ConnectionCookie);
    if (NULL == ConnectionCookie) {
        return;
    }

    PortObject = (PNKFLT_PORT_OBJECT)ConnectionCookie;
    if (PortObject->Server) {
        KeAcquireSpinLock(&NkFltConnCtx.ServerPortsCtx->Lock, &OldIrql);
        RtlClearBit(&NkFltConnCtx.ServerPortsCtx->IdleMap, PortObject->Id);
        RtlClearBit(&NkFltConnCtx.ServerPortsCtx->ConnectMap, PortObject->Id);
        FltPort = PortObject->Port;
        PortObject->Port = NULL;
        KeReleaseSpinLock(&NkFltConnCtx.ServerPortsCtx->Lock, OldIrql);
    }
    else {
        KeAcquireSpinLock(&NkFltConnCtx.ClientPortsCtx->Lock, &OldIrql);
        RtlClearBit(&NkFltConnCtx.ClientPortsCtx->IdleMap, PortObject->Id);
        RtlClearBit(&NkFltConnCtx.ClientPortsCtx->ConnectMap, PortObject->Id);
        FltPort = PortObject->Port;
        PortObject->Port = NULL;
        KeReleaseSpinLock(&NkFltConnCtx.ClientPortsCtx->Lock, OldIrql);
    }

    if (NULL != FltPort) {
        FltCloseClientPort(NkFltConnCtx.Filter, &FltPort);
    }
}


_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
NkFltClientMessageDispatch (
                          _In_ PVOID PortCookie,
                          _In_opt_ PVOID InputBuffer,
                          _In_ ULONG InputBufferLength,
                          _Out_opt_ PVOID OutputBuffer,
                          _In_ ULONG OutputBufferLength,
                          _Out_ PULONG ReturnOutputBufferLength
                          )
{
    PAGED_CODE();
    if (NULL == NkFltConnCtx.Filter) {
        return STATUS_INVALID_DEVICE_STATE;
    }
    if (NULL == NkFltConnCtx.MsgDispatch) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }
    return NkFltConnCtx.MsgDispatch(InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength, ReturnOutputBufferLength);
}