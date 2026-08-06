// TouchFish.FileMove.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "TouchFish.FileMove.h"
#include "FileOperations.h"
#include "ProgressDialog.h"
#include <shobjidl.h>
#include <shlobj.h>
#include <thread>
#include <memory>
#include <string>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define MAX_LOADSTRING 100

// 控件ID
#define IDC_SOURCE_EDIT         2001
#define IDC_SOURCE_BUTTON       2002
#define IDC_DEST_EDIT           2003
#define IDC_DEST_BUTTON         2004
#define IDC_MOVE_BUTTON         2005
#define IDC_SYMLINK_CHECK       2006
#define IDC_STATUS_STATIC       2007

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// UI控件句柄
HWND g_hSourceEdit = nullptr;
HWND g_hDestEdit = nullptr;
HWND g_hSourceButton = nullptr;
HWND g_hDestButton = nullptr;
HWND g_hMoveButton = nullptr;
HWND g_hSymlinkCheck = nullptr;
HWND g_hStatusText = nullptr;

// 文件操作对象
std::unique_ptr<FileOperations> g_fileOps = nullptr;
std::unique_ptr<ProgressDialog> g_progressDlg = nullptr;

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

// 辅助函数
std::wstring BrowseForFolder(HWND hwnd, const wchar_t* title);
void PerformMove(HWND hwnd);
void CreateUIControls(HWND hwnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TOUCHFISHFILEMOVE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TOUCHFISHFILEMOVE));

    MSG msg;

    // 主消息循环:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TOUCHFISHFILEMOVE));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TOUCHFISHFILEMOVE);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // 将实例句柄存储在全局变量中

   // 创建主窗口，固定大小
   HWND hWnd = CreateWindowW(szWindowClass, L"TouchFish.FileMove 文件移动器", 
      WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
      CW_USEDEFAULT, 0, 640, 380, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // 居中显示
   RECT rc;
   GetWindowRect(hWnd, &rc);
   int width = rc.right - rc.left;
   int height = rc.bottom - rc.top;
   int screenWidth = GetSystemMetrics(SM_CXSCREEN);
   int screenHeight = GetSystemMetrics(SM_CYSCREEN);
   SetWindowPos(hWnd, NULL,
       (screenWidth - width) / 2,
       (screenHeight - height) / 2,
       0, 0, SWP_NOSIZE | SWP_NOZORDER);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        CreateUIControls(hWnd);
        g_fileOps = std::make_unique<FileOperations>();
        break;

    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDC_SOURCE_BUTTON:
                {
                    std::wstring path = BrowseForFolder(hWnd, L"选择源文件夹");
                    if (!path.empty()) {
                        SetWindowTextW(g_hSourceEdit, path.c_str());
                    }
                }
                break;

            case IDC_DEST_BUTTON:
                {
                    std::wstring path = BrowseForFolder(hWnd, L"选择目标文件夹");
                    if (!path.empty()) {
                        SetWindowTextW(g_hDestEdit, path.c_str());
                    }
                }
                break;

            case IDC_MOVE_BUTTON:
                PerformMove(hWnd);
                break;

            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;

            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;

            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;

    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_DESTROY:
        g_fileOps.reset();
        g_progressDlg.reset();
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
// 创建UI控件
void CreateUIControls(HWND hwnd)
{
    // 初始化通用控件
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&icex);

    HFONT hFont = CreateFontW(
        20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"SimSun"
    );
    
    // 源目录标签
    HWND hLabel1 = CreateWindowW(L"STATIC", L"源目录:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 25, 80, 25,
        hwnd, NULL, hInst, NULL);
    SendMessage(hLabel1, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 源目录编辑框
    g_hSourceEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        20, 55, 460, 30,
        hwnd, (HMENU)IDC_SOURCE_EDIT, hInst, NULL);
    SendMessage(g_hSourceEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 源目录浏览按钮
    g_hSourceButton = CreateWindowW(L"BUTTON", L"浏览...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        490, 55, 90, 30,
        hwnd, (HMENU)IDC_SOURCE_BUTTON, hInst, NULL);
    SendMessage(g_hSourceButton, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 目标目录标签
    HWND hLabel2 = CreateWindowW(L"STATIC", L"目标目录:",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 105, 100, 25,
        hwnd, NULL, hInst, NULL);
    SendMessage(hLabel2, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 目标目录编辑框
    g_hDestEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        20, 135, 460, 30,
        hwnd, (HMENU)IDC_DEST_EDIT, hInst, NULL);
    SendMessage(g_hDestEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 目标目录浏览按钮
    g_hDestButton = CreateWindowW(L"BUTTON", L"浏览...",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        490, 135, 90, 30,
        hwnd, (HMENU)IDC_DEST_BUTTON, hInst, NULL);
    SendMessage(g_hDestButton, WM_SETFONT, (WPARAM)hFont, TRUE);

    // 创建符号链接复选框
    g_hSymlinkCheck = CreateWindowW(L"BUTTON", L"移动后在原位置创建符号链接",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 185, 300, 25,
        hwnd, (HMENU)IDC_SYMLINK_CHECK, hInst, NULL);
    SendMessage(g_hSymlinkCheck, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(g_hSymlinkCheck, BM_SETCHECK, BST_CHECKED, 0);

    // 开始移动按钮
    g_hMoveButton = CreateWindowW(L"BUTTON", L"开始移动",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        220, 230, 160, 45,
        hwnd, (HMENU)IDC_MOVE_BUTTON, hInst, NULL);
    
    HFONT hButtonFont = CreateFontW(
        22, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"SimSun"
    );
    SendMessage(g_hMoveButton, WM_SETFONT, (WPARAM)hButtonFont, TRUE);

    // 状态文本
    g_hStatusText = CreateWindowW(L"STATIC", L"准备就绪",
        WS_CHILD | WS_VISIBLE | SS_CENTER,
        20, 295, 560, 25,
        hwnd, (HMENU)IDC_STATUS_STATIC, hInst, NULL);
    SendMessage(g_hStatusText, WM_SETFONT, (WPARAM)hFont, TRUE);
}

// 浏览文件夹对话框
std::wstring BrowseForFolder(HWND hwnd, const wchar_t* title)
{
    std::wstring result;
    
    CoInitialize(NULL);
    
    IFileOpenDialog* pFileOpen = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
        IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
    
    if (SUCCEEDED(hr))
    {
        pFileOpen->SetTitle(title);
        
        DWORD dwOptions;
        pFileOpen->GetOptions(&dwOptions);
        pFileOpen->SetOptions(dwOptions | FOS_PICKFOLDERS);
        
        hr = pFileOpen->Show(hwnd);
        
        if (SUCCEEDED(hr))
        {
            IShellItem* pItem;
            hr = pFileOpen->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                
                if (SUCCEEDED(hr))
                {
                    result = pszFilePath;
                    CoTaskMemFree(pszFilePath);
                }
                pItem->Release();
            }
        }
        pFileOpen->Release();
    }
    
    CoUninitialize();
    return result;
}

// 执行文件移动操作
void PerformMove(HWND hwnd)
{
    // 获取输入
    wchar_t sourcePath[MAX_PATH];
    wchar_t destPath[MAX_PATH];
    
    GetWindowTextW(g_hSourceEdit, sourcePath, MAX_PATH);
    GetWindowTextW(g_hDestEdit, destPath, MAX_PATH);
    
    std::wstring source = sourcePath;
    std::wstring dest = destPath;
    
    // 验证输入
    if (source.empty() || dest.empty())
    {
        MessageBoxW(hwnd, L"请选择源文件夹和目标文件夹", L"输入错误", MB_OK | MB_ICONWARNING);
        return;
    }
    
    if (!FileOperations::IsValidPath(source))
    {
        MessageBoxW(hwnd, L"源文件夹路径无效或不存在", L"路径错误", MB_OK | MB_ICONERROR);
        return;
    }
    
    // 检查是否创建符号链接
    bool createSymlink = (SendMessage(g_hSymlinkCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
    
    // 确认操作
    std::wstring confirmMsg = L"确定要将文件夹从\n" + source + L"\n移动到\n" + dest + L"\n吗？";
    if (createSymlink)
    {
        confirmMsg += L"\n\n将在原位置创建符号链接。";
    }
    
    if (MessageBoxW(hwnd, confirmMsg.c_str(), L"确认移动", MB_YESNO | MB_ICONQUESTION) != IDYES)
    {
        return;
    }
    
    // 禁用按钮
    EnableWindow(g_hMoveButton, FALSE);
    EnableWindow(g_hSourceButton, FALSE);
    EnableWindow(g_hDestButton, FALSE);
    SetWindowTextW(g_hStatusText, L"正在移动文件...");
    
    // 创建进度对话框
    g_progressDlg = std::make_unique<ProgressDialog>();
    if (!g_progressDlg->Create(hwnd)) {
        MessageBoxW(hwnd, L"无法创建进度对话框", L"错误", MB_OK | MB_ICONERROR);
        EnableWindow(g_hMoveButton, TRUE);
        EnableWindow(g_hSourceButton, TRUE);
        EnableWindow(g_hDestButton, TRUE);
        return;
    }
    
    // 在新线程中执行移动操作
    std::thread([hwnd, source, dest, createSymlink]() {
        // 执行移动
        auto result = g_fileOps->MoveDirectory(
            source,
            dest,
            createSymlink,
            [](ULONGLONG current, ULONGLONG total, const std::wstring& msg) {
                if (g_progressDlg) {
                    g_progressDlg->UpdateProgress(current, total, msg);
                }
            }
        );
        
        // 先关闭进度对话框
        if (g_progressDlg) {
            g_progressDlg->Close();
            g_progressDlg.reset();
        }
        
        // 显示结果
        if (result.success)
        {
            MessageBoxW(hwnd, L"文件移动成功！", L"完成", MB_OK | MB_ICONINFORMATION);
            SetWindowTextW(g_hStatusText, L"移动完成");
        }
        else
        {
            // 检查是否回滚失败
            if (result.rollbackFailed)
            {
                std::wstring errorMsg = L"操作失败且回滚失败！\n\n" + result.errorMessage +
                    L"\n\n请立即手动检查源目录和目标目录的文件状态，并手动恢复！";
                MessageBoxW(hwnd, errorMsg.c_str(), L"严重错误", MB_OK | MB_ICONERROR);
                SetWindowTextW(g_hStatusText, L"移动失败且回滚失败");
            }
            else
            {
                std::wstring errorMsg = L"移动失败:\n" + result.errorMessage;
                MessageBoxW(hwnd, errorMsg.c_str(), L"错误", MB_OK | MB_ICONERROR);
                SetWindowTextW(g_hStatusText, L"移动失败");
            }
        }
        
        // 重新启用按钮
        EnableWindow(g_hMoveButton, TRUE);
        EnableWindow(g_hSourceButton, TRUE);
        EnableWindow(g_hDestButton, TRUE);
    }).detach();
}