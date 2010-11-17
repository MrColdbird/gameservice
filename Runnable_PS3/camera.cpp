/*  SCE CONFIDENTIAL
 *  PlayStation(R)3 Programmer Tool Runtime Library 330.001
 *  Copyright (C) 2010 Sony Computer Entertainment Inc.
 *  All Rights Reserved.
 */

#define __CELL_ASSERT__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/process.h>
#include <cell/sysmodule.h>

#include <cell/cell_fs.h>
#include <sys/paths.h>

// using vectormath
#include <vectormath/cpp/vectormath_aos.h>
using namespace Vectormath::Aos;

// gcmutil
#include "gcmutil.h"
using namespace CellGcmUtil;


namespace{
	const float PI = 3.14159265f;

	int32_t roundAxis(uint8_t value)
	{
		int32_t ret = value - 128;
		if(-32 < ret && ret < 32 ) ret = 0;
		if(-112 > ret) ret = -256;
		if(112 < ret) ret = 256;
		
		return ret;
	}

	Vector3 m_eye(0.0f, 0.0f, 10.0f);
	Vector3 m_at(0.0f, 0.0f, 0.0f);
	Vector3 m_up(0.0f, 1.0f, 0.0f);

	Vector3 m_pos(0.0f, 0.0f, 0.0f);
	float m_y = 0.0f;
	float m_t = 0.0f;

	Vector3 sInitEye(0.0f, 0.0f, 10.0f);
	Vector3 sInitAt(0.0f, 0.0f, 0.0f);
	Vector3 sInitUp(0.0f, 1.0f, 0.0f);

	Vector3 sInitTrans(0.0f, 0.0f, 0.0f);
	float sInitYaw(0.0f), sInitPitch(0.0f);
} // no name namespace

namespace CellGcmUtil{

void _cellGcmUtilSimpleCameraUpdateParam()
{
	Vector3 angX(1.0f, 0.0f, 0.0f);
	Matrix3 ry = Matrix3::rotationZYX(Vector3(0.0f, m_y, 0.0f));
	angX = ry * angX;

	Matrix3 rt = Matrix3::rotation(m_t, angX);
	m_eye = rt * ry * sInitEye + m_pos;
	m_up = rt * sInitUp;
	m_at = sInitAt + m_pos;
}

void cellGcmUtilSimpleCameraInit(Vector3 eye, Vector3 at, Vector3 up)
{
	sInitEye = eye;
	sInitAt = at;
	sInitUp = up;

	m_y = m_t= 0.0f;
	m_pos = Vector3(0.0f);

	_cellGcmUtilSimpleCameraUpdateParam();
}

void cellGcmUtilSimpleCameraRotate(uint8_t angleX, uint8_t angleY, bool bReverseX, bool bReverseY, bool bReset)
{
	int32_t dx = roundAxis(angleX);
	int32_t dy = roundAxis(angleY);

	float dxf = dx / 256.0f;
	float dyf = dy / 256.0f;

	float df = PI * 3.0f / 180.0f;

	m_y += df * dxf * (bReverseX? -1.0f: 1.0f);
	m_t += df * dyf * (bReverseY? -1.0f: 1.0f);
	
	if(m_t > PI * 0.5f){
		m_t = PI * 0.5f;
	}else if(m_t < -PI * 0.5f){
		m_t = -PI * 0.5f;
	}
	if(bReset){
		m_y = sInitYaw;
		m_t = sInitPitch;
	}

	_cellGcmUtilSimpleCameraUpdateParam();
}

void cellGcmUtilSimpleCameraMove(int32_t dx, int32_t dy, int32_t dz, bool bReset)
{
	Vector3 dpos(0.0f, 0.0f, 0.0f);

	float dxf = dx / 1024.0f;
	float dyf = dy / 1024.0f;
	float dzf = dz / 1024.0f;

	dpos[0] += dxf;
	dpos[1] += -dyf;
	dpos[2] += dzf;

	if(bReset){
		m_pos = sInitTrans;
	}else{
		Matrix3 ry = Matrix3::rotationZYX(Vector3(0.0f, m_y, 0.0f));
		m_pos += ry * dpos;
	}

	_cellGcmUtilSimpleCameraUpdateParam();
}

void cellGcmUtilSimpleCameraUpdate()
{
	{
		CellPadUtilAxis angle = cellPadUtilGetAxisValue(0, CELL_UTIL_ANALOG_RIGHT);
		bool reset = cellPadUtilButtonPressed(0, CELL_UTIL_BUTTON_R3);
		cellGcmUtilSimpleCameraRotate(angle.x, angle.y, false, false, reset);
	}

	if(0){
		bool reset = cellPadUtilButtonPressed(0, CELL_UTIL_BUTTON_L3);
		CellPadUtilPress press = cellPadUtilGetPressValue(0);
		int32_t dx = (press.right - press.left) * 3;
		int32_t dy = (press.down - press.up) * 3;
		cellGcmUtilSimpleCameraMove(dx, dy, 0, reset);
	}
	{
		bool reset = cellPadUtilButtonPressed(0, CELL_UTIL_BUTTON_L3);
		CellPadUtilAxis angle = cellPadUtilGetAxisValue(0, CELL_UTIL_ANALOG_LEFT);
		int32_t dx = roundAxis(angle.x);
		int32_t dy = roundAxis(angle.y);
		if(cellPadUtilButtonPressed(0, CELL_UTIL_BUTTON_L1)){
			cellGcmUtilSimpleCameraMove(dx, dy, 0, reset);
		}else{
			cellGcmUtilSimpleCameraMove(dx, 0, dy, reset);
		}
	}
}

Matrix4 cellGcmUtilSimpleCameraGetMatrix()
{
	return Matrix4::lookAt(Point3(m_eye), Point3(m_at), m_up) * Matrix4::identity();
}
void cellGcmUtilSimpleCameraGetInitParam(Vector3 *eye, Vector3 *at, Vector3 *up)
{
	if(eye) *eye = sInitEye;
	if(at) *at = sInitAt;
	if(up) *up = sInitUp;
}
void cellGcmUtilSimpleCameraGetStatus(Vector3 *eye, Vector3 *at, Vector3 *up)
{
	if(eye) *eye = m_eye;
	if(at) *at = m_at;
	if(up) *up = m_up;
}
void cellGcmUtilSimpleCameraGetParam(Vector3 *trans, float *yaw, float *pitch)
{
	if(trans) *trans = m_pos;
	if(yaw) *yaw = m_y;
	if(pitch) *pitch = m_t;
}
void cellGcmUtilSimpleCameraSetParam(Vector3 trans, float yaw, float pitch)
{
	m_pos = sInitTrans = trans;
	m_y = sInitYaw = yaw;
	m_t = sInitPitch = pitch;

	_cellGcmUtilSimpleCameraUpdateParam();
}

} // namespaceCellGcmUtil
