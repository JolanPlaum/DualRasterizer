#pragma once
// Includes
#include "Material.h"

namespace dae
{
	// Class Forward Declarations
	
	// Class Declaration
	class MaterialShading final : public Material
	{
	public:
		// Constructors and Destructor
		explicit MaterialShading(ID3D11Device* pDevice, const std::wstring& assetFile);
		~MaterialShading();
		
		// Copy and Move semantics
		MaterialShading(const MaterialShading& other)					= delete;
		MaterialShading& operator=(const MaterialShading& other)		= delete;
		MaterialShading(MaterialShading&& other) noexcept				= delete;
		MaterialShading& operator=(MaterialShading&& other) noexcept	= delete;
	
		//---------------------------
		// Public Member Functions
		//---------------------------
		virtual void SetMatrix(Matrix& matrix, const std::string& name) override;
		virtual void SetTexture(Texture* pTexture, const std::string& name) override;

		//SOFTWARE
		virtual void VertexShading(const std::vector<Vertex>& vertices_in, std::vector<Vertex_Out>& vertices_out) override;
		virtual ColorRGB PixelShading(const Vertex_Out& v) override;

		std::string CycleShading();
		bool ToggleNormalMap();

	
	private:
		// Member variables
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
		ID3DX11EffectMatrixVariable* m_pMatInvViewVariable{};

		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pGlossMapVariable{};

		Matrix m_WorldMat{};
		Matrix m_InvViewMat{};

		Texture* m_pDiffuseTexture{};
		Texture* m_pNormalTexture{};
		Texture* m_pSpecularTexture{};
		Texture* m_pGlossTexture{};

		enum class ShadingMode
		{
			ObservedArea, //Lambert Cosine Law
			Diffuse, //Including observed area
			Specular, //Including observed area
			Combined, //ObservedArea * (Diffuse + Specular)

			//@END
			END
		} m_ShadingMode{ ShadingMode::Combined };
		bool m_IsNormalMap{ true };
	
		//---------------------------
		// Private Member Functions
		//---------------------------		
		void SetWorldMatrix(Matrix& matrix);
		void SetInverseViewMatrix(Matrix& matrix);

		void SetDiffuse(Texture* pTexture);
		void SetNormal(Texture* pTexture);
		void SetSpecular(Texture* pTexture);
		void SetGlossiness(Texture* pTexture);
	
	};
}
