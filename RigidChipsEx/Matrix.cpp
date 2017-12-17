/*
** �ϊ��s��
*/
#include <cmath>

#include "Matrix.h"

/*
** �C�ӎ�����̉�]
*/
static bool rotate(float *r, float x, float y, float z, float a)
{
	float d = sqrt(x * x + y * y + z * z);
	
	if (d > 0.0f) {
		float l	 = x / d, m	 = y / d, n	 = z / d;
		float l2 = l * l, m2 = m * m, n2 = n * n;
		float lm = l * m, mn = m * n, nl = n * l;
		float rad = a * 3.141593f / 180.0f;
		float c = cos(rad), c1 = 1.0f - c;
		float s = sin(rad);
		
		r[ 0] = (1.0f - l2) * c + l2;
		r[ 1] = lm * c1 + n * s;
		r[ 2] = nl * c1 - m * s;
		r[ 3] = 0.0f;
		
		r[ 4] = lm * c1 - n * s;
		r[ 5] = (1.0f - m2) * c + m2;
		r[ 6] = mn * c1 + l * s;
		r[ 7] = 0.0f;
		
		r[ 8] = nl * c1 + m * s;
		r[ 9] = mn * c1 - l * s;
		r[10] = (1.0f - n2) * c + n2;
		r[11] = 0.0f;
		
		r[12] = 0.0f;
		r[13] = 0.0f;
		r[14] = 0.0f;
		r[15] = 1.0f;
		
		return true;
	}

	return false;
}

/*
** �s��ƃx�N�g���̐� c �� a �~ b
*/
void Matrix::projection(float *c, const float *a, const float *b)
{
	for (int i = 0; i < 4; ++i) {
		c[i] = a[ 0 + i] * b[0]
				 + a[ 4 + i] * b[1]
				 + a[ 8 + i] * b[2]
				 + a[12 + i] * b[3];
	}
}

/*
** �s��̐� c �� a �~ b
*/
void Matrix::multiply(float *c, const float *a, const float *b)
{
	for (int i = 0; i < 16; ++i) {
		int j = i & ~3, k = i & 3;

		c[i] = a[ 0 + k] * b[j + 0]
				 + a[ 4 + k] * b[j + 1]
				 + a[ 8 + k] * b[j + 2]
				 + a[12 + k] * b[j + 3];
	}
}

/*
** �P�ʍs���ݒ肷��
*/
Matrix &Matrix::loadIdentity()
{
	array[ 1] = array[ 2] = array[ 3] = array[ 4] =
	array[ 6] = array[ 7] = array[ 8] = array[ 9] =
	array[11] = array[12] = array[13] = array[14] = 0.0f;
	array[ 0] = array[ 5] = array[10] = array[15] = 1.0f;

	return *this;
}

/*
** ���s�ړ��ϊ��s���ݒ肷��
*/
Matrix &Matrix::loadTranslate(float x, float y, float z, float w)
{
	if (w != 0.0f) {
		x /= w;
		y /= w;
		z /= w;
	}
	array[12] = x;
	array[13] = y;
	array[14] = z;
	array[ 1] = array[ 2] = array[ 3] =
	array[ 4] = array[ 6] = array[ 7] =
	array[ 8] = array[ 9] = array[11] = 0.0f;
	array[ 0] = array[ 5] = array[10] = array[15] = 1.0f;
	
	return *this;
}

/*
** �g��k���ϊ��s���ݒ肷��
*/
Matrix &Matrix::loadScale(float x, float y, float z, float w)
{
	array[ 0] = x;
	array[ 5] = y;
	array[10] = z;
	array[15] = w;
	array[ 1] = array[ 2] = array[ 3] = array[ 4] =
	array[ 6] = array[ 7] = array[ 8] = array[ 9] =
	array[11] = array[12] = array[13] = array[14] = 0.0f;
	
	return *this;
}

/*
** ��]�ϊ��s���ݒ肷��
*/
Matrix &Matrix::loadRotate(float x, float y, float z, float a)
{
	::rotate(array, x, y, z, a);
	
	return *this;
}

/*
** ���s�ړ��ϊ��s����悶��
*/
Matrix &Matrix::translate(float x, float y, float z, float w)
{
	if (w != 0.0f) {
		x /= w;
		y /= w;
		z /= w;
	}
	array[12] += array[ 0] * x + array[ 4] * y + array[ 8] * z;
	array[13] += array[ 1] * x + array[ 5] * y + array[ 9] * z;
	array[14] += array[ 2] * x + array[ 6] * y + array[10] * z;
	array[15] += array[ 3] * x + array[ 7] * y + array[11] * z;

	return *this;
}

/*
** �g��k���ϊ��s����悶��
*/
Matrix &Matrix::scale(float x, float y, float z, float w)
{
	array[ 0] *= x;
	array[ 1] *= x;
	array[ 2] *= x;
	array[ 3] *= x;
	array[ 4] *= y;
	array[ 5] *= y;
	array[ 6] *= y;
	array[ 7] *= y;
	array[ 8] *= z;
	array[ 9] *= z;
	array[10] *= z;
	array[11] *= z;
	array[12] *= w;
	array[13] *= w;
	array[14] *= w;
	array[15] *= w;

	return *this;
}

/*
** ��]�s��ϊ����悶��
*/
Matrix &Matrix::rotate(float x, float y, float z, float a)
{
	float r[16];
	
	if (::rotate(r, x, y, z, a)) multiply(r);

	return *this;
}

/*
** ���_�̈ړ�
*/
Matrix &Matrix::lookat(float ex, float ey, float ez, float tx, float ty, float tz, float ux, float uy, float uz)
{
	float t[16], l;

	/* z �� = e - t */
	tx = ex - tx;
	ty = ey - ty;
	tz = ez - tz;
	l = sqrt(tx * tx + ty * ty + tz * tz);
	if (l == 0.0f) return *this;
	t[ 2] = tx / l;
	t[ 6] = ty / l;
	t[10] = tz / l;

	/* x �� = u x z �� */
	tx = uy * t[10] - uz * t[ 6];
	ty = uz * t[ 2] - ux * t[10];
	tz = ux * t[ 6] - uy * t[ 2];
	l = sqrt(tx * tx + ty * ty + tz * tz);
	if (l == 0.0f) return *this;
	t[ 0] = tx / l;
	t[ 4] = ty / l;
	t[ 8] = tz / l;

	/* y �� = z �� x x �� */
	t[ 1] = t[ 6] * t[ 8] - t[10] * t[ 4];
	t[ 5] = t[10] * t[ 0] - t[ 2] * t[ 8];
	t[ 9] = t[ 2] * t[ 4] - t[ 6] * t[ 0];

	/* ���s�ړ� */
	t[12] = -(ex * t[ 0] + ey * t[ 4] + ez * t[ 8]);
	t[13] = -(ex * t[ 1] + ey * t[ 5] + ez * t[ 9]);
	t[14] = -(ex * t[ 2] + ey * t[ 6] + ez * t[10]);

	/* �c�� */
	t[ 3] = t[ 7] = t[11] = 0.0f;
	t[15] = 1.0f;

	multiply(t);

	return *this;
}

/*
** ���s���e�ϊ��s��
*/
Matrix &Matrix::orthogonal(float left, float right, float bottom, float top, float near, float far)
{
	float dx = right - left;
	float dy = top - bottom;
	float dz = far - near;
	
	if (dx != 0.0f && dy != 0.0f && dz != 0.0f) {
		float t[16];
		
		t[ 0] =	 2.0f / dx;
		t[ 5] =	 2.0f / dy;
		t[10] = -2.0f / dz;
		t[12] = -(right + left) / dx;
		t[13] = -(top + bottom) / dy;
		t[14] = -(far + near) / dz;
		t[15] =	 1.0f;
		t[ 1] = t[ 2] = t[ 3] = t[ 4] =
		t[ 6] = t[ 7] = t[ 8] = t[ 9] = t[11] = 0.0f;
		
		multiply(t);
	}
	
	return *this;
}

/*
** �������e�ϊ��s��
*/
Matrix &Matrix::perspective(float left, float right, float bottom, float top, float near, float far)
{
	float dx = right - left;
	float dy = top - bottom;
	float dz = far - near;
	
	if (dx != 0.0f && dy != 0.0f && dz != 0.0f) {
		float t[16];
		
		t[ 0] =	 2.0f * near / dx;
		t[ 5] =	 2.0f * near / dy;
		t[ 8] =	 (right + left) / dx;
		t[ 9] =	 (top + bottom) / dy;
		t[10] = -(far + near) / dz;
		t[11] = -1.0f;
		t[14] = -2.0f * far * near / dz;
		t[ 1] = t[ 2] = t[ 3] = t[ 4] =
		t[ 6] = t[ 7] = t[12] = t[13] = t[15] = 0.0f;
		
		multiply(t);
	}
		
	return *this;
}

/*
** ��p���瓧�����e�ϊ��s��
*/
Matrix &Matrix::camera(float fovy, float aspect, float near, float far)
{
	float dz = far - near;
	
	if (dz != 0.0f) {
		float f = 1.0f / tan(fovy * 0.5f * 3.141593f / 180.0f);
		float t[16];
		
		t[ 0] = f / aspect;
		t[ 5] = f;
		t[10] = -(far + near) / dz;
		t[11] = -1.0f;
		t[14] = -2.0f * far * near / dz;
		t[ 1] = t[ 2] = t[ 3] = t[ 4] =
		t[ 6] = t[ 7] = t[ 8] = t[ 9] =
		t[12] = t[13] = t[15] = 0.0f;
		
		multiply(t);
	}

	return *this;
}

/*
** �]�u�s��
*/
Matrix &Matrix::transpose(const Matrix &m)
{
	array[ 0] = m.array[ 0];
	array[ 1] = m.array[ 4];
	array[ 2] = m.array[ 8];
	array[ 3] = m.array[12];
	array[ 4] = m.array[ 1];
	array[ 5] = m.array[ 5];
	array[ 6] = m.array[ 9];
	array[ 7] = m.array[13];
	array[ 8] = m.array[ 2];
	array[ 9] = m.array[ 6];
	array[10] = m.array[10];
	array[11] = m.array[14];
	array[12] = m.array[ 3];
	array[13] = m.array[ 7];
	array[14] = m.array[11];
	array[15] = m.array[15];
	
	return *this;
}

/*
** �t�s��
*/
Matrix &Matrix::invert(const Matrix &m)
{
	float lu[20], *plu[4];
	const float *marray = m.array;
	
	/* j �s�̗v�f�̒l�̐�Βl�̍ő�l�� plu[j][4] �ɋ��߂� */
	for (int j = 0; j < 4; ++j) {
		float max = fabs(*(plu[j] = lu + 5 * j) = *(marray++));
		
		for (int i = 0; ++i < 4;) {
			float a = fabs(plu[j][i] = *(marray++));
			if (a > max) max = a;
		}
		if (max == 0.0f) return *this;
		plu[j][4] = 1.0f / max;
	}
	
	/* �s�{�b�g���l������ LU ���� */
	for (int j = 0; j < 4; ++j) {
		float max = fabs(plu[j][j] * plu[j][4]);
		int i = j;
		
		for (int k = j; ++k < 4;) {
			float a = fabs(plu[k][j] * plu[k][4]);
			if (a > max) {
				max = a;
				i = k;
			}
		}
		if (i > j) {
			float *t = plu[j];
			plu[j] = plu[i];
			plu[i] = t;
		}
		if (plu[j][j] == 0.0f) return *this;
		for (int k = j; ++k < 4;) {
			plu[k][j] /= plu[j][j];
			for (int i = j; ++i < 4;) {
				plu[k][i] -= plu[j][i] * plu[k][j];
			}
		}
	}
	
	/* LU ��������t�s������߂� */
	for (int k = 0; k < 4; ++k) {
		/* m �ɒP�ʍs���ݒ肷�� */
		for (int i = 0; i < 4; ++i) {
			array[i * 4 + k] = (plu[i] == lu + k * 5) ? 1.0f : 0.0f;
		}
		/* lu ����t�s������߂� */
		for (int i = 0; i < 4; ++i) {
			for (int j = i; ++j < 4;) {
				array[j * 4 + k] -= array[i * 4 + k] * plu[j][i];
			}
		}
		for (int i = 4; --i >= 0;){
			for (int j = i; ++j < 4;) {
				array[i * 4 + k] -= plu[i][j] * array[j * 4 + k];
			}
			array[i * 4 + k] /= plu[i][i];
		}
	}
	
	return *this;
}

/*
** �@���ϊ��s��
*/
Matrix &Matrix::normal(const Matrix &m)
{
	array[ 0] = m.array[ 5] * m.array[10] - m.array[ 6] * m.array[ 9];
	array[ 1] = m.array[ 6] * m.array[ 8] - m.array[ 4] * m.array[10];
	array[ 2] = m.array[ 4] * m.array[ 9] - m.array[ 5] * m.array[ 8];
	array[ 4] = m.array[ 9] * m.array[ 2] - m.array[10] * m.array[ 1];
	array[ 5] = m.array[10] * m.array[ 0] - m.array[ 8] * m.array[ 2];
	array[ 6] = m.array[ 8] * m.array[ 1] - m.array[ 9] * m.array[ 0];
	array[ 8] = m.array[ 1] * m.array[ 6] - m.array[ 2] * m.array[ 5];
	array[ 9] = m.array[ 2] * m.array[ 4] - m.array[ 0] * m.array[ 6];
	array[10] = m.array[ 0] * m.array[ 5] - m.array[ 1] * m.array[ 4];
	array[ 3] = array[ 7] = array[11] = array[12] = array[13] = array[14] = 0.0f;
	array[15] = 1.0f;

	return *this;
}
