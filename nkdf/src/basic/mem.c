
#include <ntifs.h>
#include <Ntstrsafe.h>


#include <nkdf/basic/defines.h>
#include <nkdf/basic/mem.h>




//  Assign text sections for each routine.
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkLockUserBuffer)
#pragma alloc_text(PAGE, NkMapUserBuffer)
#pragma alloc_text(PAGE, NkBufferUserBuffer)
#endif



_IRQL_requires_max_(APC_LEVEL)
VOID
NkLockUserBuffer(
                 _Inout_ PIRP Irp,
                 _In_ LOCK_OPERATION Operation,
                 _In_ ULONG BufferLength
                 )
{
    PMDL Mdl = NULL;

    PAGED_CODE();

    if (Irp->MdlAddress == NULL) {

        //
        // Allocate the Mdl, and Raise if we fail.
        //
        Mdl = IoAllocateMdl(Irp->UserBuffer, BufferLength, FALSE, FALSE, Irp);
        if (Mdl == NULL) {
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        }

        //
        // Now probe the buffer described by the Irp.  If we get an exception,
        // deallocate the Mdl and return the appropriate "expected" status.
        //
        try {
            MmProbeAndLockPages(Mdl, Irp->RequestorMode, Operation);
#pragma prefast(suppress:6320, "Here we need to handle all exceptions")
        } except(EXCEPTION_EXECUTE_HANDLER) {
            NTSTATUS Status;
            Status = GetExceptionCode();
            IoFreeMdl(Mdl);
            Irp->MdlAddress = NULL;
            ExRaiseStatus(FsRtlIsNtstatusExpected(Status) ? Status : STATUS_INVALID_USER_BUFFER);
        }
    }
}

_IRQL_requires_max_(APC_LEVEL)
PVOID
NkMapUserBuffer(
                _Inout_ PIRP Irp
                )
{
    PVOID Buffer = NULL;


    PAGED_CODE();

    //
    // If there is no Mdl, then we must be in the Driver Context, and we can simply
    // return the UserBuffer field from the Irp.
    //
    if (Irp->MdlAddress == NULL) {
        Buffer = Irp->UserBuffer;
    }
    else {
        Buffer = MmGetSystemAddressForMdlSafe(Irp->MdlAddress, NormalPagePriority);
        if (Buffer == NULL) {
            ExRaiseStatus(STATUS_INSUFFICIENT_RESOURCES);
        }
    }

    return Buffer;
}

_IRQL_requires_max_(APC_LEVEL)
PVOID
NkBufferUserBuffer (
                    _Inout_ PIRP Irp,
                    _In_ ULONG BufferLength
                    )
{
    PUCHAR UserBuffer = NULL;


    PAGED_CODE();

    //
    //  Handle the no buffer case.
    //    
    if (BufferLength == 0) {
        return NULL;
    }

    //
    //  If there is no system buffer we must have been supplied an Mdl
    //  describing the users input buffer, which we will now snapshot.
    //
    if (Irp->AssociatedIrp.SystemBuffer == NULL) {

        UserBuffer = NkMapUserBuffer(Irp);
        Irp->AssociatedIrp.SystemBuffer = FsRtlAllocatePoolWithQuotaTag(NonPagedPool, BufferLength, TAG_IOBUF);

        //
        // Set the flags so that the completion code knows to deallocate the
        // buffer.
        //
        Irp->Flags |= (IRP_BUFFERED_IO | IRP_DEALLOCATE_BUFFER);

        try {
            RtlCopyMemory(Irp->AssociatedIrp.SystemBuffer, UserBuffer, BufferLength);
#pragma prefast(suppress:6320, "Here we need to handle all exceptions")
        } except(EXCEPTION_EXECUTE_HANDLER) {
            NTSTATUS Status;
            Status = GetExceptionCode();
            ExRaiseStatus(FsRtlIsNtstatusExpected(Status) ? Status : STATUS_INVALID_USER_BUFFER);
        }
    }

    return Irp->AssociatedIrp.SystemBuffer;
}