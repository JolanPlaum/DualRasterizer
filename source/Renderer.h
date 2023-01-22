#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Scene;

	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&)					= delete;
		Renderer(Renderer&&) noexcept				= delete;
		Renderer& operator=(const Renderer&)		= delete;
		Renderer& operator=(Renderer&&) noexcept	= delete;

		void Update(const Timer* pTimer);
		void Render() const;

		//SHARED
		void ToggleRasterizerMode();
		void ToggleRotation();
		void CycleCullMode();
		void ToggleUniformClearColor();
		void TogglePrintFPS();

		//HARDWARE
		void ToggleFireFX();
		void CycleSamplerState();

		//SOFTWARE
		void CycleShadingMode();
		void ToggleNormalMap();
		void ToggleDepthBuffer();
		void ToggleBoundingBox();


	private:
		SDL_Window* m_pWindow{};
		Scene* m_pScene{};

		bool m_IsInitialized{ false };
		bool m_IsUniformClearColor{ false };
		bool m_IsPrintFPS{ true };

		BYTE m_AttributeFPS{ 8 };
		BYTE m_AttributeShared{ 6 };
		BYTE m_AttributeHardware{ 2 };
		BYTE m_AttributeSoftware{ 5 };

		int m_Width{};
		int m_Height{};

		float m_PrintTimer{ 0.f };
		float m_PrintInterval{ 1.f };

		const ColorRGB m_ClearColorUniform{ 0.1f, 0.1f, 0.1f };
		const ColorRGB m_ClearColorHardware{ 0.39f, 0.59f, 0.93f };
		const ColorRGB m_ClearColorSoftware{ 0.39f, 0.39f, 0.39f };

		enum class RasterizerMode
		{
			hardware,
			software,

			//@END
			END
		} m_RasterizerMode{};


		//HARDWARE
		void RenderHardware() const;
		HRESULT InitializeDirectX(IDXGIFactory1*& pDxgiFactory);

		ID3D11Device* m_pDevice{};
		ID3D11DeviceContext* m_pDeviceContext{};

		IDXGISwapChain* m_pSwapChain{};

		ID3D11Texture2D* m_pDepthStencilBuffer{};
		ID3D11DepthStencilView* m_pDepthStencilView{};

		ID3D11Texture2D* m_pRenderTargetBuffer{};
		ID3D11RenderTargetView* m_pRenderTargetView{};


		//SOFTWARE
		void RenderSoftware() const;
		void InitializeSoftware();

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
	};
}
