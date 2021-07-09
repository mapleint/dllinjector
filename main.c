#include <Windows.h>
#include <TlHelp32.h>

/*get process id from name, if two processes share names it returns the one with the smallest pID */
DWORD32 getprocid(const char* name)
{
	PROCESSENTRY32 info = { 0 };
	info.dwSize = sizeof(info);
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	do {
		if (!strcmp(info.szExeFile, name))
			return CloseHandle(snapshot), info.th32ProcessID;
	} while (Process32Next(snapshot, &info));
	CloseHandle(snapshot);
	return 0;
}

int main(int argc, char** argv) 
{
	if (argc < 2)
		return 1;
	/*open process from name*/
	DWORD32 processid = getprocid("hl2.exe");
	if (!processid)
		return 1;

	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processid);

	/*allocate memory to write name of process to load*/
	void* namebuffer = VirtualAllocEx(process, NULL, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	
	if (!namebuffer) {
		CloseHandle(process);
		return 1;
	}

	/*write name of process we will load*/
	if (!WriteProcessMemory(process, namebuffer, argv[1], strlen(argv[1]) + 1, NULL))
		return CloseHandle(process), 1;

	/*create a thread that will run load library with the parameter being our name*/
	CreateRemoteThread(process, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryA, namebuffer, NULL, NULL);

	return 0;
}