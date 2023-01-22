//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "pch.h"
#include "MaterialShading.h"
#include "Texture.h"
#include "Utils.h"

using namespace dae;


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
MaterialShading::MaterialShading(ID3D11Device* pDevice, const std::wstring& assetFile)
	: Material(pDevice, assetFile)
{
	//Load Matrices
	m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
	if (!m_pMatWorldVariable->IsValid())
		std::wcout << L"Matrix Variable gWorld not valid\n";

	m_pMatInvViewVariable = m_pEffect->GetVariableByName("gInvView")->AsMatrix();
	if (!m_pMatInvViewVariable->IsValid())
		std::wcout << L"Matrix Variable gInvView not valid\n";


	//Load Textures
	m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
	if (!m_pDiffuseMapVariable->IsValid())
		std::wcout << L"Shader Resource gDiffuseMap Variable not valid\n";

	m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
	if (!m_pNormalMapVariable->IsValid())
		std::wcout << L"Shader Resource gNormalMap Variable not valid\n";

	m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
	if (!m_pSpecularMapVariable->IsValid())
		std::wcout << L"Shader Resource gSpecularMap Variable not valid\n";

	m_pGlossMapVariable = m_pEffect->GetVariableByName("gGlossMap")->AsShaderResource();
	if (!m_pGlossMapVariable->IsValid())
		std::wcout << L"Shader Resource gGlossMap Variable not valid\n";
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
MaterialShading::~MaterialShading()
{
	delete m_pDiffuseTexture;
	delete m_pNormalTexture;
	delete m_pSpecularTexture;
	delete m_pGlossTexture;

	if (m_pGlossMapVariable) m_pGlossMapVariable->Release();
	if (m_pSpecularMapVariable) m_pSpecularMapVariable->Release();
	if (m_pNormalMapVariable) m_pNormalMapVariable->Release();
	if (m_pDiffuseMapVariable) m_pDiffuseMapVariable->Release();

	if (m_pMatInvViewVariable) m_pMatInvViewVariable->Release();
	if (m_pMatWorldVariable) m_pMatWorldVariable->Release();
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
void MaterialShading::SetMatrix(Matrix& matrix, const std::string& name)
{
	Material::SetMatrix(matrix, name);

	if (name == "World")
	{
		SetWorldMatrix(matrix);
	}
	else if (name == "InvView")
	{
		SetInverseViewMatrix(matrix);
	}
}

void MaterialShading::SetTexture(Texture* pTexture, const std::string& name)
{
	Material::SetTexture(pTexture, name);

	if (name == "Diffuse")
	{
		SetDiffuse(pTexture);
	}
	else if (name == "Normal")
	{
		SetNormal(pTexture);
	}
	else if (name == "Specular")
	{
		SetSpecular(pTexture);
	}
	else if (name == "Gloss")
	{
		SetGlossiness(pTexture);
	}
}

void MaterialShading::VertexShading(const std::vector<Vertex>& vertices_in, std::vector<Vertex_Out>& vertices_out)
{
	vertices_out.clear();
	vertices_out.reserve(vertices_in.size());

	for (int i{}; i < vertices_in.size(); ++i)
	{
		//Create temporary variable
		Vertex_Out v{};

		//Position calculations
		v.position = m_WorldViewProjMat.TransformPoint({ vertices_in[i].position, 1.f });

		v.position.x /= v.position.w;
		v.position.y /= v.position.w;
		v.position.z /= v.position.w;

		//Set other variables
		v.uv = vertices_in[i].uv;
		v.normal = m_WorldMat.TransformVector(vertices_in[i].normal);
		v.tangent = m_WorldMat.TransformVector(vertices_in[i].tangent);
		v.worldPosition = Vector3(m_WorldMat.TransformPoint({ vertices_in[i].position, 1.f }));

		//Add the new temporary variable to the list
		vertices_out.emplace_back(v);
	}
}

ColorRGB MaterialShading::PixelShading(const Vertex_Out& v)
{
	//Pre defined variables
	Vector3 lightDirection{ .577f, -.577f, .577f };
	float lightIntensity{ 7.f };
	float shininess{ 25.f };
	ColorRGB finalColor{ 0.025f, 0.025f, 0.025f };

	Vector3 normal{ v.normal };
	Vector3 viewDirection = (v.worldPosition - m_InvViewMat[3].GetXYZ()).Normalized();

	//Normal map
	if (m_IsNormalMap)
	{
		Vector3 binormal = Vector3::Cross(v.normal, v.tangent);
		Matrix tangentSpaceAxis{ v.tangent, binormal, v.normal, Vector3::Zero };

		ColorRGB sampledColor = m_pNormalTexture->Sample(v.uv);
		sampledColor = (2.f * sampledColor) - ColorRGB{ 1.f, 1.f, 1.f };

		normal = tangentSpaceAxis.TransformVector(sampledColor.r, sampledColor.g, sampledColor.b);
	}

	//Observed area (lambert cosine law)
	float dotProduct = normal * -lightDirection;
	if (dotProduct >= 0.f)
	{
		switch (m_ShadingMode)
		{
		case ShadingMode::ObservedArea:
			finalColor += { dotProduct, dotProduct, dotProduct };
			break;

		case ShadingMode::Diffuse:
			finalColor += BRDF::Lambert(lightIntensity, m_pDiffuseTexture->Sample(v.uv)) * dotProduct;
			break;

		case ShadingMode::Specular:
			finalColor += BRDF::Phong(m_pSpecularTexture->Sample(v.uv), shininess * m_pGlossTexture->Sample(v.uv).r, -lightDirection, viewDirection, normal) * dotProduct;
			break;

		case ShadingMode::Combined:
			finalColor += BRDF::Lambert(lightIntensity, m_pDiffuseTexture->Sample(v.uv)) * dotProduct;
			finalColor += BRDF::Phong(m_pSpecularTexture->Sample(v.uv), shininess * m_pGlossTexture->Sample(v.uv).r, -lightDirection, viewDirection, normal) * dotProduct;
			break;
		}
	}

	return finalColor;
}

std::string MaterialShading::CycleShading()
{
	m_ShadingMode = ShadingMode(((int)m_ShadingMode + 1) % (int)ShadingMode::END);

	switch (m_ShadingMode)
	{
	case ShadingMode::ObservedArea:
		return "OBSERVED_AREA";

	case ShadingMode::Diffuse:
		return "DIFFUSE";

	case ShadingMode::Specular:
		return "SPECULAR";

	case ShadingMode::Combined:
		return "COMBINED";
	}

	return "";
}

bool MaterialShading::ToggleNormalMap()
{
	return m_IsNormalMap = !m_IsNormalMap;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------
void MaterialShading::SetWorldMatrix(Matrix& matrix)
{
	m_WorldMat = matrix;

	if (m_pMatWorldVariable)
		m_pMatWorldVariable->SetMatrix(reinterpret_cast<float*>(&matrix));
	else
		std::wcout << L"SetMatrix m_pMatWorldVariable failed\n";
}

void MaterialShading::SetInverseViewMatrix(Matrix& matrix)
{
	m_InvViewMat = matrix;

	if (m_pMatInvViewVariable)
		m_pMatInvViewVariable->SetMatrix(reinterpret_cast<float*>(&matrix));
	else
		std::wcout << L"SetMatrix m_pMatInvViewVariable failed\n";
}

void MaterialShading::SetDiffuse(Texture* pTexture)
{
	if (pTexture == nullptr)
		std::wcout << L"SetDiffuse failed: nullptr given\n";
	else if (m_pDiffuseMapVariable)
	{
		m_pDiffuseTexture = pTexture;
		m_pDiffuseMapVariable->SetResource(pTexture->GetResourceView());
	}
}

void MaterialShading::SetNormal(Texture* pTexture)
{
	if (pTexture == nullptr)
		std::wcout << L"SetNormal failed: nullptr given\n";
	else if (m_pNormalMapVariable)
	{
		m_pNormalTexture = pTexture;
		m_pNormalMapVariable->SetResource(pTexture->GetResourceView());
	}
}

void MaterialShading::SetSpecular(Texture* pTexture)
{
	if (pTexture == nullptr)
		std::wcout << L"SetSpecular failed: nullptr given\n";
	else if (m_pSpecularMapVariable)
	{
		m_pSpecularTexture = pTexture;
		m_pSpecularMapVariable->SetResource(pTexture->GetResourceView());
	}
}

void MaterialShading::SetGlossiness(Texture* pTexture)
{
	if (pTexture == nullptr)
		std::wcout << L"SetGlossiness failed: nullptr given\n";
	else if (m_pGlossMapVariable)
	{
		m_pGlossTexture = pTexture;
		m_pGlossMapVariable->SetResource(pTexture->GetResourceView());
	}
}

