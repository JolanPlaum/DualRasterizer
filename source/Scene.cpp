//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "pch.h"
#include "Scene.h"
#include "Camera.h"
#include "Mesh.h"
#include "MaterialShading.h"
#include "MaterialTransparency.h"
#include "Texture.h"

using namespace dae;


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
Scene::Scene(ID3D11Device* pDevice, SDL_Surface* pBackBuffer)
{
	m_pCamera = new Camera({ 0.f,0.f,0.f }, 45.f, pBackBuffer->w / (float)pBackBuffer->h);

	InitVehicle(pDevice, pBackBuffer);
	InitFireFX(pDevice, pBackBuffer);
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
Scene::~Scene()
{
	delete m_pCamera;

	delete m_pVehicle;
	delete m_pFireFX;
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
void Scene::Update(const Timer* pTimer)
{
	//Update camera first since we need to retrieve data from it
	m_pCamera->Update(pTimer);

	//Update rotation
	if (m_IsRotating)
	{
		m_pVehicle->Rotate({ 0.f, pTimer->GetElapsed() * PI_DIV_2, 0.f });
		m_pFireFX->Rotate({ 0.f, pTimer->GetElapsed() * PI_DIV_2, 0.f });
	}

	//Calculate the WorldViewProjection matrix
	Matrix viewProj = m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix();
	Matrix invView = m_pCamera->GetInverseViewMatrix();

	//Update Vehicle
	Matrix world = m_pVehicle->GetWorldMatrix();
	m_pVehicle->GetMaterial()->SetMatrix(world * viewProj, "WorldViewProj");
	m_pVehicle->GetMaterial()->SetMatrix(world, "World");
	m_pVehicle->GetMaterial()->SetMatrix(invView, "InvView");

	//Update FireFX
	world = m_pFireFX->GetWorldMatrix();
	m_pFireFX->GetMaterial()->SetMatrix(world * viewProj, "WorldViewProj");
}

void Scene::RenderHardware(ID3D11DeviceContext* pDeviceContext) const
{
	m_pVehicle->RenderHardware(pDeviceContext);

	if (m_IsShowFireFX)
		m_pFireFX->RenderHardware(pDeviceContext);
}

void Scene::RenderSoftware(SDL_Surface* pBackBuffer) const
{
	m_pVehicle->RenderSoftware(pBackBuffer);
}

bool Scene::ToggleRotation()
{
	return m_IsRotating = !m_IsRotating;
}

bool Scene::ToggleFireFX()
{
	return m_IsShowFireFX = !m_IsShowFireFX;
}

std::string Scene::CycleSamplerState()
{
	m_pVehicle->GetMaterial()->CycleTechnique();
	return m_pFireFX->GetMaterial()->CycleTechnique();
}

std::string Scene::CycleShadingMode()
{
	MaterialShading* pMat = dynamic_cast<MaterialShading*>(m_pVehicle->GetMaterial());
	if (pMat) return pMat->CycleShading();

	return "";
}

bool Scene::ToggleNormalMap()
{
	MaterialShading* pMat = dynamic_cast<MaterialShading*>(m_pVehicle->GetMaterial());
	if (pMat) return pMat->ToggleNormalMap();

	return false;
}

bool Scene::ToggleDepthBuffer()
{
	return m_pVehicle->ToggleDepthBuffer();
}

bool Scene::ToggleBoundingBox()
{
	return m_pVehicle->ToggleBoundingBox();
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------
void Scene::InitVehicle(ID3D11Device* pDevice, SDL_Surface* pBackBuffer)
{
	//1. Create new Material
	MaterialShading* pVehicleMaterial = new MaterialShading(pDevice, L"Resources/Vehicle.fx");

	//3. Instantiate Mesh
	m_pVehicle = new Mesh(pDevice, pBackBuffer, "Resources/vehicle.obj", pVehicleMaterial);
	m_pVehicle->SetPosition(0.f, 0.f, 50.f);

	//2. Set Textures
	//pVehicleMaterial->SetTexture(new Texture(pDevice, "Resources/vehicle_diffuse.png"), "Diffuse");
	//pVehicleMaterial->SetTexture(new Texture(pDevice, "Resources/vehicle_normal.png"), "Normal");
	//pVehicleMaterial->SetTexture(new Texture(pDevice, "Resources/vehicle_specular.png"), "Specular");
	//pVehicleMaterial->SetTexture(new Texture(pDevice, "Resources/vehicle_gloss.png"), "Gloss");


	pVehicleMaterial->SetDiffuseMap(	new Texture(pDevice, "Resources/vehicle_diffuse.png"));
	pVehicleMaterial->SetNormalMap(		new Texture(pDevice, "Resources/vehicle_normal.png"));
	pVehicleMaterial->SetSpecularMap(	new Texture(pDevice, "Resources/vehicle_specular.png"));
	pVehicleMaterial->SetGlossinessMap(	new Texture(pDevice, "Resources/vehicle_gloss.png"));
}

void Scene::InitFireFX(ID3D11Device* pDevice, SDL_Surface* pBackBuffer)
{
	//1. Create new Material
	MaterialTransparency* pVehicleMaterial = new MaterialTransparency(pDevice, L"Resources/Fire.fx");

	//2. Set Textures
	pVehicleMaterial->SetTexture(new Texture(pDevice, "Resources/fireFX_diffuse.png"), "Diffuse");

	//3. Instantiate Mesh
	m_pFireFX = new Mesh(pDevice, pBackBuffer, "Resources/fireFX.obj", pVehicleMaterial);
	m_pFireFX->SetPosition(0.f, 0.f, 50.f);
}

