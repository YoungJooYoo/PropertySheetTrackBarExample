///////////////////////////////////////////////////////////////////////////////
//
//              프라퍼티시트(Property Sheet) Sample
//
///////////////////////////////////////////////////////////////////////////////

#define  STRICT
#define  WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Shlwapi.h>    //wvnsprintf()
#include <CommCtrl.h>
#include "RESOURCE.H"


#define LOCAL(Type)     static Type WINAPI
#define DLGFNC(DlgFnc)  static BOOL CALLBACK DlgFnc(HWND hWnd, UINT Msg, WPARAM wPrm, LPARAM lPrm)
#define WNDFNC(WndFnc)  static LRESULT CALLBACK WndFnc(HWND hWnd, UINT Msg, WPARAM wPrm, LPARAM lPrm)
#define WMCMDID         LOWORD(wPrm)
#define WMCMDNOTIFY     HIWORD(wPrm)
#define countof(A)      (sizeof(A)/sizeof(A[0]))


static CONST CHAR MainWndClsName[]="PropSheetSample";
static HINSTANCE HInst;
static HWND hWndMain;                   //메인창핸들
static int  OpenedPropSheetPage;        //이전에 열었던 페이지 번호



//-----------------------------------------------------------------------------
//      스크린 디버그 함수
//-----------------------------------------------------------------------------
#pragma comment(lib, "Shlwapi.lib") //wvnsprintf()
VOID Printf(LPCSTR FormStr, ...)
    {
    HDC  hDC;
    va_list VL;
    CHAR Buff[MAX_PATH];
    static int  DebugPosY=0;

    va_start(VL, FormStr);
    wvnsprintf(Buff, sizeof(Buff), FormStr, VL);
    va_end(VL);

    hDC=GetDC(NULL);
    TextOut(hDC, 0, DebugPosY, Buff, lstrlen(Buff));
    ReleaseDC(NULL, hDC);
    if ((DebugPosY+=16)>=GetSystemMetrics(SM_CYSCREEN)) DebugPosY=0;
    }




//-----------------------------------------------------------------------------
//          About 대화상자 Proc
//-----------------------------------------------------------------------------
DLGFNC(AboutDlgProc)
    {
    switch (Msg)
        {
        case WM_INITDIALOG:
            return TRUE;

        case WM_COMMAND:
            EndDialog(hWnd, TRUE);
            return TRUE;
        }
    return FALSE;
    }



//-----------------------------------------------------------------------------
//          프라퍼시 시트중 범위설정 페이지
//-----------------------------------------------------------------------------
DLGFNC(RangeDlgProc)
    {
    static UINT SavedMin, SavedMax;
    static PROPSHEETPAGE *PSP;

    switch (Msg)
        {
        case WM_INITDIALOG:
            PSP=(PROPSHEETPAGE*)lPrm;
            return TRUE;

        #if 0
        case WM_COMMAND:
            switch (WMCMDID)
                {
                case TbRangeMinEBID:
                case TbRangeMaxEBID:
                    if (WMCMDNOTIFY==EN_CHANGE)
                        SendMessage(GetParent(hWnd), PSM_CHANGED, (WPARAM)hWnd, 0);
                }
            break;
        #endif

        case WM_NOTIFY:
            switch (((NMHDR*)lPrm)->code)
                {
                case PSN_SETACTIVE:     //이 Tab이 열릴 때
                    OpenedPropSheetPage=0;
                    SavedMin=SendDlgItemMessage(hWndMain, TrackBarID, TBM_GETRANGEMIN, 0, 0);
                    SavedMax=SendDlgItemMessage(hWndMain, TrackBarID, TBM_GETRANGEMAX, 0, 0);
                    SetDlgItemInt(hWnd, TbRangeMinEBID, SavedMin, TRUE);
                    SetDlgItemInt(hWnd, TbRangeMaxEBID, SavedMax, TRUE);
                    break;

                case PSN_APPLY:         //확인을 눌렀을 때
                    {
                    UINT Min, Max;

                    Min=GetDlgItemInt(hWnd, TbRangeMinEBID, NULL, TRUE);
                    Max=GetDlgItemInt(hWnd, TbRangeMaxEBID, NULL, TRUE);
                    SendDlgItemMessage(hWndMain, TrackBarID, TBM_SETRANGE, TRUE, MAKELONG(Min, Max));
                    SetWindowLong(hWnd, DWL_MSGRESULT, TRUE);
                    break;
                    }

                case PSN_KILLACTIVE:    //이 TAB이 닫힐 때
                    SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
                    return 1;

                case PSN_RESET:         //취소키를 누름 (이 페이지를 오픈한 적이 있는 경우에만)
                    SendDlgItemMessage(hWndMain, TrackBarID, TBM_SETRANGE, TRUE, MAKELONG(SavedMin, SavedMax));
                    SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);  //DWL_MSGRESULT에 TRUE를 설정하면 프라퍼티시트가 닫히지 않음
                    break;
                }
        }
    return FALSE;
    }




//-----------------------------------------------------------------------------
//          프라퍼시 시트중 페이지 크기 설정 페이지
//-----------------------------------------------------------------------------
DLGFNC(PageSizeDlgProc)
    {
    static UINT SavedPage, SavedLine;
    static PROPSHEETPAGE *PSP;

    switch (Msg)
        {
        case WM_INITDIALOG:
            PSP=(PROPSHEETPAGE*)lPrm;
            return TRUE;

        case WM_NOTIFY:
            switch (((NMHDR*)lPrm)->code)
                {
                case PSN_SETACTIVE:     //이 Tab이 열릴 때
                    OpenedPropSheetPage=1;
                    SavedPage=SendDlgItemMessage(hWndMain, TrackBarID, TBM_GETPAGESIZE, 0, 0);
                    SavedLine=SendDlgItemMessage(hWndMain, TrackBarID, TBM_GETLINESIZE, 0, 0);
                    SetDlgItemInt(hWnd, TbLineSizeEBID, SavedLine, TRUE);
                    SetDlgItemInt(hWnd, TbPageSizeEBID, SavedPage, TRUE);
                    break;

                case PSN_APPLY:
                    {
                    UINT Page, Line;

                    Page=GetDlgItemInt(hWnd, TbPageSizeEBID, 0, TRUE);
                    SendDlgItemMessage(hWndMain, TrackBarID, TBM_SETPAGESIZE, 0, Page);

                    Line=GetDlgItemInt(hWnd, TbLineSizeEBID, 0, TRUE);
                    SendDlgItemMessage(hWndMain, TrackBarID, TBM_SETLINESIZE, 0, Line);
                    SetWindowLong(hWnd, DWL_MSGRESULT, TRUE);
                    break;
                    }

                case PSN_KILLACTIVE:    //이 TAB이 닫힐 때
                    SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);  //TRUE를 입력하면 이 페이지를 떠나지 않음
                    return 1;

                case PSN_RESET:         //취소키를 누름 (이 페이지를 오픈한 적이 있는 경우에만)
                    SendDlgItemMessage(hWndMain, TrackBarID, TBM_SETPAGESIZE, 0, SavedPage);
                    SendDlgItemMessage(hWndMain, TrackBarID, TBM_SETLINESIZE, 0, SavedLine);
                    SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);  //DWL_MSGRESULT에 TRUE를 설정하면 프라퍼티시트가 닫히지 않음
                    break;
                }
        }
    return FALSE;
    }




//-----------------------------------------------------------------------------
//          TrackBar 설정창 프라퍼티페이지를 띄움
//-----------------------------------------------------------------------------
LOCAL(int) CreatePropertySheet(HWND hWnd)
    {
    PROPSHEETHEADER PSH;
    PROPSHEETPAGE   PSP[2];

    PSP[0].dwSize       = sizeof(PROPSHEETPAGE);
    PSP[0].dwFlags      = 0;
    PSP[0].hInstance    = HInst;
    PSP[0].pszTemplate  = MAKEINTRESOURCE(SetTbRangeDialog);
    PSP[0].pfnDlgProc   = RangeDlgProc;
    PSP[0].lParam       = 0;

    PSP[1].dwSize       = sizeof(PROPSHEETPAGE);
    PSP[1].dwFlags      = PSP_USETITLE;
    PSP[1].hInstance    = HInst;
    PSP[1].pszTemplate  = MAKEINTRESOURCE(SetTbPageLineDialog);
    PSP[1].pszTitle     = "-트랙바 페이지 설정-";   //지정안하면 리소스의 대화상자 타이틀을 사용함
    PSP[1].pfnDlgProc   = PageSizeDlgProc;
    PSP[1].lParam       = 0;

    PSH.dwSize          = sizeof(PROPSHEETHEADER);
    PSH.dwFlags         = PSH_PROPSHEETPAGE;
    PSH.hwndParent      = hWnd;
    PSH.hInstance       = HInst;
    PSH.pszIcon         = NULL;
    PSH.pszCaption      = "Slider Properties";
    PSH.nPages          = countof(PSP);
    PSH.nStartPage      = OpenedPropSheetPage;
    PSH.ppsp            = PSP;
    return PropertySheet(&PSH);
    }




//-----------------------------------------------------------------------------
//          Main Window Proc
//-----------------------------------------------------------------------------
WNDFNC(MainWndProc)
    {
    switch (Msg)
        {
        case WM_CREATE:
            hWndMain=hWnd;
            CreateWindow(STATUSCLASSNAME, "", WS_CHILD | WS_BORDER | WS_VISIBLE, -100,-100,10,10, hWnd, (HMENU)StatusBarID, HInst, NULL);
            CreateWindow(TRACKBAR_CLASS, "", WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_TOP | TBS_ENABLESELRANGE, 50,50,400,40, hWnd, (HMENU)TrackBarID, HInst, NULL);

            SendDlgItemMessage(hWnd, TrackBarID, TBM_SETRANGE, TRUE, MAKELONG(1,10));
            SendDlgItemMessage(hWnd, TrackBarID, TBM_SETSEL, TRUE, MAKELONG(3,5));

            Ret0:
            return 0;

        case WM_HSCROLL:
            {
            LPCSTR TbMsg=NULL;

            //Printf("TB wPrm=%d", LOWORD(wPrm));
            switch (LOWORD(wPrm))
                {
                case TB_BOTTOM:         TbMsg="TB_BOTTOM"; break;
                case TB_ENDTRACK:       TbMsg="TB_ENDTRACK"; break;
                case TB_LINEDOWN:       TbMsg="TB_LINEDOWN"; break;
                case TB_LINEUP:         TbMsg="TB_LINEUP"; break;
                case TB_PAGEDOWN:       TbMsg="TB_PAGEDOWN"; break;
                case TB_PAGEUP:         TbMsg="TB_PAGEUP"; break;
                case TB_THUMBPOSITION:  TbMsg="TB_THUMPOSITION"; break;
                case TB_THUMBTRACK:     TbMsg="TB_THUMBTRACK"; break;
                }

            if (TbMsg)
                {
                CHAR Buff[80];
                wsprintf(Buff, "Trackbar Msg: %s", TbMsg);
                SetDlgItemText(hWnd, StatusBarID, Buff);
                }
            goto Ret0;
            }

        case WM_COMMAND:
            switch (WMCMDID)
                {
                case SettingCmdID:  CreatePropertySheet(hWnd); break;
                case ExitCmdID:     SendMessage(hWnd, WM_CLOSE, 0, 0); break;
                case AboutCmdID:    DialogBox(HInst, MAKEINTRESOURCE(AboutDialog), hWnd, AboutDlgProc); break;
                }
            goto Ret0;

        case WM_DESTROY:
            PostQuitMessage(0);
            hWndMain=NULL;
            goto Ret0;
        }

    return DefWindowProc(hWnd, Msg, wPrm, lPrm);
    }




//-----------------------------------------------------------------------------
//          WinMain
//-----------------------------------------------------------------------------
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR CmdLine, int CmdShow)
    {
    MSG      M;
    HWND     hWnd;
    WNDCLASS WC;

    M.wParam=0;
    HInst=hInstance;

    InitCommonControls();

    WC.style        = 0;
    WC.lpfnWndProc  = MainWndProc;
    WC.cbClsExtra   = 0;
    WC.cbWndExtra   = 0;
    WC.hInstance    = hInstance;
    WC.hIcon        = LoadIcon(hInstance, MAKEINTRESOURCE(MainExeIconID));
    WC.hCursor      = LoadCursor(NULL, IDC_ARROW);
    WC.hbrBackground= (HBRUSH)GetStockObject(WHITE_BRUSH);
    WC.lpszMenuName = MAKEINTRESOURCE(MainMenuID);
    WC.lpszClassName= MainWndClsName;
    RegisterClass(&WC);

    if ((hWnd=CreateWindow(MainWndClsName, "프라퍼티시트/트랙바 예제",
              WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
              NULL, NULL, hInstance, NULL))==NULL) goto ProcExit;

    ShowWindow(hWnd, CmdShow);
    UpdateWindow(hWnd);

    while (GetMessage(&M, NULL, 0, 0))
        {
        TranslateMessage(&M);
        DispatchMessage(&M);
        }

    ProcExit:
    return M.wParam;
    }




//-----------------------------------------------------------------------------
//          실행파일에서 처음 실행되는 곳
//-----------------------------------------------------------------------------
int WinMainCRTStartup()
    {
    int Rslt;
    STARTUPINFO SI;

    SI.dwFlags=0;
    GetStartupInfo(&SI);
    Rslt=WinMain(GetModuleHandle(0), 0, GetCommandLine(),
        (BYTE)SI.dwFlags & STARTF_USESHOWWINDOW ? SI.wShowWindow:SW_SHOWDEFAULT);

    ExitProcess(Rslt);
    return Rslt;
    }


