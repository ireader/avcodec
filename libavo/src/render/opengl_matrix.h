#ifndef _opengl_matrix_h_
#define _opengl_matrix_h_

#include <string.h>

static void opengl_matrix_identity(GLfloat m[16])
{
	memset(m, 0, sizeof(GLfloat) * 16);
	m[0] = 1.0f;
	m[5] = 1.0f;
	m[10] = 1.0f;
	m[15] = 1.0f;
}

#endif /* !_opengl_matrix_h_ */
