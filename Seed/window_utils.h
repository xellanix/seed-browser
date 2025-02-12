#pragma once

#ifndef XELLANIX_WINDOW_UTILS_H
#define XELLANIX_WINDOW_UTILS_H

#include <Windows.h>
#include <ShlObj.h>

namespace xellanix::desktop
{
	struct __declspec(uuid("45D64A29-A63E-4CB6-B498-5781D298CB4F")) __declspec(novtable)
		ICoreWindowInterop : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE get_WindowHandle(HWND* hwnd) = 0;
		virtual HRESULT STDMETHODCALLTYPE put_MessageHandled(unsigned char value) = 0;
	};

	struct __declspec(uuid("3E68D4BD-7135-4D10-8018-9FB6D9F33FA1")) __declspec(novtable)
		IInitializeWithWindow : public IUnknown
	{
	public:
		virtual HRESULT STDMETHODCALLTYPE Initialize(__RPC__in HWND hwnd) = 0;
	};

	inline HWND WindowHandle = nullptr;
}

namespace xd = xellanix::desktop;

#endif // !XELLANIX_WINDOW_UTILS_H
