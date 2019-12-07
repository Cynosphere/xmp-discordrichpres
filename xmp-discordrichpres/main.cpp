#include <windows.h>
#include <cstdlib>
#include "xmp-sdk/xmpdsp.h" // requires the XMPlay "DSP/general plugin SDK"
#include "rpc.h"

static XMPFUNC_MISC *xmpfmisc;

static HINSTANCE ghInstance;

static HWND xmpwin;
static HHOOK hook;

static void WINAPI DSP_About(HWND win);
static void *WINAPI DSP_New(void);
static void WINAPI DSP_Free(void *inst);
static const char *WINAPI DSP_GetDescription(void *inst);
static void WINAPI DSP_Config(void *inst, HWND win);
static DWORD WINAPI DSP_GetConfig(void *inst, void *config);
static BOOL WINAPI DSP_SetConfig(void *inst, void *config, DWORD size);

typedef struct {
} Config;
static Config ourConfig;

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
};

bool init = false;

static void WINAPI SetNowPlaying(bool close)
{
	char *title = NULL;

	if (!close) {
		if (init == false) {
			InitDiscord();
			init = true;
		}

		if (!title) title = xmpfmisc->GetTag(TAG_FORMATTED_TITLE); // get track title
	}

	//int length = strtol(xmpfmisc->GetTag(TAG_LENGTH), NULL, 10);

	UpdatePresence(title);

	if (title) {
		xmpfmisc->Free(title);
	}
}

static LRESULT CALLBACK HookProc(int n, WPARAM w, LPARAM l)
{
	if (n == HC_ACTION) {
		CWPSTRUCT *cwp = (CWPSTRUCT*)l;
		if (cwp->message == WM_SETTEXT && cwp->hwnd == xmpwin) // title change
			SetNowPlaying(FALSE);
	}
	return CallNextHookEx(hook, n, w, l);
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

	SetNowPlaying(FALSE);

	// setup hook to catch title changes
	hook = SetWindowsHookEx(WH_CALLWNDPROC, &HookProc, NULL, GetWindowThreadProcessId(xmpwin, NULL));

	return (void*)1;
}

static void WINAPI DSP_Free(void *inst)
{
	UnhookWindowsHookEx(hook);
	SetNowPlaying(TRUE);
}

static void WINAPI DSP_Config(void *inst, HWND win)
{
	MessageBoxA(win, "Nothing to configure (yet).", "Discord Rich Presence", MB_OK | MB_ICONINFORMATION);
}

static DWORD WINAPI DSP_GetConfig(void *inst, void *config)
{
	memcpy(config, &ourConfig, sizeof(ourConfig));
	return sizeof(ourConfig); // return size of config info
}

static BOOL WINAPI DSP_SetConfig(void *inst, void *config, DWORD size)
{
	memcpy(&ourConfig, config, min(size, sizeof(ourConfig)));
	SetNowPlaying(FALSE);
	return TRUE;
}

// get the plugin's XMPDSP interface
#ifdef __cplusplus
extern "C"
#endif
XMPDSP *WINAPI XMPDSP_GetInterface2(DWORD face, InterfaceProc faceproc)
{
	if (face != XMPDSP_FACE) return NULL;
	xmpfmisc = (XMPFUNC_MISC*)faceproc(XMPFUNC_MISC_FACE); // import "misc" functions
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