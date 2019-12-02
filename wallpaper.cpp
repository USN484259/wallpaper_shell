#include "wallpaper.h"
#include <locale>
#include <codecvt>

#include <Windows.h>
#include <wininet.h>
#include <shlobj.h>

using namespace std;


Wallpaper::Wallpaper(void) {
	CoInitialize(NULL);
}

Wallpaper::~Wallpaper(void) {
	CoUninitialize();
}

bool Wallpaper::changeable(void) const {
	do {
		SYSTEM_POWER_STATUS ps;
		if (!GetSystemPowerStatus(&ps))
			break;
		if (ps.ACLineStatus == 0)
			break;

		if (FindWindow(TEXT("CabinetWClass"), NULL))
			break;


		return true;
	} while (false);


	return false;
}


bool Wallpaper::set(const string& str) {
	bool suc = false;
	IActiveDesktop *pIAD = nullptr;

	do {
		
		wstring_convert<codecvt_utf8_utf16 <wchar_t> > converter;

		wstring result = converter.from_bytes(str);

		if (result.empty())
			break;

		HRESULT hr;

		hr = CoCreateInstance(CLSID_ActiveDesktop, NULL, CLSCTX_INPROC_SERVER, IID_IActiveDesktop, (void**)&pIAD);
		if (!SUCCEEDED(hr))
			break;
		
		hr = pIAD->SetWallpaper(result.c_str(), 0);

		if (!SUCCEEDED(hr))
			break;
		WALLPAPEROPT wpo;
		wpo.dwSize = sizeof(wpo);
		wpo.dwStyle = WPSTYLE_KEEPASPECT;
		hr = pIAD->SetWallpaperOptions(&wpo, 0);
		if (!SUCCEEDED(hr))
			break;
		hr = pIAD->ApplyChanges(AD_APPLY_ALL);
		if (!SUCCEEDED(hr))
			break;

		//DWORD_PTR p;
		//SendMessageTimeout(HWND_BROADCAST, WM_SETTINGCHANGE, 0, 0, SMTO_NOTIMEOUTIFNOTHUNG, 0, &p);



	} while (suc=true,false);

	pIAD->Release();

	return suc;
}