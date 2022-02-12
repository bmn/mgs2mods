#pragma comment(lib, "comctl32.lib")
//#include <Prsht.h>
#include <wtypes.h>

//extern HINSTANCE g_hinst;
HINSTANCE g_hinst;

VOID DoPropertySheet(HWND hwndOwner)
{
    PROPSHEETPAGEA psp[2];

    PROPSHEETHEADERA psh;

    psp[0].dwSize = sizeof(PROPSHEETPAGEA);
    psp[0].dwFlags = PSP_USEICONID | PSP_USETITLE;
    psp[0].hInstance = g_hinst;
    //psp[0].pszTemplate = MAKEINTRESOURCEA(DLG_FONT);
    //psp[0].pszIcon = MAKEINTRESOURCEA(IDI_FONT);
    //psp[0].pfnDlgProc = FontDialogProc;
    //psp[0].pszTitle = MAKEINTRESOURCEA(IDS_FONT);
    psp[0].lParam = 0;
    psp[0].pfnCallback = NULL;
    psp[1].dwSize = sizeof(PROPSHEETPAGEA);
    psp[1].dwFlags = PSP_USEICONID | PSP_USETITLE;
    psp[1].hInstance = g_hinst;
    //psp[1].pszTemplate = MAKEINTRESOURCEA(DLG_BORDER);
    //psp[1].pszIcon = MAKEINTRESOURCEA(IDI_BORDER);
    //psp[1].pfnDlgProc = BorderDialogProc;
    //psp[1].pszTitle = MAKEINTRESOURCEA(IDS_BORDER);
    psp[1].lParam = 0;
    psp[1].pfnCallback = NULL;

    psh.dwSize = sizeof(PROPSHEETHEADERA);
    psh.dwFlags = PSH_USEICONID | PSH_PROPSHEETPAGE;
    psh.hwndParent = hwndOwner;
    psh.hInstance = g_hinst;
    //psh.pszIcon = MAKEINTRESOURCEA(IDI_CELL_PROPERTIES);
    psh.pszCaption = (LPSTR)"Cell Properties";
    psh.nPages = sizeof(psp) / sizeof(PROPSHEETPAGEA);
    psh.nStartPage = 0;
    psh.ppsp = (LPCPROPSHEETPAGEA)&psp;
    psh.pfnCallback = NULL;

    PropertySheetA(&psh);

    return;
}