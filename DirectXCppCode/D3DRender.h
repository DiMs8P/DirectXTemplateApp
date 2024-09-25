#pragma once
//#include <D3D9.h>
#include <DXGI.h>
#include <D3D11.h>
#include <D2D1.h>
#include <Dwrite.h>
#include <directxmath.h>
#include <atlbase.h>
#include <memory>
#include <vector>
#include <array>
#include <chrono>
#include <ctime>
#include <string>
#include <cmath>

namespace DX
{
    namespace Shaders
    {
#include "Shaders/VertexShader.hvs"
#include "Shaders/PixelShader.hps"
    }

    inline void CheckHR(HRESULT hr)
    {
        if (hr < 0)
        {
            //		if(hr == D2DERR_RECREATE_TARGET)return;
            if (hr == E_OUTOFMEMORY)throw std::exception("DirectX error Out of Memory");
            if (hr == E_INVALIDARG)throw std::exception("DirectX error InvalidArgs");
            char buf[100];
            throw std::exception(("DirectX error" + std::string(_itoa(hr, buf, 10))).c_str());
        }
    }

    class Directx11
    {
    public:
        struct RenderingUnit
        {
            CComPtr<ID3D11Buffer> vertexBuffer;
            CComPtr<ID3D11Buffer> indexBuffer;

            enum class UnitType { Triangle } uType;

            UINT IndexCount;
        };

    protected:
        struct VertexForColorTriangle
        {
            DirectX::XMFLOAT3 Pos;
            DirectX::XMFLOAT3 Normal;
            DirectX::XMFLOAT4 Color;
        };

        struct ConstantBufferStruct
        {
            DirectX::XMMATRIX mView;
            DirectX::XMMATRIX mProjection;
            DirectX::XMMATRIX mModelMatrix;
        };

        CComPtr<ID3D11Device> device;
        CComPtr<ID3D11DeviceContext> context;
        CComPtr<IDXGISwapChain> swapChain;

        CComPtr<ID3D11Texture2D> renderTarget;
        CComPtr<ID3D11Texture2D> depthStencil;
        CComPtr<ID3D11RenderTargetView> renderTargetView;
        CComPtr<ID3D11DepthStencilView> depthStencilView;


        CComPtr<ID3D11ShaderResourceView> textureView;


        CComPtr<ID2D1RenderTarget> renderTarget2D;
        CComPtr<ID2D1Factory> factory2D;

        CComPtr<ID3D11InputLayout> vertexLayout;
        CComPtr<ID3D11VertexShader> vertexShader;
        CComPtr<ID3D11PixelShader> pixelShader;


        CComPtr<ID3D11Buffer> constantBufferForVertexShader;

        CComPtr<ID3D11RasterizerState> rasterizerState;

        std::vector<RenderingUnit> SavedScene;


        CComPtr<ID2D1Brush> current2DBrush;
        CComPtr<IDWriteFactory> writeFactory;
        CComPtr<IDWriteTextFormat> textFormat;

        D2D1::Matrix3x2F transformationMatrix2D;

        void CreateShaders()
        {
            vertexShader.Release();
            pixelShader.Release();

            CheckHR(device->CreateVertexShader(Shaders::VertexShader, sizeof(Shaders::VertexShader), nullptr,
                                               &vertexShader));
            CheckHR(
                device->CreatePixelShader(Shaders::PixelShader, sizeof(Shaders::PixelShader), nullptr, &pixelShader));

            D3D11_INPUT_ELEMENT_DESC layout[] =
            {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
                {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
            };

            CheckHR(device->CreateInputLayout(layout, ARRAYSIZE(layout), Shaders::VertexShader,
                                              sizeof(Shaders::VertexShader), &vertexLayout));
        }

    public:
        Directx11(HWND hWnd)
        {
            D3D_FEATURE_LEVEL FeatureLevels[] = {
                D3D_FEATURE_LEVEL_11_0,
                D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0,
                D3D_FEATURE_LEVEL_9_3, D3D_FEATURE_LEVEL_9_2, D3D_FEATURE_LEVEL_9_1
            };
            D3D_FEATURE_LEVEL pFeatureLevel;

            RECT rc;
            GetClientRect(hWnd, &rc);
            UINT width = rc.right - rc.left;
            UINT height = rc.bottom - rc.top;

            DXGI_SWAP_CHAIN_DESC sd;
            ZeroMemory(&sd, sizeof(sd));
            sd.BufferCount = 1;
            sd.BufferDesc.Width = width;
            sd.BufferDesc.Height = height;
            sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            sd.BufferDesc.RefreshRate.Numerator = 60;
            sd.BufferDesc.RefreshRate.Denominator = 1;
            sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            sd.OutputWindow = hWnd;
            sd.SampleDesc.Count = 1;
            sd.SampleDesc.Quality = 0;
            sd.Windowed = TRUE;

            //CheckHR(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, FeatureLevels, 6,
            //		D3D11_SDK_VERSION, &sd, &swapChain, &device, &pFeatureLevel, &context));
            CheckHR(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                                                  FeatureLevels, 6,
                                                  D3D11_SDK_VERSION, &sd, &swapChain, &device, &pFeatureLevel,
                                                  &context));

            CreateShaders();

            D3D11_RASTERIZER_DESC rdesc;
            ZeroMemory(&rdesc, sizeof(rdesc));
            rdesc.CullMode=D3D11_CULL_MODE::D3D11_CULL_BACK;
            rdesc.FillMode = D3D11_FILL_SOLID;
            rdesc.AntialiasedLineEnable = false;
            rdesc.DepthClipEnable = false;
            rdesc.MultisampleEnable = false;

            CheckHR(device->CreateRasterizerState(&rdesc, &rasterizerState));

            factory2D.Release();
            CheckHR(D2D1CreateFactory(D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory2D));

            writeFactory.Release();
            CheckHR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
                                        reinterpret_cast<IUnknown**>(&writeFactory)));

            constantBufferForVertexShader.Release();

            int sz1 = sizeof(ConstantBufferStruct);
            CD3D11_BUFFER_DESC bufferDesc(sz1, D3D11_BIND_CONSTANT_BUFFER);
            CheckHR(device->CreateBuffer(&bufferDesc, nullptr, &constantBufferForVertexShader));
            context->VSSetConstantBuffers(0, 1, &constantBufferForVertexShader.p);
        }

        HRESULT CreateDDSTextureFromFile(ID3D11Device* d3dDevice, const wchar_t* fileName, ID3D11Resource** texture,
                                         ID3D11ShaderResourceView** textureView, size_t maxsize = 0);

        void CreateTarget(int width, int height)
        {
            renderTarget.Release();
            depthStencil.Release();
            renderTargetView.Release();
            depthStencilView.Release();

            //CD3D11_TEXTURE2D_DESC renderDesc(DXGI_FORMAT_B8G8R8A8_UNORM, width, height, 1, 1,
            //	D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE, D3D11_USAGE_DEFAULT,
            //	0, 1, 0, D3D11_RESOURCE_MISC_SHARED);
            //CheckHR(device->CreateTexture2D(&renderDesc, nullptr, &renderTarget));


            // Create a render target view
            //		ID3D11Texture2D* pBackBuffer = NULL;
            CheckHR(swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&renderTarget));
            CheckHR(device->CreateRenderTargetView(renderTarget, nullptr, &renderTargetView));

            CComPtr<IDXGISurface> surface;
            CheckHR(renderTarget->QueryInterface(&surface));


            D2D1_RENDER_TARGET_PROPERTIES renderTargetProperties = D2D1::RenderTargetProperties(
                D2D1_RENDER_TARGET_TYPE_DEFAULT,
                D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

            renderTarget2D.Release();
            CheckHR(factory2D->CreateDxgiSurfaceRenderTarget(surface, &renderTargetProperties, &renderTarget2D));


            CD3D11_TEXTURE2D_DESC depthDesc(DXGI_FORMAT_D24_UNORM_S8_UINT, width, height, 1, 1,
                                            D3D11_BIND_DEPTH_STENCIL);

            CheckHR(device->CreateTexture2D(&depthDesc, nullptr, &depthStencil));
            CheckHR(device->CreateDepthStencilView(depthStencil, nullptr, &depthStencilView));

            CD3D11_VIEWPORT viewport(renderTarget, renderTargetView);
            context->OMSetRenderTargets(1, &renderTargetView.p, depthStencilView);
            context->RSSetViewports(1, &viewport);

            current2DBrush.Release();
            CComPtr<ID2D1SolidColorBrush> scbrush;
            CheckHR(renderTarget2D->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &scbrush));
            current2DBrush = scbrush;

            // TODO remove redundant files
            CheckHR(CreateDDSTextureFromFile(device, L"..\\Saved\\TextureExample5.dds", NULL, &textureView));
        }

        void SetNewSolidBrush(D2D1::ColorF col)
        {
            current2DBrush.Release();
            CComPtr<ID2D1SolidColorBrush> scbrush;
            CheckHR(renderTarget2D->CreateSolidColorBrush(col, &scbrush));
            current2DBrush = scbrush;
        }

        void Clear()
        {
            FLOAT colorRGBA[] = { 1.0f,1.0f,1.0f,1.0f };
            context->ClearRenderTargetView(renderTargetView, colorRGBA);
            context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
            context->RSSetState(rasterizerState);
        }

        void ClearAll()
        {
            SavedScene.clear();
            Clear();
        }

        void UpdateProectionsAndLightingData()
        {
            ConstantBufferStruct cb;
            cb.mView = XMMatrixTranspose(XMLoadFloat4x4(&ViewMatrix));
            cb.mProjection = XMMatrixTranspose(XMLoadFloat4x4(&ProjectionMatrix));
            cb.mModelMatrix = XMMatrixTranspose(XMLoadFloat4x4(&ModelMatrix));
            context->UpdateSubresource(constantBufferForVertexShader, 0, nullptr, &cb, 0, 0);
        }

        void RenderStart()
        {
            FLOAT colorRGBA[] = {1.0f, 1.0f, 1.0f, 1.0f};
            context->ClearRenderTargetView(renderTargetView, colorRGBA);
            context->ClearDepthStencilView(depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
        }

        void EndRender()
        {
            context->Flush();
            swapChain->Present(0, 0);
        }

        void RenderUnit(RenderingUnit& unit)
        {
            if (unit.IndexCount == 0)return;
            switch (unit.uType)
            {
            case RenderingUnit::UnitType::Triangle:
                {
                    UINT stride = sizeof(VertexForColorTriangle);
                    UINT offset = 0;

                    context->IASetVertexBuffers(0, 1, &unit.vertexBuffer.p, &stride, &offset);
                    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
                    context->IASetIndexBuffer(unit.indexBuffer, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
                    context->IASetInputLayout(vertexLayout);
                    context->VSSetShader(vertexShader, nullptr, 0);
                    context->PSSetShader(pixelShader, nullptr, 0);
                    context->PSSetShaderResources(0, 1, &textureView.p); //�������� �������� � ������

                    context->DrawIndexed(unit.IndexCount, 0, 0);
                }
                break;
            }
        }
        
        void RotateByTime(double deltaTime) {
            static float angleX = 0.0f;
            static float angleY = 0.0f;

            float rotationSpeed = 1.5f;
            angleX += rotationSpeed * deltaTime;
            angleY += rotationSpeed * deltaTime / 2;

        	angleX = std::fmodf(angleX, 360.0f);
        	angleY = std::fmodf(angleY, 360.0f);

            DirectX::XMMATRIX rotX = DirectX::XMMatrixRotationX(angleX);
            DirectX::XMMATRIX rotY = DirectX::XMMatrixRotationY(angleY);

            DirectX::XMMATRIX worldMatrix = rotX * rotY;

            XMStoreFloat4x4(&ModelMatrix, worldMatrix);
        }


        RenderingUnit CreateCubeColorUnit(std::vector<std::array<double, 3>>& all_points)
        {
            RenderingUnit unit;
		switch (all_points.size())
		{
		case 8:
			unit.uType = RenderingUnit::UnitType::Triangle;
			break;
		default:
			break;
		}
		unit.IndexCount = 36;
		if (unit.IndexCount == 0)return unit;

		std::vector<VertexForColorTriangle> vertarray =
		{
			{DirectX::XMFLOAT3(all_points[0][0], all_points[0][1], all_points[0][2]), DirectX::XMFLOAT3(-1, +0, +0), DirectX::XMFLOAT4(0, 0.5, 0, 0)},
			{DirectX::XMFLOAT3(all_points[2][0], all_points[2][1], all_points[2][2]), DirectX::XMFLOAT3(-1, +0, +0), DirectX::XMFLOAT4(0.32, 0.5, 0, 0)},
			{DirectX::XMFLOAT3(all_points[4][0], all_points[4][1], all_points[4][2]), DirectX::XMFLOAT3(-1, +0, +0), DirectX::XMFLOAT4(0, 0, 0, 0)},
			{DirectX::XMFLOAT3(all_points[6][0], all_points[6][1], all_points[6][2]), DirectX::XMFLOAT3(-1, +0, +0), DirectX::XMFLOAT4(0.32, 0, 0, 0)},

			{DirectX::XMFLOAT3(all_points[5][0], all_points[5][1], all_points[5][2]), DirectX::XMFLOAT3(+1, +0, +0), DirectX::XMFLOAT4(0.64, 0.5, 0, 0)},
			{DirectX::XMFLOAT3(all_points[7][0], all_points[7][1], all_points[7][2]), DirectX::XMFLOAT3(+1, +0, +0), DirectX::XMFLOAT4(0.34, 0.5, 0, 0)},
			{DirectX::XMFLOAT3(all_points[3][0], all_points[3][1], all_points[3][2]), DirectX::XMFLOAT3(+1, +0, +0), DirectX::XMFLOAT4(0.34, 0, 0, 0)},
			{DirectX::XMFLOAT3(all_points[1][0], all_points[1][1], all_points[1][2]), DirectX::XMFLOAT3(+1, +0, +0), DirectX::XMFLOAT4(0.64, 0, 0, 0)},

			{DirectX::XMFLOAT3(all_points[5][0], all_points[5][1], all_points[5][2]), DirectX::XMFLOAT3(+0, -1, +0), DirectX::XMFLOAT4(0.67, 0.5, 0, 0)},
			{DirectX::XMFLOAT3(all_points[1][0], all_points[1][1], all_points[1][2]), DirectX::XMFLOAT3(+0, -1, +0), DirectX::XMFLOAT4(0.99, 0.5, 0, 0)},
			{DirectX::XMFLOAT3(all_points[0][0], all_points[0][1], all_points[0][2]), DirectX::XMFLOAT3(+0, -1, +0), DirectX::XMFLOAT4(0.99, 0, 0, 0)},
			{DirectX::XMFLOAT3(all_points[4][0], all_points[4][1], all_points[4][2]), DirectX::XMFLOAT3(+0, -1, +0), DirectX::XMFLOAT4(0.67, 0, 0, 0)},

			{DirectX::XMFLOAT3(all_points[7][0], all_points[7][1], all_points[7][2]), DirectX::XMFLOAT3(+0, +1, +0), DirectX::XMFLOAT4(0, 0.5, 0, 0)},
			{DirectX::XMFLOAT3(all_points[6][0], all_points[6][1], all_points[6][2]), DirectX::XMFLOAT3(+0, +1, +0), DirectX::XMFLOAT4(0, 0.99, 0, 0)},
			{DirectX::XMFLOAT3(all_points[2][0], all_points[2][1], all_points[2][2]), DirectX::XMFLOAT3(+0, +1, +0), DirectX::XMFLOAT4(0.32, 0.99, 0, 0)},
			{DirectX::XMFLOAT3(all_points[3][0], all_points[3][1], all_points[3][2]), DirectX::XMFLOAT3(+0, +1, +0), DirectX::XMFLOAT4(0.32, 0.5, 0, 0)},

			{DirectX::XMFLOAT3(all_points[0][0], all_points[0][1], all_points[0][2]), DirectX::XMFLOAT3(+0, +0, -1), DirectX::XMFLOAT4(0.34, 0.5, 0, 0)},
			{DirectX::XMFLOAT3(all_points[2][0], all_points[2][1], all_points[2][2]), DirectX::XMFLOAT3(+0, +0, -1), DirectX::XMFLOAT4(0.34, 0.95, 0, 0)},
			{DirectX::XMFLOAT3(all_points[3][0], all_points[3][1], all_points[3][2]), DirectX::XMFLOAT3(+0, +0, -1), DirectX::XMFLOAT4(0.62, 0.95, 0, 0)},
			{DirectX::XMFLOAT3(all_points[1][0], all_points[1][1], all_points[1][2]), DirectX::XMFLOAT3(+0, +0, -1), DirectX::XMFLOAT4(0.62, 0.5, 0, 0)},

			{DirectX::XMFLOAT3(all_points[5][0], all_points[5][1], all_points[5][2]), DirectX::XMFLOAT3(+0, +0, +1), DirectX::XMFLOAT4(0.67, 0.98, 0, 0)},
			{DirectX::XMFLOAT3(all_points[7][0], all_points[7][1], all_points[7][2]), DirectX::XMFLOAT3(+0, +0, +1), DirectX::XMFLOAT4(0.67, 0.5, 0, 0)},
			{DirectX::XMFLOAT3(all_points[6][0], all_points[6][1], all_points[6][2]), DirectX::XMFLOAT3(+0, +0, +1), DirectX::XMFLOAT4(0.99, 0.5, 0, 0)},
			{DirectX::XMFLOAT3(all_points[4][0], all_points[4][1], all_points[4][2]), DirectX::XMFLOAT3(+0, +0, +1), DirectX::XMFLOAT4(0.99, 0.98, 0, 0)},
			
		};

        std::vector<DWORD> indexarray =
		{
			0, 2, 1,
			2, 3, 1,

			4, 6, 5,
			6, 4, 7,

			8, 10, 9,
			10, 8, 11,

			12, 14, 13,
			14, 12, 15,

			16, 17, 18,
			18, 19, 16,

			20, 21, 22,
			22, 23, 20,
		};

		CD3D11_BUFFER_DESC bufferDesc((int)vertarray.size() * sizeof(VertexForColorTriangle), D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA data;

		data.pSysMem = vertarray.data();
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;
		CheckHR(device->CreateBuffer(&bufferDesc, &data, &unit.vertexBuffer));

		{
			CD3D11_BUFFER_DESC bufferDesc((int)indexarray.size() * sizeof(DWORD), D3D11_BIND_INDEX_BUFFER);
			D3D11_SUBRESOURCE_DATA data;
			data.pSysMem = indexarray.data();
			data.SysMemPitch = 0;
			data.SysMemSlicePitch = 0;
			CheckHR(device->CreateBuffer(&bufferDesc, &data, &unit.indexBuffer));
		}
		return unit;
        }

        void RenderScene(std::vector<RenderingUnit>& scene)
        {
            UpdateProectionsAndLightingData();
            for (auto& i : scene)RenderUnit(i);
        }

        void AddToSaved(RenderingUnit& unit)
        {
            SavedScene.push_back(unit);
        }

        void RenderSavedData()
        {
            RenderScene(SavedScene);
            context->Flush();

            swapChain->Present(0, 0);
        }

        DirectX::XMFLOAT4X4 ProjectionMatrix;
        DirectX::XMFLOAT4X4 ViewMatrix;
        DirectX::XMFLOAT4X4 ModelMatrix;
    };
}
