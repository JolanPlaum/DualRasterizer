#pragma once
// Includes

namespace dae
{
	// Forward Declarations
	
	// Class Declaration
	class Camera final
	{
	public:
		// Constructors and Destructor
		explicit Camera();
		explicit Camera(const Vector3& origin, float fovAngle, float aspectRatio);
		~Camera();
		
		// Copy and Move semantics
		Camera(const Camera& other)					= default;
		Camera& operator=(const Camera& other)		= default;
		Camera(Camera&& other) noexcept				= default;
		Camera& operator=(Camera&& other) noexcept	= default;
	
		//---------------------------
		// Public Member Functions
		//---------------------------
		void Update(const Timer* pTimer);

		void SetFovAngle(float fovAngle);
		void SetAspectRatio(float aspectRatio);

		Matrix GetInverseViewMatrix() const { return m_InvViewMatrix; }
		Matrix GetViewMatrix() const { return m_ViewMatrix; }
		Matrix GetProjectionMatrix() const { return m_ProjectionMatrix; }
	
	
	private:
		// Member variables
		Vector3 m_Origin{};
		float m_FovAngle{};
		float m_Fov{};
		float m_AspectRatio{};

		Vector3 m_Forward{ Vector3::UnitZ };
		Vector3 m_Up{ Vector3::UnitY };
		Vector3 m_Right{ Vector3::UnitX };

		float m_TotalPitch{};
		float m_TotalYaw{};

		Matrix m_InvViewMatrix{};
		Matrix m_ViewMatrix{};
		Matrix m_ProjectionMatrix{};

		const float m_MovementSpeed{ 10.f };
		const float m_RotationSpeed{ 1.f };

		const float m_Near{ 0.1f };
		const float m_Far{ 100.f };
	
		//---------------------------
		// Private Member Functions
		//---------------------------
		void CalculateViewMatrix();
		void CalculateProjectionMatrix();
	
	};
}
