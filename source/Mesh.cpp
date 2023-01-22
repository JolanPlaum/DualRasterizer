//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "pch.h"
#include "Mesh.h"
#include "Utils.h"
#include "Material.h"
#include "Texture.h"

using namespace dae;


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
Mesh::Mesh(ID3D11Device* pDevice, SDL_Surface* pBackBuffer, const std::string& filename, Material* pMaterial)
	: m_pMaterial(pMaterial)
{
	//Create Software Buffers
	m_pBackBufferPixels = (uint32_t*)pBackBuffer->pixels;
	m_pDepthBufferPixels = new float[pBackBuffer->w * pBackBuffer->h];

	//Get Vertices and Indices
	Utils::ParseOBJ(filename, m_Vertices, m_Indices);

	//Create Vertex Buffer
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(m_Vertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = m_Vertices.data();

	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		return;

	//Create Index Buffer
	m_NumIndices = static_cast<uint32_t>(m_Indices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	initData.pSysMem = m_Indices.data();

	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		return;
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
Mesh::~Mesh()
{
	delete[] m_pDepthBufferPixels;

	m_pIndexBuffer->Release();
	m_pVertexBuffer->Release();

	delete m_pMaterial;
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
void Mesh::RenderHardware(ID3D11DeviceContext* pDeviceContext) const
{
	//1. Set Primitive Topology
	pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pMaterial->GetInputLayout());

	//3. Set Vertex Buffer
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set Index Buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pMaterial->GetTechnique()->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pMaterial->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}

void Mesh::RenderSoftware(SDL_Surface* pBackBuffer) const
{
	//1. Reset Detph Buffer
	std::fill_n(m_pDepthBufferPixels, pBackBuffer->w * pBackBuffer->h, FLT_MAX);

	//2. Vertex Shading
	std::vector<Vertex_Out> verticesOut;
	m_pMaterial->VertexShading(m_Vertices, verticesOut);

	//3. Render Triangles
	for (int i{}; i < m_Indices.size() - 2; i += 3)
	{
		RenderTriangle(pBackBuffer,
			verticesOut[m_Indices[i]],
			verticesOut[m_Indices[i + 1]],
			verticesOut[m_Indices[i + 2]]
		);
	}
}

bool Mesh::ToggleDepthBuffer()
{
	return m_IsShowDepthBuffer = !m_IsShowDepthBuffer;
}

bool Mesh::ToggleBoundingBox()
{
	return m_IsShowBoundingBox = !m_IsShowBoundingBox;
}

void Mesh::Translate(const Vector3& translation)
{
	m_Position += translation;
}

void Mesh::Rotate(const Vector3& rotation)
{
	m_Rotation += rotation;
}

void Mesh::Scale(float scale)
{
	m_Scale *= scale;
}

void Mesh::SetPosition(float x, float y, float z)
{
	m_Position = Vector3{ x, y, z };
}

void Mesh::SetRotation(float pitch, float yaw, float roll)
{
	m_Rotation = Vector3{ pitch, yaw, roll };
}

void Mesh::SetScale(const Vector3& scale)
{
	m_Scale = scale;
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------
void Mesh::RenderTriangle(SDL_Surface* pBackBuffer, const Vertex_Out& _v0, const Vertex_Out& _v1, const Vertex_Out& _v2) const
{
	// Variables
	int width{ pBackBuffer->w };
	int height{ pBackBuffer->h };

	//1. Frustum Culling
	if (FrustumCulling(_v0.position) || FrustumCulling(_v1.position) || FrustumCulling(_v2.position)) return;

	//2. NDC to Raster Space
	Vertex_Out v0{ NDCToRaster(_v0, width, height) };
	Vertex_Out v1{ NDCToRaster(_v1, width, height) };
	Vertex_Out v2{ NDCToRaster(_v2, width, height) };

	Vector2 edge0 = v2.position.GetXY() - v1.position.GetXY();
	Vector2 edge1 = v0.position.GetXY() - v2.position.GetXY();
	Vector2 edge2 = v1.position.GetXY() - v0.position.GetXY();

	//3. Calculate Signed Area
	float area{ Vector2::Cross(edge0, edge1) };
	if (area < 0.001f) return;

	//4. Calculate Bounding Box
	int left{ (int)std::min(v0.position.x, std::min(v1.position.x, v2.position.x)) };
	int top{ (int)std::min(v0.position.y, std::min(v1.position.y, v2.position.y)) };
	int right{ (int)ceilf(std::max(v0.position.x, std::max(v1.position.x, v2.position.x))) };
	int bottom{ (int)ceilf(std::max(v0.position.y, std::max(v1.position.y, v2.position.y))) };

	if (left < 0) left = 0;
	if (top < 0) top = 0;
	if (right >= width) right = width - 1;
	if (bottom >= height) bottom = height - 1;

	//5. Render Pixels
	for (int px{ left }; px < right; ++px)
	{
		for (int py{ top }; py < bottom; ++py)
		{
			if (m_IsShowBoundingBox)
			{
				m_pBackBufferPixels[px + (py * width)] = SDL_MapRGB(pBackBuffer->format,
					static_cast<uint8_t>(255),
					static_cast<uint8_t>(255),
					static_cast<uint8_t>(255));

				continue;
			}

			float w0, w1, w2;
			Vector2 pixel{ (float)px, (float)py };

			//Check if pixel is inside triangle
			Vector2 pixelToSide = pixel - v0.position.GetXY();
			if ((w2 = Vector2::Cross(edge2, pixelToSide) / area) < 0.f) continue;

			pixelToSide = pixel - v1.position.GetXY();
			if ((w0 = Vector2::Cross(edge0, pixelToSide) / area) < 0.f) continue;

			pixelToSide = pixel - v2.position.GetXY();
			if ((w1 = Vector2::Cross(edge1, pixelToSide) / area) < 0.f) continue;

			//Calculate depth buffer
			float depthBuffer = 1.f / ((w0 / v0.position.z) + (w1 / v1.position.z) + (w2 / v2.position.z));

			if (depthBuffer < 0 || depthBuffer > 1) continue;

			//Depth Test
			if (depthBuffer < m_pDepthBufferPixels[px + (py * width)])
			{
				//Depth Write
				m_pDepthBufferPixels[px + (py * width)] = depthBuffer;

				ColorRGB finalColor{};
				if (m_IsShowDepthBuffer)
				{
					//Remap the depthbuffer to avoid having everything in white
					Remap(depthBuffer, 0.995f, 1.f);

					//Clamp the depthbuffer to prevent negative values
					depthBuffer = Clamp(depthBuffer, 0.f, 1.f);

					finalColor = { depthBuffer,depthBuffer,depthBuffer };
				}
				else
				{
					Vector3 worldPosition = (w0 * v0.worldPosition + w1 * v1.worldPosition + w2 * v2.worldPosition);

					//Depth correction
					w0 /= v0.position.w;
					w1 /= v1.position.w;
					w2 /= v2.position.w;

					//Calculate depth
					float depth = 1.f / (w0 + w1 + w2);

					//Update Color in Buffer
					Vertex_Out temp{};
					temp.position.x = (float)px;
					temp.position.y = (float)py;
					temp.uv = (w0 * v0.uv + w1 * v1.uv + w2 * v2.uv) * depth;
					temp.normal = ((w0 * v0.normal + w1 * v1.normal + w2 * v2.normal) * depth).Normalized();
					temp.tangent = ((w0 * v0.tangent + w1 * v1.tangent + w2 * v2.tangent) * depth).Normalized();
					temp.worldPosition = worldPosition;

					finalColor = m_pMaterial->PixelShading(temp);
				}

				//Update Color in Buffer
				finalColor.MaxToOne();

				m_pBackBufferPixels[px + (py * width)] = SDL_MapRGB(pBackBuffer->format,
					static_cast<uint8_t>(finalColor.r * 255),
					static_cast<uint8_t>(finalColor.g * 255),
					static_cast<uint8_t>(finalColor.b * 255));
			}
		}
	}
}

Vertex_Out Mesh::NDCToRaster(const Vertex_Out& v, int width, int heigth) const
{
	Vertex_Out temp{ v };
	temp.position.x = ((1.f + v.position.x) / 2.f) * width;
	temp.position.y = ((1.f - v.position.y) / 2.f) * heigth;
	return temp;
}

bool Mesh::FrustumCulling(const Vector4& v) const
{
	if (v.x < -1.f || v.x > 1.f) return true;
	if (v.y < -1.f || v.y > 1.f) return true;
	if (v.z < 0.f || v.z > 1.f) return true;

	return false;
}

bool Mesh::Remap(float& value, float min, float max) const
{
	value = (value - min) / (max - min);

	if (value < 0.f || value > 1.f) return false;
	return true;
}

