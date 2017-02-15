#include "FaceVerificationDll.h"
#include "resource.h"

WCHAR	wszWndClassName[] = TEXT("loadCameraClass");
WCHAR	wszWndName[] = TEXT("OpenCV-Camera:");

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK MyDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

enum OperationModeType
{
	ModeType_Train = 1,
	ModeType_Verifier
};

HINSTANCE	g_hInstance;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR cmdLine, int nShow) {
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(cmdLine);

	g_hInstance = hInstance;

	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;
	wndClass.hInstance = hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = wszWndClassName;
	if (!RegisterClassEx(&wndClass))
		return -1;

	RECT rc = { 0, 0, 640, 480 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);

	//create Menu
	HMENU	hMenu = CreateMenu();

	HMENU	hPopMenu = CreatePopupMenu();
	AppendMenu(hPopMenu, MF_STRING, ID_FACEVERIFICER_TRAINFACE, TEXT("Train Face"));
	AppendMenu(hPopMenu, MF_STRING, ID_FACEVERIFICER_VERIFICATION, TEXT("Verifaction"));

	AppendMenu(hMenu, MF_POPUP, (UINT)hPopMenu, TEXT("Face Verifier"));


	HWND hwnd = CreateWindow(wszWndClassName, wszWndName,
		WS_OVERLAPPEDWINDOW, 400, 400, rc.right - rc.left, rc.bottom - rc.top,
		NULL, hMenu, hInstance, NULL);

	if (!hwnd)
		return -1;

	ShowWindow(hwnd, nShow);

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return static_cast<int>(msg.wParam);
}



LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
	}
	break;
	case WM_COMMAND:
	{
		switch (LOWORD(wParam))
		{
		case ID_FACEVERIFICER_TRAINFACE:
			//train face 
			DialogBoxParamW(g_hInstance, MAKEINTRESOURCE(IDD_DLGFACE), hwnd, MyDlgProc, (LPARAM)ModeType_Train);
			break;
		case ID_FACEVERIFICER_VERIFICATION:
			//verifier
			DialogBoxParamW(g_hInstance, MAKEINTRESOURCE(IDD_DLGFACE), hwnd, MyDlgProc, (LPARAM)ModeType_Verifier);
			break;
		default:
			break;
		}
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

#include <Windows.h>


IFaceVerification *pFaceVer;

INT_PTR CALLBACK MyDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static bool isBegin = false;
	static HWND	hWndImage;
	static HWND	hWndStatus;
	static OperationModeType dwModeType = ModeType_Train;
	RECT	rect;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		dwModeType = (OperationModeType)lParam;
		if (ModeType_Train == dwModeType)
		{
			SetDlgItemTextW(hwndDlg, IDOK, TEXT("录取数据"));
		}
		else if (ModeType_Verifier == dwModeType)
		{
			SetDlgItemTextW(hwndDlg, IDOK, TEXT("验证"));
		}
		hWndImage = GetDlgItem(hwndDlg, IDC_Image);
		hWndStatus = GetDlgItem(hwndDlg, IDC_STATIC_STATUS);
		GetWindowRect(hwndDlg, &rect);
		SetWindowPos(hwndDlg, HWND_TOP, 200, 200, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);

		GetClientRect(hWndImage, &rect);
		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		pFaceVer = AllocFaceVerificationInstance(g_hInstance);

		pFaceVer->initialize();
		//pFaceVer->setCameraId(1);
		pFaceVer->setHwndForShowCameraVideo(GetDlgItem(hwndDlg, IDC_Image));
		pFaceVer->setHwndForShowStatusTips(hWndStatus);
		pFaceVer->setCameraWindowSize(width, height);

		if (ModeType_Train == dwModeType)
		{
			//pFaceVer->trainFaceImage("test");
			pFaceVer->trainFaceImage("test3", true);
		}
		else if (ModeType_Verifier == dwModeType)
		{
			pFaceVer->faceVerifier();
			if (pFaceVer->getFaceVerifierReuslt())
			{
				EndDialog(hwndDlg, wParam);
			}
		}

	}
		return TRUE;
	case WM_COMMAND:
	{
		/*
		switch (LOWORD(wParam))
		{
		case IDOK:
			if (!isBegin)
			{
				isBegin = true;
				GetClientRect(hWndImage, &rect);
				int width = rect.right - rect.left;
				int height = rect.bottom - rect.top;

				faceVer.initialize();
				faceVer.setHwndForShowCameraVideo(GetDlgItem(hwndDlg, IDC_Image));
				faceVer.setHwndForShowStatusTips(hWndStatus);
				faceVer.setCameraWindowSize(width, height);

				if (ModeType_Train == dwModeType)
				{
					faceVer.trainFaceImage("test1");
				}
				else if (ModeType_Verifier == dwModeType)
				{
					faceVer.faceVerifier();
				}
			}
			break;
		case IDCANCEL:
		{
			faceVer.quit();
			isBegin = false;
			GetClientRect(hWndImage, &rect);
		}
		break;
		}
		*/
	}
	return TRUE;
	case WM_DESTROY:
	case WM_QUIT:
	case WM_CLOSE:
		pFaceVer->quit();
		EndDialog(hwndDlg, wParam);
		return TRUE;
	}
	return FALSE;
}