#include<windows.h>
int main()
{
	HWND hwnd;
	hwnd=FindWindow("ConsoleWindowClass",NULL);	
	if(hwnd)
	{
		ShowWindow(hwnd,SW_HIDE);//设置指定窗口的显示状态
	}
	MessageBoxW(NULL,L"控制台已隐藏",L"提示",MB_OK);
	// system("pause");
	return 0;
}