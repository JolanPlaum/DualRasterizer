//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "pch.h"
#include "Texture.h"
#include <cassert>

using namespace dae;


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
Texture::Texture(ID3D11Device* pDevice, const std::string& path)
{
	//Load SDL_Surface using IMG_LOAD
	SDL_Surface* pSurface = IMG_Load(path.c_str());
	assert(pSurface && "Image failed to load!");
	m_pSurface = pSurface;
	m_pSurfacePixels = (uint32_t*)m_pSurface->pixels;


	//Create Resource
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = m_pSurface->w;
	desc.Height = m_pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = m_pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(m_pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(m_pSurface->h * m_pSurface->pitch);

	HRESULT result = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);
	if (FAILED(result))
		return;


	//Create Resource View
	D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
	SRVDesc.Format = format;
	SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	SRVDesc.Texture1D.MipLevels = 1;

	result = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pSRV);
	if (FAILED(result))
		return;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
Texture::~Texture()
{
	if (m_pSurface) SDL_FreeSurface(m_pSurface);

	if (m_pSRV) m_pSRV->Release();
	if (m_pResource) m_pResource->Release();
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
ColorRGB Texture::Sample(const Vector2& uv) const
{
	int width = m_pSurface->w;
	int height = m_pSurface->h;

	int px = int(uv.x * width);
	int py = int(uv.y * height);

	Uint8 r, g, b;
	SDL_GetRGB(m_pSurfacePixels[px + (py * width)], m_pSurface->format, &r, &g, &b);

	return { r / 255.f, g / 255.f, b / 255.f };
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------

