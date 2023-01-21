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
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}
	}

	Renderer::~Renderer()
	{

	}

	void Renderer::Update(const Timer* pTimer)
	{

	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;
	}

#pragma region SHARED
	// Public
	void Renderer::ToggleRasterizerMode()
	{
	}
	void Renderer::ToggleRotation()
	{
	}
	void Renderer::CycleCullMode()
	{
	}
	void Renderer::ToggleUniformClearColor()
	{
	}
	void Renderer::TogglePrintFPS()
	{
	}

	// Private
#pragma endregion

#pragma region HARDWARE
	// Public
	void Renderer::ToggleFireFX()
	{
	}
	void Renderer::CycleSamplerState()
	{
	}

	// Private
	void Renderer::RenderHardware()
	{
	}
	HRESULT Renderer::InitializeDirectX()
	{
		return S_FALSE;
	}
#pragma endregion

#pragma region SOFTWARE
	// Public
	void Renderer::CycleShadingMode()
	{
	}
	void Renderer::ToggleNormalMap()
	{
	}
	void Renderer::ToggleDepthBuffer()
	{
	}
	void Renderer::ToggleBoundingBox()
	{
	}

	// Private
	void Renderer::RenderSoftware()
	{
	}
#pragma endregion

}
