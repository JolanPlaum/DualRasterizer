//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "pch.h"
#include "MaterialTransparency.h"
#include "Texture.h"

using namespace dae;


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
MaterialTransparency::MaterialTransparency(ID3D11Device* pDevice, const std::wstring& assetFile)
	: Material(pDevice, assetFile)
{
	//Load Textures
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::wcout << L"Shader Resource gDiffuseMap Variable not valid\n";
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
MaterialTransparency::~MaterialTransparency()
{
	delete m_pDiffuseTexture;

	if (m_pDiffuseMapVariable) m_pDiffuseMapVariable->Release();
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
void MaterialTransparency::SetTexture(Texture* pTexture, const std::string& name)
{
	Material::SetTexture(pTexture, name);

	if (name == "Diffuse")
	{
		if (pTexture == nullptr)
			std::wcout << L"SetDiffuse failed: nullptr given\n";
		else if (m_pDiffuseMapVariable)
		{
			m_pDiffuseTexture = pTexture;
			m_pDiffuseMapVariable->SetResource(pTexture->GetResourceView());
		}
	}
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

