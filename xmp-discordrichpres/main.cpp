#include <windows.h>
#include <cstdlib>
#include "xmp-sdk/xmpdsp.h" // requires the XMPlay "DSP/general plugin SDK"
#include "rpc.h"

static XMPFUNC_MISC *xmpfmisc;
static XMPFUNC_STATUS *xmpfstatus;

static HINSTANCE ghInstance;
static HWND confwin = 0;

static HWND xmpwin;

static BOOL CALLBACK DSPDialogProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void WINAPI DSP_About(HWND win);
static void *WINAPI DSP_New(void);
static void WINAPI DSP_Free(void *inst);
static const char *WINAPI DSP_GetDescription(void *inst);
static void WINAPI DSP_Config(void *inst, HWND win);
static DWORD WINAPI DSP_GetConfig(void *inst, void *config);
static BOOL WINAPI DSP_SetConfig(void *inst, void *config, DWORD size);
static void WINAPI DSP_NewTrack(void *inst, const char *file);
static void WINAPI DSP_Reset(void *inst);
static void WINAPI DSP_NewTitle(void *inst, const char *title);

typedef struct {
	BOOL useFileName;
} Config;
static Config conf;

static XMPDSP dsp = {
	XMPDSP_FLAG_NODSP,
	"Discord Rich Presence",
	DSP_About,
	DSP_New,
	DSP_Free,
	DSP_GetDescription,
	DSP_Config,
	DSP_GetConfig,
	DSP_SetConfig,
	DSP_NewTrack,
	NULL,
	DSP_Reset,
	NULL,
	DSP_NewTitle,
};

bool init = false;

static void WINAPI SetNowPlaying(BOOL close)
{
	char *title = NULL;

	if (!close) {
		if (init == false) {
			InitDiscord();
			init = true;
		}

		if (!title) {
			if (conf.useFileName == TRUE) {
				title = xmpfmisc->GetTag(TAG_FILENAME);
			}
			else {
				title = xmpfmisc->GetTag(TAG_FORMATTED_TITLE);
			}
		}
	} else {
		ClearPresence();
	}


	if (title) {
		char *len = xmpfmisc->GetTag(TAG_LENGTH);
		int length = strtol(len, NULL, 10);
		int pos = xmpfstatus->GetTime();
		char *type = xmpfmisc->GetTag(TAG_FILETYPE);
		DWORD _version = xmpfmisc->GetVersion();

		int patch = (_version & 0xFF);
		int rev = (_version & 0xFF00) >> 8;
		int minor = (_version & 0xFF0000) >> 16;
		int major = (_version & 0xFF000000) >> 24;

		char version[256];

		sprintf_s(version, 256, "%d.%d.%d.%d", major, minor, rev, patch);

		UpdatePresence(title, type, length, pos, version);

		xmpfmisc->Free(title);
		xmpfmisc->Free(len);
	}
}

static void WINAPI DSP_About(HWND win)
{
	MessageBoxA(win, "xmp-discordrichpres by Cynthia (Cynosphere)\nBased off of MSN plugin from the SDK", "About Discord Rich Presence", MB_OK | MB_ICONINFORMATION);
}

static const char *WINAPI DSP_GetDescription(void *inst)
{
	return dsp.name;
}

static void *WINAPI DSP_New()
{
	xmpwin = xmpfmisc->GetWindow();

	conf.useFileName = FALSE;
	SetNowPlaying(FALSE);

	return (void*)1;
}

#define MESS(id,m,w,l) SendDlgItemMessage(h,id,m,(WPARAM)(w),(LPARAM)(l))

static BOOL CALLBACK DSPDialogProc(HWND h, UINT m, WPARAM w, LPARAM l)
{
	switch (m) {
		case WM_COMMAND:
			switch (LOWORD(w)) {
				case IDOK:
					conf.useFileName = (BST_CHECKED == MESS(10, BM_GETCHECK, 0, 0));
					SetNowPlaying(FALSE);
				case IDCANCEL:
					EndDialog(h, 0);
					break;
				}
			break;
		case WM_INITDIALOG:
			confwin = h;
			MESS(10, BM_SETCHECK, conf.useFileName, 0);
			return TRUE;

		case WM_DESTROY:
			confwin = 0;
			break;
	}
	return FALSE;
}

static void WINAPI DSP_Free(void *inst)
{
	if (confwin) EndDialog(confwin, 0);
	SetNowPlaying(TRUE);
}

static void WINAPI DSP_Config(void *inst, HWND win)
{
	DialogBox(ghInstance, MAKEINTRESOURCE(1000), win, &DSPDialogProc);
}

static DWORD WINAPI DSP_GetConfig(void *inst, void *config)
{
	memcpy(config, &conf, sizeof(conf));
	return sizeof(conf); // return size of config info
}

static BOOL WINAPI DSP_SetConfig(void *inst, void *config, DWORD size)
{
	memcpy(&conf, config, min(size, sizeof(conf)));
	SetNowPlaying(FALSE);
	return TRUE;
}

static void WINAPI DSP_Reset(void *inst)
{
	SetNowPlaying(FALSE);
}

static void WINAPI DSP_NewTrack(void *inst, const char *file)
{
	if (file == NULL) {
		SetNowPlaying(TRUE);
	}
	else {
		SetNowPlaying(FALSE);
	}
}

static void WINAPI DSP_NewTitle(void *inst, const char *title) {
	SetNowPlaying(FALSE);
}

// get the plugin's XMPDSP interface
#ifdef __cplusplus
extern "C"
#endif
XMPDSP *WINAPI XMPDSP_GetInterface2(DWORD face, InterfaceProc faceproc)
{
	if (face != XMPDSP_FACE) return NULL;
	xmpfmisc = (XMPFUNC_MISC*)faceproc(XMPFUNC_MISC_FACE); // import "misc" functions
	xmpfstatus = (XMPFUNC_STATUS*)faceproc(XMPFUNC_STATUS_FACE); // import "status" functions
	return &dsp;
}

BOOL WINAPI DllMain(HINSTANCE hDLL, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH) {
		ghInstance = hDLL;
		DisableThreadLibraryCalls(hDLL);
	}
	return TRUE;
}