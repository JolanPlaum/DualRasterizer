//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "pch.h"
#include "Material.h"
#include <cassert>

using namespace dae;


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
Material::Material(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	//Load Effect
	m_pEffect = LoadEffect(pDevice, assetFile);


	//Load Techniques
	m_pTechniquePoint = m_pEffect->GetTechniqueByName("DefaultTechnique");
	if (!m_pTechniquePoint->IsValid())
		std::wcout << L"DefaultTechnique not valid\n";

	m_pTechniqueLinear = m_pEffect->GetTechniqueByName("LinearTechnique");
	if (!m_pTechniqueLinear->IsValid())
		std::wcout << L"LinearTechnique not valid\n";

	m_pTechniqueAnisotropic = m_pEffect->GetTechniqueByName("AnisotropicTechnique");
	if (!m_pTechniqueAnisotropic->IsValid())
		std::wcout << L"AnisotropicTechnique not valid\n";

	m_pTechnique = m_pTechniquePoint;


	//Load Matrix
	m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
	if (!m_pMatWorldViewProjVariable->IsValid())
		std::wcout << L"Matrix Variable gWorldViewProj not valid\n";


	//Create Vertex Layout
	static constexpr uint32_t numElements{ 4 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = 0;
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "NORMAL";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = 12;
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TANGENT";
	vertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[2].AlignedByteOffset = 24;
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "TEXCOORD";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[3].AlignedByteOffset = 36;
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	//Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	const HRESULT result = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout);

	if (FAILED(result))
		assert(false);
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
Material::~Material()
{
	if (m_pMatWorldViewProjVariable) m_pMatWorldViewProjVariable->Release();

	if (m_pTechniquePoint) m_pTechniquePoint->Release();
	if (m_pTechniqueLinear) m_pTechniqueLinear->Release();
	if (m_pTechniqueAnisotropic) m_pTechniqueAnisotropic->Release();

	if (m_pInputLayout) m_pInputLayout->Release();
	if (m_pEffect) m_pEffect->Release();
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
ID3DX11Effect* Material::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob{ nullptr };
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i = 0; i < pErrorBlob->GetBufferSize(); i++)
			{
				ss << pErrors[i];
			}

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << std::endl;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << std::endl;
			return nullptr;
		}
	}

	return pEffect;
}

std::string Material::CycleTechnique()
{
	m_TechniqueType = TechniqueType(((int)m_TechniqueType + 1) % (int)TechniqueType::END);

	switch (m_TechniqueType)
	{
	case TechniqueType::Point:
		if (m_pTechniquePoint)
		{
			m_pTechnique = m_pTechniquePoint;
			return "POINT";
		} break;
	case TechniqueType::Linear:
		if (m_pTechniqueLinear)
		{
			m_pTechnique = m_pTechniqueLinear;
			return "LINEAR";
		} break;
	case TechniqueType::Anisotropic:
		if (m_pTechniqueAnisotropic)
		{
			m_pTechnique = m_pTechniqueAnisotropic;
			return "ANISOTROPIC";
		} break;
	}

	return "";
}

void Material::SetMatrix(Matrix& matrix, const std::string& name)
{
	if (name == "WorldViewProj")
	{
		m_WorldViewProjMat = matrix;

		if (m_pMatWorldViewProjVariable)
			m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<float*>(&matrix));
		else
			std::wcout << L"SetMatrix m_pMatWorldViewProjVariable failed\n";
	}
}


//-----------------------------------------------------------------
// Protected Member Functions
//-----------------------------------------------------------------

