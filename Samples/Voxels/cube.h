#pragma once

static const float CUBE_VERTICES[] = {
	-1.0f,-1.0f,-1.0f,  // -X side
	-1.0f,-1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,

	-1.0f,-1.0f,-1.0f,  // -Z side
	1.0f, 1.0f,-1.0f,
	1.0f,-1.0f,-1.0f,
	-1.0f,-1.0f,-1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,

	-1.0f,-1.0f,-1.0f,  // -Y side
	1.0f,-1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f,-1.0f,
	1.0f,-1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,

	-1.0f, 1.0f,-1.0f,  // +Y side
	-1.0f, 1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f, 1.0f,-1.0f,
	1.0f, 1.0f, 1.0f,
	1.0f, 1.0f,-1.0f,

	1.0f, 1.0f,-1.0f,  // +X side
	1.0f, 1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f,-1.0f,-1.0f,
	1.0f, 1.0f,-1.0f,

	-1.0f, 1.0f, 1.0f,  // +Z side
	-1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
	-1.0f,-1.0f, 1.0f,
	1.0f,-1.0f, 1.0f,
	1.0f, 1.0f, 1.0f,
};

static const float CUBE_UVS[] = {
	0.0f, 1.0f,  // -X side
	1.0f, 1.0f,
	1.0f, 0.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,
	0.0f, 1.0f,

	1.0f, 1.0f,  // -Z side
	0.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 0.0f,

	1.0f, 0.0f,  // -Y side
	1.0f, 1.0f,
	0.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	0.0f, 0.0f,

	1.0f, 0.0f,  // +Y side
	0.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,

	1.0f, 0.0f,  // +X side
	0.0f, 0.0f,
	0.0f, 1.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,

	0.0f, 0.0f,  // +Z side
	0.0f, 1.0f,
	1.0f, 0.0f,
	0.0f, 1.0f,
	1.0f, 1.0f,
	1.0f, 0.0f,
}; 
