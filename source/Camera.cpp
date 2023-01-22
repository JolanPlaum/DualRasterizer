//-----------------------------------------------------------------
// Includes
//-----------------------------------------------------------------
#include "pch.h"
#include "Camera.h"

using namespace dae;


//-----------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------
Camera::Camera()
	: Camera({ 0.f,0.f,0.f }, 90.f, 1.f)
{
}

Camera::Camera(const Vector3& origin, float fovAngle, float aspectRatio)
	: m_Origin(origin)
	, m_FovAngle(fovAngle)
	, m_Fov(tanf((fovAngle* TO_RADIANS) / 2.f))
	, m_AspectRatio(aspectRatio)
{
	CalculateViewMatrix();
	CalculateProjectionMatrix();
}


//-----------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------
Camera::~Camera()
{
}


//-----------------------------------------------------------------
// Public Member Functions
//-----------------------------------------------------------------
void Camera::Update(const Timer* pTimer)
{
	const float deltaTime = pTimer->GetElapsed();

	//Keyboard Input
	const uint8_t* pKeyboardState = SDL_GetKeyboardState(nullptr);

	//Mouse Input
	int mouseX{}, mouseY{};
	const uint32_t mouseState = SDL_GetRelativeMouseState(&mouseX, &mouseY);

	//Check if any valid input is received
	if (mouseState & (SDL_BUTTON_LMASK ^ SDL_BUTTON_RMASK)
		|| pKeyboardState[SDL_SCANCODE_W]
		|| pKeyboardState[SDL_SCANCODE_S]
		|| pKeyboardState[SDL_SCANCODE_D]
		|| pKeyboardState[SDL_SCANCODE_A])
	{
		//Camera Movement/Rotation
		float moveSpeed{ m_MovementSpeed * deltaTime * (pKeyboardState[SDL_SCANCODE_LSHIFT] * 3 + 1) };
		float rotSpeed{ m_RotationSpeed * deltaTime };

		m_Origin += pKeyboardState[SDL_SCANCODE_W] * m_Forward * moveSpeed
			- pKeyboardState[SDL_SCANCODE_S] * m_Forward * moveSpeed
			+ pKeyboardState[SDL_SCANCODE_D] * m_Right * moveSpeed
			- pKeyboardState[SDL_SCANCODE_A] * m_Right * moveSpeed;

		bool lmb = mouseState == SDL_BUTTON_LMASK;
		bool rmb = mouseState == SDL_BUTTON_RMASK;
		bool lrmb = mouseState == (SDL_BUTTON_LMASK ^ SDL_BUTTON_RMASK);
		m_Origin -= lmb * m_Forward * moveSpeed * (float)mouseY;
		m_Origin -= lrmb * m_Up * moveSpeed * (float)mouseY;

		m_TotalPitch -= rmb * rotSpeed * (float)mouseY;
		m_TotalYaw += lmb * rotSpeed * (float)mouseX;
		m_TotalYaw += rmb * rotSpeed * (float)mouseX;

		Matrix finalRotation{ Matrix::CreateRotationX(m_TotalPitch) * Matrix::CreateRotationY(m_TotalYaw) };
		m_Forward = finalRotation.TransformVector(Vector3::UnitZ);

		//Update Matrices
		CalculateViewMatrix();
	}
}

void Camera::SetFovAngle(float fovAngle)
{
	m_FovAngle = fovAngle;
	m_Fov = tanf((fovAngle * TO_RADIANS) / 2.f);

	//Re-calculate when fov/aspectRatio changes
	CalculateProjectionMatrix();
}

void Camera::SetAspectRatio(float aspectRatio)
{
	m_AspectRatio = aspectRatio;

	//Re-calculate when fov/aspectRatio changes
	CalculateProjectionMatrix();
}


//-----------------------------------------------------------------
// Private Member Functions
//-----------------------------------------------------------------
void Camera::CalculateViewMatrix()
{
	m_Forward.Normalize();
	m_Right = Vector3::Cross(Vector3::UnitY, m_Forward).Normalized();
	m_Up = Vector3::Cross(m_Forward, m_Right).Normalized();

	m_InvViewMatrix = Matrix{
		m_Right,
		m_Up,
		m_Forward,
		m_Origin
	};

	m_ViewMatrix = Matrix::CreateLookAtLH(m_Origin, m_Forward, Vector3::UnitY);
}

void Camera::CalculateProjectionMatrix()
{
	m_ProjectionMatrix = Matrix::CreatePerspectiveFovLH(m_Fov, m_AspectRatio, m_Near, m_Far);
}

