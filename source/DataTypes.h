#pragma once

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		ColorRGB color{ colors::White };
	};

	struct Vertex_Out
	{
		Vector4 position{};
		Vector2 uv{};
		Vector3 normal{};
		Vector3 tangent{};
		ColorRGB color{ colors::White };
		Vector3 worldPosition{};
	};
}
