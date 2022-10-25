

#include <ntifs.h>
#include <Ntstrsafe.h>

#include <nkdf/basic/device.h>



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NkDeviceIoControl)
#endif





_Check_return_
_IRQL_requires_max_(APC_LEVEL)
NTSTATUS
NkDeviceIoControl(
                  _In_ ULONG IoControlCode,
                  _In_ PDEVICE_OBJECT Device,
                  _In_opt_ PVOID InputBuffer,
                  _In_ ULONG InputBufferLength,
                  _Out_opt_ PVOID OutputBuffer,
                  _In_ ULONG OutputBufferLength,
                  _In_ BOOLEAN InternalDeviceIoControl,
                  _In_ BOOLEAN OverrideVerify,
                  _Out_opt_ PIO_STATUS_BLOCK Iosb
                  )
{
    NTSTATUS         Status;
    PIRP             Irp;
    KEVENT           Event;
    IO_STATUS_BLOCK  LocalIosb;
    PIO_STATUS_BLOCK IosbToUse = &LocalIosb;

    PAGED_CODE();


    //  Check if the user gave us an Iosb.
    if (ARGUMENT_PRESENT(Iosb)) {
        IosbToUse = Iosb;
    }

    IosbToUse->Status = 0;
    IosbToUse->Information = 0;

    KeInitializeEvent(&Event, NotificationEvent, FALSE);
    Irp = IoBuildDeviceIoControlRequest(IoControlCode,
                                        Device,
                                        InputBuffer,
                                        InputBufferLength,
                                        OutputBuffer,
                                        OutputBufferLength,
                                        InternalDeviceIoControl,
                                        &Event,
                                        IosbToUse);
    if (Irp == NULL) {
        IosbToUse->Status = STATUS_INSUFFICIENT_RESOURCES;
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (OverrideVerify) {
        SetFlag(IoGetNextIrpStackLocation(Irp)->Flags, SL_OVERRIDE_VERIFY_VOLUME);
    }

    Status = IoCallDriver(Device, Irp);
    if (Status == STATUS_PENDING) {
        (VOID)KeWaitForSingleObject(&Event, Executive, KernelMode, FALSE, (PLARGE_INTEGER)NULL);
        Status = IosbToUse->Status;
    }

    return Status;
}
