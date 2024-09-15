// OpenGlCppCode.cpp: ���������� ���������������� ������� ��� ���������� DLL.
//

#include "stdafx.h"
#include <exception>
#include <map>
#include <memory>

#include "D3DRender.h"

#pragma comment(lib, "D3d11.lib")
#pragma comment(lib, "D2d1.lib")
#pragma comment(lib, "Dwrite.lib")

#define CPP_EXPORTS_API
#ifdef CPP_EXPORTS_API
#define CPP_API extern "C" __declspec(dllexport)
#else
#define CPP_API __declspec(dllimport)
#endif
using namespace std; 
extern map<HWND, shared_ptr<DX::Directx11>>* device = NULL;
CPP_API void InitDirectX(int handle)
{
	(*device)[(HWND) handle] = make_shared<DX::Directx11>((HWND)handle);
}

CPP_API void PrepareScene(int handle,int w,int h)
{
	auto finded = device->find((HWND) handle);
	if (finded != device->end())
	{
		finded->second->CreateTarget(w, h);
		finded->second->ClearAll();
		vector<array<double, 3>> xyz;
		xyz.resize(8);  // ����� 24 ������� ��� 12 �������������

		double size = 1.0;
		for (int i = 0; i < 8; ++i) {
			double x = ((i & 1) ? size : -size) / 2.0;
			double y = ((i & 2) ? size : -size) / 2.0;
			double z = ((i & 4) ? size : -size) / 2.0;

			xyz[i] = { x, y, z };
		}

		finded->second->RenderStart();

		auto unit = finded->second->CreateCubeColorUnit(xyz);
		finded->second->AddToSaved(unit);
		finded->second->RenderSavedData();
		finded->second->EndRender();
	}
}
CPP_API void RenderScene(int handle)
{
	auto finded = device->find((HWND) handle);
	if (finded != device->end())
	{
		DirectX::XMStoreFloat4x4(&(finded->second->ModelViewMatrix), DirectX::XMMatrixIdentity());
		DirectX::XMStoreFloat4x4(&(finded->second->ProjectionMatrix), DirectX::XMMatrixIdentity());

		finded->second->RenderSavedData();
		//finded->second->RotateByTime();
		finded->second->EndRender();
	}
}
