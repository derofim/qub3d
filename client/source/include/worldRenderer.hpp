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

#pragma once

#include <viking/IGraphicsPipeline.hpp>
#include <viking/IRenderer.hpp>
#include <viking/IUniformBuffer.hpp>
#include <viking/IModelPool.hpp>

#include "camera.hpp"
#include "chunk.hpp"

namespace qub3d
{
	struct CameraUniformBufferData
	{
		glm::mat4 view;
		glm::mat4 projection;
	};

	class WorldRenderer
	{
	public:
		void initalize(viking::IRenderer* renderDevice, Camera *initalCamera);
		void freeResources();

		void render();

		Chunk *createChunk();

		void setActiveCamera(Camera *camera);
		inline Camera *getActiveCamera() const { return m_activeCamera; }

	private:
		Camera *m_activeCamera;
		CameraUniformBufferData m_cameraUniformBufferData;
		viking::IUniformBuffer *m_cameraUniformBuffer;

		viking::IRenderer *m_renderDevice;
		viking::IModelPool *m_modelPool;

		viking::VertexBufferBase m_vertexBufferBase;
		viking::IGraphicsPipeline *m_pipeline;
	};
}