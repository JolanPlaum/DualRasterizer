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

	//Create Vertex Layout
	D3D11_INPUT_ELEMENT_DESC* pVertexDesc{};
	uint32_t numElements = CreateVertexLayout(pVertexDesc);

	//Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

	const HRESULT result = pDevice->CreateInputLayout(
		pVertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout);

	if (FAILED(result))
		assert(false);

	delete pVertexDesc;


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

void Material::SetMatrix(const Matrix& matrix, const std::string& name)
{
	if (name == "WorldViewProj")
	{
		m_WorldViewProjMat = matrix;

		if (m_pMatWorldViewProjVariable)
			m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<float*>(&m_WorldViewProjMat));
		else
			std::wcout << L"SetMatrix m_pMatWorldViewProjVariable failed\n";
	}
}


//-----------------------------------------------------------------
// Protected Member Functions
//-----------------------------------------------------------------
uint32_t Material::CreateVertexLayout(D3D11_INPUT_ELEMENT_DESC*& pVertexDesc)
{
	static constexpr uint32_t numElements{ 4 };
	pVertexDesc = new D3D11_INPUT_ELEMENT_DESC[numElements]{};

	pVertexDesc[0].SemanticName = "POSITION";
	pVertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	pVertexDesc[0].AlignedByteOffset = 0;
	pVertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	pVertexDesc[1].SemanticName = "TEXCOORD";
	pVertexDesc[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	pVertexDesc[1].AlignedByteOffset = 12;
	pVertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	pVertexDesc[2].SemanticName = "NORMAL";
	pVertexDesc[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	pVertexDesc[2].AlignedByteOffset = 20;
	pVertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	pVertexDesc[3].SemanticName = "TANGENT";
	pVertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	pVertexDesc[3].AlignedByteOffset = 32;
	pVertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	return numElements;
}

