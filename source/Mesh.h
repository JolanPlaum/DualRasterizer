#pragma once
// Includes
#include "DataTypes.h"

namespace dae
{
	// Class Forward Declarations
	class Material;
	class Texture;
	
	// Class Declaration
	class Mesh final
	{
	public:
		// Constructors and Destructor
		explicit Mesh(ID3D11Device* pDevice, SDL_Surface* pBackBuffer, const std::string& filename, Material* pMaterial);
		~Mesh();
		
		// Copy and Move semantics
		Mesh(const Mesh& other)					= delete;
		Mesh& operator=(const Mesh& other)		= delete;
		Mesh(Mesh&& other) noexcept				= delete;
		Mesh& operator=(Mesh&& other) noexcept	= delete;
	
		//---------------------------
		// Public Member Functions
		//---------------------------
		void Render(ID3D11DeviceContext* pDeviceContext) const;
		void Render(SDL_Surface* pBackBuffer) const;

		void Translate(const Vector3& translation);
		void Rotate(const Vector3& rotation);
		void Scale(float scale);

		void SetPosition(float x, float y, float z);
		void SetRotation(float pitch, float yaw, float roll);
		void SetScale(const Vector3& scale);

		Material* GetMaterial() const { return m_pMaterial; }
		Matrix GetWorldMatrix() const { return Matrix::CreateTransform(m_Position, m_Rotation, m_Scale); }

	
	private:
		// Member variables
		Material* m_pMaterial{};

		Vector3 m_Position{ 0.f, 0.f, 0.f };
		Vector3 m_Rotation{ 0.f, 0.f, 0.f };
		Vector3 m_Scale{ 1.f, 1.f, 1.f };

		//HARDWARE
		uint32_t m_NumIndices{};
		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11Buffer* m_pIndexBuffer{};

		//SOFTWARE
		uint32_t* m_pBackBufferPixels{};
		float* m_pDepthBufferPixels{};
		std::vector<Vertex> m_Vertices{};
		std::vector<uint32_t> m_Indices{};

		bool m_IsShowDepthBuffer{ false };
		bool m_IsShowBoundingBox{ false };
	
		//---------------------------
		// Private Member Functions
		//---------------------------
		void RenderTriangle(SDL_Surface* pBackBuffer, const Vertex_Out& v0, const Vertex_Out& v1, const Vertex_Out& v2) const;

		Vertex_Out NDCToRaster(const Vertex_Out& v, int width, int heigth) const;
		bool FrustumCulling(const Vector4& v) const;
		bool Remap(float& value, float min, float max) const;
	
	};
}
