/*
*	 Copyright (C) 2018 Qub³d Engine Group.
*	 All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without modification,
*  are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice, this
*  list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright notice,
*  this list of conditions and the following disclaimer in the documentation and/or
*  other materials provided with the distribution.
*
*  3. Neither the name of the copyright holder nor the names of its contributors
*  may be used to endorse or promote products derived from this software without
*  specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
*  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
*  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
*  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
*  ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
*  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
*  ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
*  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "worldRenderer.hpp"

#include <vector>
#include <fstream>
#include <string>

using namespace qub3d;

#define RESOLVE_ASSET_PATH(path) ASSETS_PATH path

namespace {
	struct Vertex
	{
		Vertex() {}
		Vertex(glm::vec3 position, glm::vec2 uv) : position(position), uv(uv) {}
		glm::vec3 position;
		glm::vec2 uv;

		bool operator==(Vertex v)
		{
			return v.position == position && v.uv == uv;
		}
	};

	viking::ITextureBuffer *loadTexture(viking::IRenderer* renderDevice, const char *path)
	{
		unsigned char header[54];
		unsigned int dataPos;
		unsigned int width, height;
		unsigned int imageSize;
		unsigned char *data;

		FILE *file = fopen(path, "rb");
		if (!file)
		{
			printf("Image could not be opened\n");
			return 0;
		}

		if (fread(header, 1, 54, file) != 54)
		{
			printf("Not a correct BMP file\n");
			return 0;
		}

		if (header[0] != 'B' || header[1] != 'M')
		{
			printf("Not a correct BMP file\n");
			return 0;
		}

		dataPos = *(int *)&(header[0x0A]);
		imageSize = *(int *)&(header[0x22]);
		width = *(int *)&(header[0x12]);
		height = *(int *)&(header[0x16]);

		if (imageSize == 0)
			imageSize = width * height * 3;
		if (dataPos == 0)
			dataPos = 54;

		data = new unsigned char[imageSize];

		fread(data, 1, imageSize, file);

		fclose(file);

		return renderDevice->createTextureBuffer(data, width, height);
	}

	std::vector<std::string> splitString(std::string dec, std::string str)
	{
		std::vector<std::string> parts;
		while (true)
		{
			int index = static_cast<int>(str.find(dec));
			if (index == std::string::npos)
				break;
			parts.push_back(str.substr(0, index));
			str.replace(0, index + dec.length(), "");
		}
		parts.push_back(str);
		return parts;
	}

	std::vector<std::string> splitString(const char *dec, std::string str)
	{
		std::string s(dec);
		return splitString(s, str);
	}

	void loadOBJ(std::string location, std::vector<uint16_t> &out_vertex_index,
		std::vector<Vertex> &out_vertices)
	{
		std::ifstream myfile(location);
		if (myfile.is_open())
		{
			std::string line;
			std::vector<glm::vec3> temp_vertices;
			std::vector<glm::vec2> temp_uvs;
			std::vector<glm::vec3> temp_normals;
			std::vector<std::vector<std::string>> temp_faces;
			while (std::getline(myfile, line))
			{
				std::vector<std::string> parts = splitString(" ", line);

				if (line.find("v ") == 0)
				{
					if (parts.size() >= 4)
					{
						glm::vec3 vertex(::atof(parts[parts.size() - 3].c_str()), ::atof(parts[parts.size() - 2].c_str()), ::atof(parts[parts.size() - 1].c_str()));
						temp_vertices.push_back(vertex);
					}
					else
					{
						assert(0 && "Malformed Vertex, attempting to continue");
					}
				}
				else if (line.find("vt ") == 0)
				{
					if (parts.size() >= 3)
					{
						glm::vec2 vt(::atof(parts[parts.size() - 2].c_str()), ::atof(parts[parts.size() - 1].c_str()));
						temp_uvs.push_back(vt);
					}
					else
					{
						assert(0 && "Malformed Vertex Texture, attempting to continue");
					}
				}
				else if (line.find("vn ") == 0)
				{
					if (parts.size() >= 4)
					{
						glm::vec3 vn(::atof(parts[parts.size() - 3].c_str()), ::atof(parts[parts.size() - 2].c_str()), ::atof(parts[parts.size() - 1].c_str()));
						temp_normals.push_back(vn);
					}
					else
					{
						assert(0 && "Malformed Vertex Normal, attempting to continue");
					}
				}
				else if (line.find("f ") == 0)
				{
					temp_faces.push_back(parts);
				}
			}
			for (auto parts : temp_faces)
			{
				if (parts.size() >= 4)
				{
					std::vector<std::string> face_parts = { parts[1], parts[2], parts[3] };
					for (int j = 4; j < parts.size(); j++)
					{
						face_parts.push_back(parts[1]);
						face_parts.push_back(parts[j - 1]);
						face_parts.push_back(parts[j]);
					}

					for (int j = 0; j < face_parts.size(); j++)
					{
						std::vector<std::string> parts2 = splitString("/", face_parts[j]);
						if (parts2.size() != 0)
						{
							int requested_vertex = static_cast<int>(::atof(parts2[0].c_str()) - 1);
							int requested_texture = static_cast<int>((parts2.size() > 1) ? ::atof(parts2[1].c_str()) - 1 : 0);
							int requested_normal = static_cast<int>((parts2.size() > 2) ? ::atof(parts2[2].c_str()) - 1 : 0);

							auto pair = std::make_pair(requested_texture, requested_normal);
							glm::vec2 textureCoord(1.0f);
							if (temp_uvs.size() > requested_texture)
								textureCoord = temp_uvs[requested_texture];

							Vertex v = { temp_vertices[requested_vertex], textureCoord };
							auto it = std::find(out_vertices.begin(), out_vertices.end(), v);
							if (it == out_vertices.end())
							{
								out_vertex_index.push_back(static_cast<uint32_t>(out_vertices.size()));
								out_vertices.push_back(v);
							}
							else
							{
								int pos = static_cast<int>(std::distance(out_vertices.begin(), it));
								out_vertex_index.push_back(pos);
							}
						}
					}
				}
			}
		}
	}
}

void WorldRenderer::initalize(viking::IRenderer *renderDevice, Camera * initalCamera)
{
	using namespace viking;

	m_renderDevice = renderDevice;
	
	m_pipeline = renderDevice->createGraphicsPipeline(
		{ 
			{ ShaderStage::VERTEX_SHADER, RESOLVE_ASSET_PATH("assets/shaders/shader.vert") },
			{ ShaderStage::FRAGMENT_SHADER, RESOLVE_ASSET_PATH("assets/shaders/shader.frag") } 
		}
	);

	m_vertexBufferBase = {
		{
			{ 0, sizeof(glm::vec3), offsetof(Vertex, position) },
			{ 1, sizeof(glm::vec2), offsetof(Vertex, uv) }
		},
		sizeof(Vertex)
	};

	m_pipeline->attachVertexBinding(m_vertexBufferBase);
	m_pipeline->build();

	std::vector<Vertex> vertexData;
	std::vector<uint16_t> indexData;

	loadOBJ(RESOLVE_ASSET_PATH("assets/models/cube.obj"), indexData, vertexData);

	IBuffer *vertexBuffer = renderDevice->createBuffer(vertexData.data(), sizeof(Vertex), vertexData.size());
	IBuffer *indexBuffer = renderDevice->createBuffer(indexData.data(), sizeof(uint16_t), indexData.size());

	m_modelPool = renderDevice->createModelPool(&m_vertexBufferBase, vertexBuffer, indexBuffer);

	ITextureBuffer *textureBuffer = loadTexture(renderDevice, RESOLVE_ASSET_PATH("assets/textures/cobble.bmp"));
	m_modelPool->attachBuffer(textureBuffer);

	m_cameraUniformBufferData.view = glm::mat4(1.0f);
	m_cameraUniformBufferData.projection = initalCamera->projectionMatrix;
	m_activeCamera = initalCamera;

	m_cameraUniformBuffer = renderDevice->createUniformBuffer(
		&m_cameraUniformBufferData,
		sizeof(CameraUniformBufferData),
		1,
		viking::ShaderStage::VERTEX_SHADER,
		1
	);

	m_modelPool->attachBuffer(m_cameraUniformBuffer);
	m_pipeline->attachModelPool(m_modelPool);
}

Chunk *WorldRenderer::createChunk()
{
	Chunk *chunk = new Chunk;
	chunk->createMesh(m_renderDevice, m_modelPool);

	return chunk;
}

void WorldRenderer::render()
{
	m_cameraUniformBufferData.view = m_activeCamera->calculateViewMatrix();
	m_cameraUniformBuffer->setData();
}

void WorldRenderer::freeResources()
{

}

void WorldRenderer::setActiveCamera(Camera *camera)
{
	m_activeCamera = camera;
	m_cameraUniformBufferData.projection = camera->projectionMatrix;
	m_cameraUniformBufferData.view = camera->calculateViewMatrix();
}
