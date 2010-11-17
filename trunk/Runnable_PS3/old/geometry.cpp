/*   SCE CONFIDENTIAL                                       */
/*   PlayStation(R)3 Programmer Tool Runtime Library 330.001 */
/*   Copyright (C) 2006 Sony Computer Entertainment Inc.    */
/*   All Rights Reserved.                                   */

#include <stdio.h>
#include <assert.h>

#include "cellutil.h"

using namespace Vectormath;
using namespace Vectormath::Aos;

void cellUtilMatrixToFloatArray(Matrix4 &matrix, float array[])
{
	for (int j = 0; j < 4; ++j) {
		for (int i = 0; i < 4; ++i) {
			array[j*4+i] = matrix.getElem(i, j);
		}
	}
}

uint32_t cellUtilGeneratePlane(VertexData3D *point, const float size,
							   const uint32_t row, const uint32_t col)
{
	for (uint32_t j = 0; j < col; ++j) {
		for (uint32_t i = 0; i < row; ++i) {
			point[(j*row+i)*4].pos
				= Point3(-size*row/2 + size*i, 0.f, size*col/2 - size*j -size);
			point[(j*row+i)*4+1].pos
				= Point3(-size*row/2 + size*i, 0.f, size*col/2 - size*j);
			point[(j*row+i)*4+2].pos
				= Point3(-size*row/2 + size*i + size, 0.f, size*col/2 - size*j);
			point[(j*row+i)*4+3].pos
				= Point3(-size*row/2 + size*i + size, 0.f, size*col/2 - size*j - size);
			point[(j*row+i)*4].u = 0.0f; point[(j*row+i)*4].v = 0.0f;
			point[(j*row+i)*4+1].u = 0.0f; point[(j*row+i)*4+1].v = 1.0f;
			point[(j*row+i)*4+2].u = 1.0f; point[(j*row+i)*4+2].v = 1.0f;
			point[(j*row+i)*4+3].u = 1.0f; point[(j*row+i)*4+3].v = 0.0f;
		}
	}

	return row * col * 4;
}
#if 0
uint32_t cellUtilGenerateWall(const uint32_t col, const uint32_t row,
                              const float size,
                              float *vertices, float *uv, uint16_t *indices)
{
	for (uint32_t j = 0; j < row; ++j) {
		for (uint32_t i = 0; i < col; ++i) {
			vertices[(j*col+i)*3]	= ((float)i - (float)col/2.f) * size;
			vertices[(j*col+i)*3+1] = ((float)row/2.f - j - 1) * size;
			vertices[(j*col+i)*3+2] = 0.f;
			uv[(j*col+i)*2] = (float)i / (col - 1);
			uv[(j*col+i)*2+1] = 1.0f - (float)j / (row - 1);
		}
	}
        
	for (uint32_t j = 0; j < row - 1; ++j) {
            for (uint32_t i = 0; i < col - 1; ++i) {
                    uint32_t vertex = j*col+i;
			indices[(j*col+i)*6] = vertex;
			indices[(j*col+i)*6+1] = vertex+col;
			indices[(j*col+i)*6+2] = vertex+1;
			indices[(j*col+i)*6+3] = vertex+col;
			indices[(j*col+i)*6+4] = vertex+col+1;
			indices[(j*col+i)*6+5] = vertex+1;
              
                        }
	}

	return (row - 1) * (col - 1) * 6;
}
#endif /* 0 */
uint32_t cellUtilGenerateQuad(VertexData2D *point, const float size)
{
	point[0].x = -size; point[0].y =  size;
	point[1].x = -size; point[1].y = -size;
	point[2].x =  size; point[2].y = -size;
	point[3].x =  size; point[3].y =  size;

	point[0].u = 0.0f; point[0].v = 0.0f;
	point[1].u = 0.0f; point[1].v = 1.0f;
	point[2].u = 1.0f; point[2].v = 1.0f;
	point[3].u = 1.0f; point[3].v = 0.0f;

	return 4;
}

uint32_t cellUtilGenerateRECTQuad(VertexData2D *point,
								  const uint32_t width, const uint32_t height)
{
	point[0].x = -1.0f; point[0].y =  1.0f;
	point[1].x = -1.0f; point[1].y = -1.0f;
	point[2].x =  1.0f; point[2].y = -1.0f;
	point[3].x =  1.0f; point[3].y =  1.0f;

	point[0].u = 0.0f;                point[0].v = 0.0f;
	point[1].u = 0.0f;                point[1].v = (float)height - 1.0f;
	point[2].u = (float)width - 1.0f; point[2].v = (float)height - 1.0f;
	point[3].u = (float)width - 1.0f; point[3].v = 0.0f;

	return 4;
}

uint32_t cellUtilGenerateCube(VertexData3D *V, const float size)
{
	const VertexData3D *Base = V;
	Point3 point[8];
	point[0] = Point3(-size, -size, -size);
	point[1] = Point3( size, -size, -size);
	point[2] = Point3(-size,  size, -size);
	point[3] = Point3( size,  size, -size);
	point[4] = Point3(-size, -size,  size);
	point[5] = Point3( size, -size,  size);
	point[6] = Point3(-size,  size,  size);
	point[7] = Point3( size,  size,  size);

	// face 0
	V->pos = Point3(point[0]); V->u = 0.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[1]); V->u = 1.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[2]); V->u = 0.0f; V->v = 1.0f; V++;

	V->pos = Point3(point[1]); V->u = 1.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[2]); V->u = 0.0f; V->v = 1.0f; V++;
	V->pos = Point3(point[3]); V->u = 1.0f; V->v = 1.0f; V++;

	// face 1
	V->pos = Point3(point[0]); V->u = 0.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[1]); V->u = 1.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[4]); V->u = 0.0f; V->v = 1.0f; V++;

	V->pos = Point3(point[1]); V->u = 1.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[4]); V->u = 0.0f; V->v = 1.0f; V++;
	V->pos = Point3(point[5]); V->u = 1.0f; V->v = 1.0f; V++;

	// face 2
	V->pos = Point3(point[2]); V->u = 0.0f; V->v = 1.0f; V++;
	V->pos = Point3(point[3]); V->u = 1.0f; V->v = 1.0f; V++;
	V->pos = Point3(point[6]); V->u = 0.0f; V->v = 0.0f; V++;

	V->pos = Point3(point[3]); V->u = 1.0f; V->v = 1.0f; V++;
	V->pos = Point3(point[6]); V->u = 0.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[7]); V->u = 1.0f; V->v = 0.0f; V++;

	// face 3
	V->pos = Point3(point[6]); V->u = 0.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[7]); V->u = 1.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[4]); V->u = 0.0f; V->v = 1.0f; V++;

	V->pos = Point3(point[7]); V->u = 1.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[4]); V->u = 0.0f; V->v = 1.0f; V++;
	V->pos = Point3(point[5]); V->u = 1.0f; V->v = 1.0f; V++;

	// face 4
	V->pos = Point3(point[3]); V->u = 1.0f; V->v = 1.0f; V++;
	V->pos = Point3(point[1]); V->u = 0.0f; V->v = 1.0f; V++;
	V->pos = Point3(point[5]); V->u = 0.0f; V->v = 0.0f; V++;

	V->pos = Point3(point[3]); V->u = 1.0f; V->v = 1.0f; V++;
	V->pos = Point3(point[5]); V->u = 0.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[7]); V->u = 1.0f; V->v = 0.0f; V++;

	// face 5
	V->pos = Point3(point[2]); V->u = 0.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[0]); V->u = 1.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[4]); V->u = 1.0f; V->v = 1.0f; V++;

	V->pos = Point3(point[2]); V->u = 0.0f; V->v = 0.0f; V++;
	V->pos = Point3(point[4]); V->u = 1.0f; V->v = 1.0f; V++;
	V->pos = Point3(point[6]); V->u = 0.0f; V->v = 1.0f; V++;

	return V - Base;
}

uint32_t cellUtilGenerateSphere(const uint32_t meridians,
								const uint32_t parallels,
								const float size,
								float* vertices, uint16_t* indices)
{
	float dtheta = 2.f*M_PI/meridians;
	float dphi = M_PI/parallels;
	for (uint32_t j = 0; j <= parallels; ++j) {
		float y = size * cosf(j*dphi);
		float r = size * sinf(j*dphi);
		for (uint32_t i = 0; i <= meridians; ++i) {
			uint32_t vertex = j*(meridians+1)+i;
			vertices[vertex*3] = r*cosf(i*dtheta);
			vertices[vertex*3+1] = y;
			vertices[vertex*3+2] = r*sinf(i*dtheta);
		}
	}
	for (uint32_t j = 0; j < parallels; ++j) {
		for (uint32_t i = 0; i<meridians; ++i) {
			uint32_t index = j*meridians+i;
			uint32_t vertex = j*(meridians+1)+i;
			indices[index*6] = vertex;
			indices[index*6+1] = vertex+meridians+1;
			indices[index*6+2] = vertex+1;
			indices[index*6+3] = vertex+meridians+1;
			indices[index*6+4] = vertex+meridians+2;
			indices[index*6+5] = vertex+1;
		}
	}

	return parallels * meridians * 6;
}
