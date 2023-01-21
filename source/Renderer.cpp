#include "pch.h"
#include "Renderer.h"
#include "Utils.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow)
		: m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		IDXGIFactory1* pDxgiFactory{};
		const HRESULT result = InitializeDirectX(pDxgiFactory);
		if (pDxgiFactory) pDxgiFactory->Release();

		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		//Initialize Software pipeline
		InitializeSoftware();
	}

	Renderer::~Renderer()
	{
		//if (m_pScene) delete m_pScene;

		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		if (m_pRenderTargetBuffer) m_pRenderTargetBuffer->Release();

		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pDepthStencilBuffer) m_pDepthStencilBuffer->Release();

		if (m_pSwapChain) m_pSwapChain->Release();

		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		if (m_pDevice) m_pDevice->Release();
	}

	void Renderer::Update(const Timer* pTimer)
	{
		//m_pScene->Update(pTimer);

		if (m_IsPrintFPS)
		{
			m_PrintTimer -= pTimer->GetElapsed();
			if (m_PrintTimer <= 0.f)
			{
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, m_AttributeFPS);

				m_PrintTimer = m_PrintInterval;
				std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		switch (m_RasterizerMode)
		{
		case RasterizerMode::hardware:
			RenderHardware();
			break;

		case RasterizerMode::software:
			RenderSoftware();
			break;
		}
	}

#pragma region SHARED
	// Public
	void Renderer::ToggleRasterizerMode()
	{
		m_RasterizerMode = RasterizerMode(((int)m_RasterizerMode + 1) % (int)RasterizerMode::END);

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeShared);
		std::string s = (m_RasterizerMode == RasterizerMode::hardware) ? "HARDWARE" : "SOFTWARE";
		std::cout << "**(SHARED) Rasterizer Mode = " << s << std::endl;
	}

	void Renderer::ToggleRotation()
	{
		//TODO: implement function
		bool isRotating = true;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeShared);
		std::string s = (isRotating) ? "ON" : "OFF";
		std::cout << "**(SHARED) Vehicle Rotation " << s << std::endl;
	}

	void Renderer::CycleCullMode()
	{
		//TODO: implement function
		bool cullmode = true;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeShared);
		std::string s;
		switch (cullmode)
		{
			//s = "NONE";
			//s = "FRONT";
			//s = "BACK";
		default:
			break;
		}
		std::cout << "**(SHARED) CullMode = " << s << std::endl;
	}

	void Renderer::ToggleUniformClearColor()
	{
		m_IsUniformClearColor = !m_IsUniformClearColor;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeShared);
		std::string s = (m_IsUniformClearColor) ? "ON" : "OFF";
		std::cout << "**(SHARED) Uniform ClearColor " << s << std::endl;
	}

	void Renderer::TogglePrintFPS()
	{
		m_IsPrintFPS = !m_IsPrintFPS;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeShared);
		std::string s = (m_IsPrintFPS) ? "ON" : "OFF";
		std::cout << "**(SHARED) Print FPS " << s << std::endl;
	}


	// Private
#pragma endregion

#pragma region HARDWARE
	// Public
	void Renderer::ToggleFireFX()
	{
		//TODO: implement function
		bool isFire = true;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeHardware);
		std::string s = (isFire) ? "ON" : "OFF";
		std::cout << "**(HARDWARE) FireFX " << s << std::endl;
	}

	void Renderer::CycleSamplerState()
	{
		//TODO: implement function
		bool sampler = true;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeHardware);
		std::string s;
		switch (sampler)
		{
			//s = "POINT";
			//s = "LINEAR";
			//s = "ANISOTROPIC";
		default:
			break;
		}
		std::cout << "**(HARDWARE) Sampler Filter = " << s << std::endl;
	}


	// Private
	void Renderer::RenderHardware() const
	{
		//1. CLEAR RTV & DSV
		ColorRGB clearColor = m_ClearColorHardware;
		if (m_IsUniformClearColor) clearColor = m_ClearColorUniform;
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. SET PIPELINE + INVOKE DRAWCALLS (= RENDER)
		//m_pScene->Render(m_pDeviceContext);

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	HRESULT Renderer::InitializeDirectX(IDXGIFactory1*& pDxgiFactory)
	{
		//1. Create Device & Context
		//=====
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result = D3D11CreateDevice(nullptr,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			createDeviceFlags,
			&featureLevel,
			1,
			D3D11_SDK_VERSION,
			&m_pDevice,
			nullptr,
			&m_pDeviceContext);
		if (FAILED(result))
			return result;

		//Create DXGI Factory
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;



		//2. Create Swapchain
		//=====
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Creat Swapchain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;



		//3. Create DepthStencil (DS) and DepthStencilView (DSV)
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		// View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;



		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//=====

		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;



		//5. Bind RTV & DSV to Output Merger Stage
		//=====
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);



		//6. Set Viewport
		//=====
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);



		return result;
	}
#pragma endregion

#pragma region SOFTWARE
	// Public
	void Renderer::CycleShadingMode()
	{
		//TODO: implement function
		bool shadingMode = true;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeSoftware);
		std::string s;
		switch (shadingMode)
		{
			//s = "OBSERVED_AREA";
			//s = "DIFFUSE";
			//s = "SPECULAR";
			//s = "COMBINED";
		default:
			break;
		}
		std::cout << "**(SOFTWARE) Shading Mode = " << s << std::endl;
	}

	void Renderer::ToggleNormalMap()
	{
		//TODO: implement function
		bool isFire = true;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeSoftware);
		std::string s = (isFire) ? "ON" : "OFF";
		std::cout << "**(SOFTWARE) NormalMap " << s << std::endl;
	}

	void Renderer::ToggleDepthBuffer()
	{
		//TODO: implement function
		bool isFire = true;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeSoftware);
		std::string s = (isFire) ? "ON" : "OFF";
		std::cout << "**(SOFTWARE) DepthBuffer Visualization " << s << std::endl;
	}

	void Renderer::ToggleBoundingBox()
	{
		//TODO: implement function
		bool isFire = true;

		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, m_AttributeSoftware);
		std::string s = (isFire) ? "ON" : "OFF";
		std::cout << "**(SOFTWARE) BoundingBox Visualization " << s << std::endl;
	}


	// Private
	void Renderer::RenderSoftware() const
	{
		//TODO: implement function
	}

	void Renderer::InitializeSoftware()
	{
		//Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(m_pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;
	}
#pragma endregion

}
