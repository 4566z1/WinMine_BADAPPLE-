#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <process.h>
#include <time.h>


#define MAX_WINDOW 4

using namespace std;

#define HEIGHT 45
#define WIDTH 56

RECT rect;
CHAR DATA_BUFFER[HEIGHT][WIDTH] = { 0 };

DWORD BASE_ADDRESS = 0x01005361;	// 游戏矩阵基地址

HANDLE PROCESS_HANDLE[MAX_WINDOW] = { 0 };
DWORD WINDOW_PROCESSID[MAX_WINDOW] = { 0 };
DWORD CURRENT_HWND_NUMS = NULL;
HWND WINDOW_HWND[8] = { 0 };

SIZE_T RET_BUFFER = NULL;



BOOL GetProcessIDByNAME(CONST TCHAR *PROCESS_NAME) {
	PROCESSENTRY32W ProcessInformation = { 0 };
	ProcessInformation.dwSize = sizeof(ProcessInformation);
	HANDLE bRet = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (bRet == INVALID_HANDLE_VALUE)
		return FALSE;
	if (!Process32FirstW(bRet, &ProcessInformation))
		return FALSE;

	INT I = NULL;
	do
	{
		if (I == 4)
			break;
		if (!lstrcmpiW(ProcessInformation.szExeFile, PROCESS_NAME)) {
			WINDOW_PROCESSID[I] = ProcessInformation.th32ProcessID;
			I++;
		}

	} while (Process32NextW(bRet, &ProcessInformation));
	CloseHandle(bRet);
	return TRUE;
}

BOOL CALLBACK EnumWindowProc(HWND hwnd, LPARAM lParma) {
	DWORD Buffer = NULL;
	GetWindowThreadProcessId(hwnd, &Buffer);
	if (CURRENT_HWND_NUMS == 4)
		return FALSE;
	for (int i = 0; i < MAX_WINDOW; i++)
	{
		if (WINDOW_PROCESSID[i] == Buffer) {
			WINDOW_HWND[CURRENT_HWND_NUMS] = hwnd;
			CURRENT_HWND_NUMS++;
			break;
		}
	}
	return TRUE;
}

DWORD GET_WINDOW_HWND() {
	CURRENT_HWND_NUMS = NULL;
	EnumWindows(EnumWindowProc, NULL);
	return TRUE;
}

// 更新窗口
UINT WINAPI UPDATE_WINDOW(LPVOID) {
	for (int i = 0; i < MAX_WINDOW; i++) {
		InvalidateRect(WINDOW_HWND[i], NULL, FALSE);
	}
	return 0;
}


// 处理图片数据
VOID PROCESS_FLAG() {
	INT J = 0;
	for (int i = HEIGHT; i > 0; i--)
	{
		if (i >= 23) {
			WriteProcessMemory(PROCESS_HANDLE[0], (LPVOID)(BASE_ADDRESS + (DWORD)(J * 32)), (LPVOID)(DATA_BUFFER[J]), sizeof(CHAR) * 28, &RET_BUFFER);
			WriteProcessMemory(PROCESS_HANDLE[1], (LPVOID)(BASE_ADDRESS + (DWORD)(J * 32)), (LPVOID)(DATA_BUFFER[J] + 28), sizeof(CHAR) * 28, &RET_BUFFER);
		}
		if (i < 23) {
			WriteProcessMemory(PROCESS_HANDLE[2], (LPVOID)(BASE_ADDRESS + (DWORD)((J - 23) * 32)), (LPVOID)(DATA_BUFFER[J]), sizeof(CHAR) * 28, &RET_BUFFER);
			WriteProcessMemory(PROCESS_HANDLE[3], (LPVOID)(BASE_ADDRESS + (DWORD)((J - 23) * 32)), (LPVOID)(DATA_BUFFER[J] + 28), sizeof(CHAR) * 28, &RET_BUFFER);
		}
		J++;
	}
}

// 读取数据
BOOL READ_DATA(FILE *DATA) {
	// i: HEIGHT j: WIDTH 

	INT C = NULL;

	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
		{
			C = fgetc(DATA);
			if (C == EOF)
				return FALSE;
			if (C == '0')
				DATA_BUFFER[i][j] = rand() % 7 + 65;	// 取消旗帜
			if (C == '1')
				DATA_BUFFER[i][j] = 0x40;	// 随机数字
		}
	}
	return TRUE;
}

int main() {
	cout << "注意： 你需要把扫雷的窗口大小调成 高23 宽28 ，而且需要把窗口摆好" << endl;
	if (!GetProcessIDByNAME(L"winmine.exe")) {
		cout << "[*] 获取进程ID失败 " << endl;
		system("pause");
		ExitProcess(0);
	}
	if (WINDOW_PROCESSID[3] == 0) {
		cout << "[*] 扫雷窗口不足四个 " << endl;
		system("pause");
		ExitProcess(0);
	}

	// 获取四个窗口句柄
	GET_WINDOW_HWND();

	// 打开四个进程
	for (int i = 0; i < MAX_WINDOW; i++)
	{
		PROCESS_HANDLE[i] = OpenProcess(PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, WINDOW_PROCESSID[i]);
	}

	FILE* DATA = fopen("DATA.txt", "r");
	if (DATA == NULL)
		return 0;

	srand(time(NULL));
	std::cout << "正在工作中.." << endl;
	while (READ_DATA(DATA)) {
		PROCESS_FLAG();
		UPDATE_WINDOW(NULL);
		Sleep(51);
	}
	fclose(DATA);
	return 0;
}