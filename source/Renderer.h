#pragma once

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
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

		bool m_IsInitialized{ false };
		bool m_IsUniformClearColor{ false };
		bool m_IsPrintFPS{ false };

		int m_Width{};
		int m_Height{};

		float m_PrintTimer{ 0.f };

		ColorRGB m_ColUniform{ 0.1f, 0.1f, 0.1f };
		ColorRGB m_ColHardware{ 0.39f, 0.59f, 0.93f };
		ColorRGB m_ColSoftware{ 0.39f, 0.39f, 0.39f };

		enum RasterizerMode
		{
			hardware,
			software,

			//@END
			END
		} m_RasterizerMode;


		//HARDWARE
		void RenderHardware();
		HRESULT InitializeDirectX();

		//SOFTWARE
		void RenderSoftware();
	};
}
