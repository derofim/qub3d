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

#include "chunk.hpp"

#include <glm/gtc/matrix_transform.hpp>

using namespace qub3d;

void Chunk::createMesh(viking::IRenderer *renderDevice, viking::IModelPool *modelPool)
{
	m_blockUniformBuffer = renderDevice->createUniformBuffer(
		&m_blockPositionsBuffer, 
		sizeof(glm::mat4), 
		16 * 16 * 256, 
		viking::ShaderStage::VERTEX_SHADER, 
		2
	);

	modelPool->attachBuffer(0, m_blockUniformBuffer);

	for (int x = 0; x < 16; x++)
	{
		for (int y = 0; y < 256; y++)
		{
			for (int z = 0; z < 16; z++)
			{
				glm::mat4 blockPosition = glm::mat4(1.0f);
				blockPosition = glm::translate(blockPosition, glm::vec3(x, y, z));

				viking::IModel *model = modelPool->createModel();
				model->SetData(0, &blockPosition);
				
				m_models.push_back(model);
			}
		}
	}
}