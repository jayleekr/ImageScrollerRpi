#include <stdio.h>
#include <string.h>

#include <math.h>
#include <assert.h>
#include <fcntl.h>
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <sys/time.h>
#include <signal.h>
#include <bcm_host.h>
#include <errno.h>
#include <pthread.h>
#include "Screen_setup.h"
#include "LoadShaders.h"
#include <FreeImage.h>
#include <stdint.h>     	// uintXX_t
#include <string>       	// std::string
#include <sstream>      	// std::ostringstream
#include <iostream>    	 	// cout cerr
#include <unistd.h>     	// access
#include <list>             // std::list
#include <algorithm>		// std::copy , std::fill_n
#include <getopt.h>     	// getopt_long


#define _IN
#define _OUT

#define OFFSET					0.005
#define FPS						50
#define VERTEX_POS_SIZE 		3
#define VERTEX_TEXCOORD0_SIZE  	2
#define VERTEX_TEXCOORD1_SIZE  	2
#define VERTEX_TEXCOORD2_SIZE  	2
#define VERTEX_TEXCOORD3_SIZE  	2
#define VERTEX_TEXCOORD_SIZE 	2

typedef
	std::list< std::string >
	FileList_t;

typedef enum Mode
{
	LEFT,
	RIGHT,
	CENTER,
	SCROLLING
}Mode_t;

typedef struct _Display
{
	GLuint Width,Height;
	void Clear()
	{
		Width = Height = 0;
	};
}Display_t;

typedef struct _ID
{
	GLint program, vertex, texture0,texture1, texture2,texture3,
			StartLocation,MappedTextures,
			tex0Matrix, tex1Matrix,tex2Matrix,tex3Matrix,Translation,
			tex0Left,tex1Left,tex2Left,tex3Left;
	void Clear()
	{
		program = vertex = texture0 = texture1 = texture2 = texture3 =
		StartLocation = MappedTextures=
		tex0Left = tex1Left = tex2Left = tex3Left =
		tex0Matrix = tex1Matrix = tex2Matrix = tex3Matrix = Translation = -1;
	};
}ID_t;

class Configuration
{
public:
	GLfloat			RenderPoint;
	Mode_t			Mode;
	ID_t 			ID;
	Display_t 		Display;

	int32_t         Y;
	int32_t         Height;
	int32_t         Speed;
    int32_t         Layer;

	GLfloat			Yconverted;
	GLfloat			Xconverted;
	GLfloat			TurningPoint;
	GLuint 			NumberOfTextures;
	GLuint			Fps;
	GLfloat			Offset;

	bool			Verbose;
	bool			ViewPoint;


	std::string     Alignment;

	FileList_t	    FileList;

	Configuration();
};

class TextureInfo
{
public:
	GLint 	TextureID;
	GLint	MatrixID;
	GLint 	LeftMarginID;
	GLuint 	Width;
	GLuint 	Height;
	GLfloat Matrix[16];

	GLfloat ScaledWidth;
	GLfloat ScaledHeight;

	GLfloat GLCoord_ScaledWidth;
	GLfloat GLCoord_ScaledHeight;

	GLfloat LeftMargin;

	TextureInfo();

	void CalculateScaleValues(void);
};

typedef
	std::list< TextureInfo >
	TextureList_t;

class ImageInfo
{
public:
	GLfloat				GLCoord_TotalWidth;

	TextureList_t		TextureList;

	ImageInfo();
	void CalculateTotalWidth();
	void UpdateLocations();
};


void InitImageScroller(void);
void ActivateTextures(void);
bool DoRender(void);
void LoadFiles(void);

void Close(std::string errorMgs = "");

void printUsage(const char *progr);
static void printVersion();


const GLfloat g_vertex_buffer_data[2*4][3*3]=
{
	// total 4 textures
	// 1-2 Triangles -->0th quad
	{
		 0.0f, 0.0f, 1.0f,
		 0.0f, 1.0f, 1.0f,
		 0.999999f, 0.0f, 1.0f
	},
	{
		 0.0f, 1.0f, 1.0f,
		 0.999999f, 0.0f, 1.0f,
		 0.999999f, 1.0f, 1.0f
	},

	// 3-4 Triangles -->1st quad

	{
		 1.0f, 0.0f, 1.0f,
		 1.0f, 1.0f, 1.0f ,
		 1.999999f, 0.0f, 1.0f

	},
	{
		1.0f, 1.0f, 1.0f,
		1.999999f, 0.0f, 1.0f,
		1.999999f, 1.0f, 1.0f
	},

	// 5-6 Triangls -->2nd quad
	{
		2.0f, 0.0f, 1.0f,
		2.0f, 1.0f, 1.0f,
		2.999999f, 0.0f, 1.0f
	},
	{
		2.0f, 1.0f, 1.0f,
		2.999999f, 0.0f, 1.0f,
		2.999999f, 1.0f, 1.0f
	},
	// 6-7 Triangles -->3rd quad
	{
		3.0f, 0.0f, 1.0f,
		3.0f, 1.0f, 1.0f,
		3.999999f, 0.0f, 1.0f
	},
	{
		3.0f, 1.0f, 1.0f,
		3.999999f, 0.0f, 1.0f,
		3.999999f, 1.0f, 1.0f
	},
	// total 4 textures
};
const GLfloat texture0_buffer_data[2*2][2*3] =
{
	{
		0.0f, 0.0f,
		0.0f, 1.0f,
		 1.0f, 0.0f
	},
	{
		0.0f, 1.0f,
		 1.0f, 0.0f,
		  1.0f, 1.0f
	}


};
const GLfloat texture1_buffer_data[2*2][2*3] =
{
	{
		0.0f, 0.0f,
		0.0f, 1.0f,
		 1.0f, 0.0f
	},
	{
		0.0f, 1.0f,
		 1.0f, 0.0f,
		  1.0f, 1.0f
	}
};
const GLfloat texture2_buffer_data[2*2][2*3] =
{
	{
		0.0f, 0.0f,
		0.0f, 1.0f,
		 1.0f, 0.0f
	},
	{
		0.0f, 1.0f,
		 1.0f, 0.0f,
		  1.0f, 1.0f
	}
};
const GLfloat texture3_buffer_data[2*2][2*3] =
{
	{
		0.0f, 0.0f,
		0.0f, 1.0f,
		 1.0f, 0.0f
	},
	{
		0.0f, 1.0f,
		 1.0f, 0.0f,
		  1.0f, 1.0f
	}
};
