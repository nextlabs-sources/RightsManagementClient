

#ifndef __NXRM_VHD_DEVICE_H__
#define __NXRM_VHD_DEVICE_H__



_IRQL_requires_(PASSIVE_LEVEL)
NTSTATUS
VhdDispatchDeviceControl(
	                     _In_ PDEVICE_OBJECT DeviceObject,
	                     _In_ PIRP Irp
	                     );



#endif