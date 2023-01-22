#pragma once
// Includes
#include "DataTypes.h"

namespace dae
{
	// Class Forward Declarations
	class Texture;
	
	// Class Declaration
	class Material
	{
	public:
		// Constructors and Destructor
		explicit Material(ID3D11Device* pDevice, const std::wstring& assetFile);
		virtual ~Material();
		
		// Copy and Move semantics
		Material(const Material& other)					= delete;
		Material& operator=(const Material& other)		= delete;
		Material(Material&& other) noexcept				= delete;
		Material& operator=(Material&& other) noexcept	= delete;
	
		//---------------------------
		// Public Member Functions
		//---------------------------
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		virtual void SetMatrix(const Matrix& matrix, const std::string& name);
		virtual void SetTexture(Texture* pTexture, const std::string& name) {};

		ID3DX11Effect* GetEffect() const { return m_pEffect; }
		ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }
		ID3D11InputLayout* GetInputLayout() const { return m_pInputLayout; }
		
		//HARDWARE
		std::string CycleTechnique();

		//SOFTWARE
		virtual std::vector<Vertex_Out>* VertexShading(const std::vector<Vertex>& vertices_in) {};
		virtual ColorRGB PixelShading(const Vertex_Out& v) {};

	
	protected:
		// Member variables
		ID3DX11Effect* m_pEffect{};
		ID3D11InputLayout* m_pInputLayout{};

		ID3DX11EffectTechnique* m_pTechnique{};
		ID3DX11EffectTechnique* m_pTechniquePoint{};
		ID3DX11EffectTechnique* m_pTechniqueLinear{};
		ID3DX11EffectTechnique* m_pTechniqueAnisotropic{};

		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		Matrix m_WorldViewProjMat{};

		enum class TechniqueType
		{
			Point,
			Linear,
			Anisotropic,

			//@END
			END
		} m_TechniqueType{};
	
		//---------------------------
		// Protected Member Functions
		//---------------------------
		uint32_t CreateVertexLayout(D3D11_INPUT_ELEMENT_DESC*& pVertexDesc);
	
	};
}
