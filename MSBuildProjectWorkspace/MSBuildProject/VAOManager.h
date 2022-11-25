#pragma once

#include <string>
#include <map>
#include <glad/glad.h>
#include <glm/glm.hpp>

struct Vertex
{
	float x, y, z;
	float r, g, b;
	float nx, ny, nz;
};

struct ModelDrawInfo
{
	ModelDrawInfo();

	std::string meshName;

	unsigned int VAOId;

	unsigned int VertexBufferId;
	unsigned int VertexBufferStartIndex;
	unsigned int numberOfVertices;

	unsigned int IndexBufferId;
	unsigned int IndexBufferStartIndex;
	unsigned int numberOfIndices;
	unsigned int numberOfTriangles;

	Vertex* pVertices;
	unsigned int* pIndices;
};

class VAOManager
{
public:
	VAOManager();
	~VAOManager();
	bool LoadModelIntoVAO(std::string fileName,
		ModelDrawInfo& drawInfo,
		unsigned int shaderProgramID);

	bool FindDrawInfoByModelName(std::string filename,
		ModelDrawInfo& drawInfo);

	std::string GetLastError(bool bAndClear = true);

private:
	std::map<std::string, ModelDrawInfo> mMapModelNameToVAOId;
};
