#pragma once
// Includes
#include "Material.h"

namespace dae
{
	// Class Forward Declarations
	
	// Class Declaration
	class MaterialTransparency final : public Material
	{
	public:
		// Constructors and Destructor
		explicit MaterialTransparency(ID3D11Device* pDevice, const std::wstring& assetFile);
		~MaterialTransparency();
		
		// Copy and Move semantics
		MaterialTransparency(const MaterialTransparency& other)					= delete;
		MaterialTransparency& operator=(const MaterialTransparency& other)		= delete;
		MaterialTransparency(MaterialTransparency&& other) noexcept				= delete;
		MaterialTransparency& operator=(MaterialTransparency&& other) noexcept	= delete;
	
		//---------------------------
		// Public Member Functions
		//---------------------------
		virtual void SetTexture(Texture* pTexture, const std::string& name) override;
	
	
	private:
		// Member variables
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};

		Texture* m_pDiffuseTexture{};
	
		//---------------------------
		// Private Member Functions
		//---------------------------
	
	};
}
