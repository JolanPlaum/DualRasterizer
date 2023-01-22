#pragma once
// Includes

namespace dae
{
	// Class Forward Declarations
	class Camera;
	class Mesh;
	
	// Class Declaration
	class Scene final
	{
	public:
		// Constructors and Destructor
		explicit Scene(ID3D11Device* pDevice, SDL_Surface* pBackBuffer);
		~Scene();
		
		// Copy and Move semantics
		Scene(const Scene& other)					= delete;
		Scene& operator=(const Scene& other)		= delete;
		Scene(Scene&& other) noexcept				= delete;
		Scene& operator=(Scene&& other) noexcept	= delete;
	
		//---------------------------
		// Public Member Functions
		//---------------------------
		void Update(const Timer* pTimer);
		void RenderHardware(ID3D11DeviceContext* pDeviceContext) const;
		void RenderSoftware(SDL_Surface* pBackBuffer) const;

		//SHARED
		bool ToggleRotation();

		//HARDWARE
		bool ToggleFireFX();
		std::string CycleSamplerState();

		//SOFTWARE
		std::string CycleShadingMode();
		bool ToggleNormalMap();
		bool ToggleDepthBuffer();
		bool ToggleBoundingBox();
		
	
	private:
		// Member variables
		Camera* m_pCamera{};
		Mesh* m_pVehicle{};
		Mesh* m_pFireFX{};

		bool m_IsRotating{ true };
		bool m_IsShowFireFX{ true };
	
		//---------------------------
		// Private Member Functions
		//---------------------------
		void InitVehicle(ID3D11Device* pDevice, SDL_Surface* pBackBuffer);
		void InitFireFX(ID3D11Device* pDevice, SDL_Surface* pBackBuffer);
	
	};
}
