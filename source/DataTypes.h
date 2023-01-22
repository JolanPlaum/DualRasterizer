#pragma once

namespace dae
{
	struct Vertex
	{
		Vector3 position{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector2 uv{};
	};

	struct Vertex_Out
	{
		Vector4 position{};
		Vector3 normal{};
		Vector3 tangent{};
		Vector2 uv{};
		Vector3 worldPosition{};
	};
}
