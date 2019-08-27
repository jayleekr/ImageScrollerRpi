#include "ImageScroller.hpp"


static const struct option longOpts[] = {
	{"help", no_argument, 0, 'h'},
	{"version", no_argument, 0, 'v'},
	{"debug", no_argument, 0, 'g'},
	{"point", no_argument, 0, 'p'},

    {"speed", required_argument, 0, 's'}, 	// speed
    {"align", required_argument, 0, 'a'}, 	// align

    {"win", required_argument, 0, 0x102}, 	// y,h
    {"file", required_argument, 0, 0x103}, 	// file names
    {"layer", required_argument, 0, 0x104}, // layer
	{0, 0, 0, 0}
};


static 	ImageInfo 			_ImageInfo;
static 	Configuration		_Config;

const int textureUnits[4] = {0,1,2,3};

Configuration::Configuration()
{
	Mode = SCROLLING;
	ID.Clear();

	Y = Xconverted = Yconverted = TurningPoint =
	NumberOfTextures = RenderPoint = 0;
	Height = 256;
	Speed = 2;

	Offset = OFFSET;
	Fps	   = FPS;

	Layer = 11; //Default

	Verbose = false;
	ViewPoint = false;
}

TextureInfo::TextureInfo()
{
	TextureID = MatrixID = -1;
	Width = Height = 0;
	ScaledWidth = ScaledHeight =
	GLCoord_ScaledWidth = GLCoord_ScaledHeight =0;
	LeftMargin = 0;
	std::fill_n(Matrix,16,0);
}

ImageInfo::ImageInfo()
{
	GLCoord_TotalWidth = 0;
}

void TextureInfo::CalculateScaleValues()
{
	// ImageWidth : ImageHeight = ScaledWidth : config.Height(=ScaledHeight)
	// Scaled values must be doubled, because of the GL coordinate system
	ScaledWidth = 2 * (GLfloat) Width * (GLfloat) _Config.Height / (GLfloat) Height;
	ScaledHeight= 2 * (GLfloat) _Config.Height;

	GLCoord_ScaledWidth = ScaledWidth/(GLfloat)_Config.Display.Width;
	GLCoord_ScaledHeight = ScaledHeight/(GLfloat)_Config.Display.Height;
	GLfloat mat[16] =
	{
		GLCoord_ScaledWidth, 0,0,0,
		0, GLCoord_ScaledHeight, 0,0,
		0,0,1,0,
		0,0,0,1
	};
	std::copy(mat, mat +16 , Matrix);
};

void ImageInfo::CalculateTotalWidth(void)
{
	GLfloat w = 0.0f;
	for (auto it = TextureList.begin();
	it != TextureList.end();
	++it)
	{
		w += it->ScaledWidth;
	}

	GLCoord_TotalWidth = w / (GLfloat)_Config.Display.Width;
}

void ImageInfo::UpdateLocations(void)
{

	_Config.TurningPoint = 0.0f;
	switch(_Config.Mode)
	{
		case LEFT:
			break;
		case CENTER:
		{
			_Config.Xconverted = 1.0f - GLCoord_TotalWidth /2.0f;
			break;
		}
		case RIGHT:
		{
			_Config.Xconverted = 2.0f - GLCoord_TotalWidth;
			break;
		}
		case SCROLLING:
		{
			_Config.Xconverted = 2.0f;
			_Config.TurningPoint = 2.0f +  GLCoord_TotalWidth;
			break;
		}
	}

	_Config.Yconverted =  1.0f - (2.0 * (GLfloat) _Config.Height/(GLfloat)_Config.Display.Height)
						- 2.0 * (GLfloat)_Config.Y/(GLfloat)_Config.Display.Height;

}


int main(int argc, char * argv[])
{

	//Get Options
    int opt;
	std::ostringstream oss;
    while((opt = getopt_long(argc, argv, "hvgps:a:",
            longOpts, NULL)) != -1)
    {
        switch(opt)
        {
            case 'h':
                printUsage(argv[0]);
                Close(oss.str());
            case 'v':
                printVersion();
                Close(oss.str());
            case 'g':
                _Config.Verbose = true;
                break;
			case 'p':
                _Config.ViewPoint = true;
                break;
            case 's':
            {
                _Config.Speed = (int32_t)  strtol(optarg,NULL,10);
                if(_Config.Speed < 0 || _Config.Speed > 5)
                {
                    oss <<"Set integer speed value from 1 to 5\n";
                    Close(oss.str());
                }
                break;
            }
            case 'a':
            {
                if (strstr(optarg,"left") != NULL)
                {
                    _Config.Mode = LEFT;
                }
                else if (strstr(optarg,"center") != NULL)
                {
                    _Config.Mode = CENTER;
                }
                else if (strstr(optarg,"right") != NULL)
                {
                    _Config.Mode = RIGHT;
                }
                else
                {
                    oss << optarg <<" is invalid value for -a option\n";
                    Close(oss.str());
                }
                break;
            }

            case 0x102:;
            {
                char *pos = strtok(optarg ,", '");
                _Config.Y = (int32_t) strtol(pos, NULL, 10);
                if ( _Config.Y < 0)
                {
                    oss << _Config.Y <<" is invalid value for --win option\n";
                    Close(oss.str());
                }

                pos = strtok (NULL,", ");
                if(pos!=NULL)
                {
                    _Config.Height = (int32_t) strtol(pos, NULL, 10);
                }

                if ( _Config.Height < 0 ||
                     _Config.Height > 1000 )
                {
                    oss << _Config.Height <<" is invalid value for --win option\n";
                    Close(oss.str());
                }
                break;
            }
            case 0x103:;
            {
                do
                {
                    std::ostringstream _oss ;
                    _oss << optarg;
                    std::string s = _oss.str();
					std::string fileName;

                    // first file
                    size_t pos;
					pos  = s.find(",");
                    fileName = s.substr(0, pos);
                    if ( access(fileName.c_str(),R_OK) != 0 )
                    {
                        oss << "Cannot find "<< fileName <<"\n";
                        Close(oss.str());
                    }

					_Config.FileList.push_back(fileName);

                    if (pos == std::string::npos)
                        break; //break from while

                    //CaptionImageFile1
                    s = s.substr(pos+1);
                    pos = s.find(",");
                    fileName = s.substr(0,pos);

                    if (access(fileName.c_str(),R_OK) != 0 )
                    {
                        oss << "Cannot find "<< fileName <<"\n";
                        Close(oss.str());
                    }

					_Config.FileList.push_back(fileName);
                    if (pos == std::string::npos)
                        break; //break from while

                    //CaptionImageFile2
                    s = s.substr(pos+1);
                    pos = s.find(",");
					fileName = s.substr(0,pos);

                    if (access(fileName.c_str(), R_OK) != 0 )
                    {
                        oss << "Cannot find " << fileName <<"\n";
                        Close(oss.str());
                    }

					_Config.FileList.push_back(fileName);

                    if (pos == std::string::npos)
                        break; //break from while

                    //CaptionImageFile3
                    s = s.substr(pos+1);
                    pos = s.find(",");
					fileName = s.substr(0,pos);


                    if (access(fileName.c_str(), R_OK) != 0 )
                    {
                        oss << "Cannot find "<< fileName <<"\n";
                        Close(oss.str());
                    }

					_Config.FileList.push_back(fileName);

                    if (pos != std::string::npos)
                    {
                        oss << argv[0] << "supports only 4 image file"<<"\n";
                        Close(oss.str());
                    }
                }while(false);

                break;
            }
            case 0x104:;
            {
                _Config.Layer = (int32_t) strtol(optarg, NULL, 10);

                if ( _Config.Layer < -99 ||
                     _Config.Layer > 99 )
                {
                    oss << _Config.Layer <<" is invalid value for --layer option\n";
                    Close(oss.str());
                }
                break;
            }
        }
    }

    if (_Config.FileList.empty())
    {
		oss << "You should type image file name with --file option";
        Close(oss.str());
    }

	if (_Config.Height == 0)
	{
		_Config.Height = 256;
	}

	InitGraphics(_Config.Layer);

	if (_Config.Verbose)
		std::cout << "InitGraphics done Layer : "<< _Config.Layer <<"\n";

	uint32_t success = graphics_get_display_size(0, &_Config.Display.Width, &_Config.Display.Height);

	if (success < 0)
    {
		oss << "[graphics_get_display_size] Failed \n";
		Close(oss.str());
    }

	_Config.ID.Clear();
	_Config.ID.program = LoadShaders( "/usr/local/bin/vertshader.glsl", "/usr/local/bin/fragshader.glsl" );

	_Config.ID.vertex = glGetAttribLocation(_Config.ID.program, "vertex");
	_Config.ID.texture0 = glGetAttribLocation(_Config.ID.program, "texture_loc0");
	_Config.ID.texture1 = glGetAttribLocation(_Config.ID.program, "texture_loc1");
	_Config.ID.texture2 = glGetAttribLocation(_Config.ID.program, "texture_loc2");
	_Config.ID.texture3 = glGetAttribLocation(_Config.ID.program, "texture_loc3");

	_Config.ID.StartLocation = glGetUniformLocation(_Config.ID.program, "startLocation");   //uniform_vector to start scrolling wherever user wanted
	_Config.ID.MappedTextures = glGetUniformLocation(_Config.ID.program, "umaps");
	_Config.ID.tex0Matrix = glGetUniformLocation(_Config.ID.program, "tex0Matrix");
	_Config.ID.tex1Matrix = glGetUniformLocation(_Config.ID.program, "tex1Matrix");
	_Config.ID.tex2Matrix = glGetUniformLocation(_Config.ID.program, "tex2Matrix");
	_Config.ID.tex3Matrix = glGetUniformLocation(_Config.ID.program, "tex3Matrix");
	_Config.ID.tex0Left = glGetUniformLocation(_Config.ID.program, "leftMarginTexture0");
	_Config.ID.tex1Left = glGetUniformLocation(_Config.ID.program, "leftMarginTexture1");
	_Config.ID.tex2Left = glGetUniformLocation(_Config.ID.program, "leftMarginTexture2");
	_Config.ID.tex3Left = glGetUniformLocation(_Config.ID.program, "leftMarginTexture3");
	_Config.ID.Translation = glGetUniformLocation(_Config.ID.program, "trans");

	if (_Config.Verbose)
	{
		std::cout << "Screen info w: "<< _Config.Display.Width<< " h: "<<_Config.Display.Height <<std::endl;
		std::cout
			<<"---------------------------------------------------------\n"
			<< "Assigned ID List \n"
			<<"_Config.ID.program : "  <<	_Config.ID.program << "\n"
			<<"_Config.ID.vertex : "   <<	_Config.ID.vertex << "\n"
			<<"_Config.ID.texture0 : " <<	_Config.ID.texture0 << "\n"
			<<"_Config.ID.texture1 : " <<	_Config.ID.texture1 << "\n"
			<<"_Config.ID.texture2 : " <<	_Config.ID.texture2 << "\n"
			<<"_Config.ID.texture3 : " <<	_Config.ID.texture3 << "\n"
			<<"_Config.ID.StartLocation : " <<	_Config.ID.StartLocation << "\n"
			<<"_Config.ID.MappedTextures : " <<	_Config.ID.MappedTextures << "\n"
			<<"_Config.ID.tex0Matrix : " <<	_Config.ID.tex0Matrix << "\n"
			<<"_Config.ID.tex1Matrix : " <<	_Config.ID.tex1Matrix << "\n"
			<<"_Config.ID.tex2Matrix : " <<	_Config.ID.tex2Matrix << "\n"
			<<"_Config.ID.tex3Matrix : " <<	_Config.ID.tex3Matrix << "\n"
			<<"_Config.ID.tex0Left : " <<	_Config.ID.tex0Left << "\n"
			<<"_Config.ID.tex1Left : " <<	_Config.ID.tex1Left << "\n"
			<<"_Config.ID.tex2Left : " <<	_Config.ID.tex2Left << "\n"
			<<"_Config.ID.tex3Left : " <<	_Config.ID.tex3Left << "\n"
			<<"_Config.ID.Translation : " <<	_Config.ID.Translation << "\n"
			<<"---------------------------------------------------------\n";
	}

	LoadFiles();
	InitImageScroller();

	if (_Config.Mode == SCROLLING)
	{
		while (DoRender())
		{
			usleep(1000*1000/_Config.Fps);
		}
	}
	//LEFT/CENTER/RIGHT

	else
	{
		while(DoRender())
		{
			getchar();
		}while(1);
	}

	Close();
}

void InitImageScroller()
{
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  	glClear( GL_COLOR_BUFFER_BIT );
	glUseProgram(_Config.ID.program);
	ActivateTextures();

	GLfloat _attachedWidth = 0.0f;

	int index = 0;
	for (auto it = _ImageInfo.TextureList.begin();
		it != _ImageInfo.TextureList.end();
		++it)
	{
		it->CalculateScaleValues();
		if (index > 0)
		{
			it->LeftMargin = _attachedWidth;
		}
		_attachedWidth += it->ScaledWidth;
		GLfloat mar =  it->LeftMargin /(GLfloat) _Config.Display.Width;

		if (_Config.Verbose)
		{
			std::cout << "---------TextureInfo------------\n"
			<< "ImageID : " << it->TextureID << "\n"
			<< "MatirixID : " << it->MatrixID << "\n"
			<< "w : " << it->Width << "\n"
			<< "h : " << it->Height<< "\n"
			<< "ScaledWidth : "<< it->ScaledWidth << "\n"
			<< "ScaledHeight : "<< it->ScaledHeight << "\n"
			<< "LeftMargin : "<< it->LeftMargin << "\n"
			<< "Margin Calculated : "<< mar << "\n";
		}

		glUniformMatrix4fv(it->MatrixID, 1, GL_FALSE, it->Matrix);
		glUniform1f(it->LeftMarginID, mar);

		index++;
	}
	_ImageInfo.CalculateTotalWidth(); //GLCoord_TotalWidth
	_ImageInfo.UpdateLocations(); //Xconverted,Yconverted,TurningPoint

	glUniform4f(_Config.ID.StartLocation, _Config.Xconverted,_Config.Yconverted,0,0);

	if (_Config.Verbose)
	{
		std::cout << "---------StartLocation------------\n"
				<< "StartLocation : x:" << _Config.Xconverted << "\n"
				<< "StartLocation : y:" << _Config.Yconverted << "\n"
				<< "GLCoord_TotalWidth : " << _ImageInfo.GLCoord_TotalWidth << "\n"
				<< "TurningPoint : " << _Config.TurningPoint << "\n";
	}

	glEnableVertexAttribArray(_Config.ID.vertex);
	glEnableVertexAttribArray(_Config.ID.texture0);
	glEnableVertexAttribArray(_Config.ID.texture1);
	glEnableVertexAttribArray(_Config.ID.texture2);
	glEnableVertexAttribArray(_Config.ID.texture3);
	glVertexAttribPointer(
        _Config.ID.texture0,
        VERTEX_TEXCOORD0_SIZE,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        sizeof(GLfloat)* VERTEX_TEXCOORD0_SIZE,                  // stride
    	  texture0_buffer_data
		 );
	glVertexAttribPointer(
        _Config.ID.texture1,
        VERTEX_TEXCOORD1_SIZE,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        VERTEX_TEXCOORD1_SIZE * sizeof(GLfloat),                  // stride
        texture1_buffer_data
		 );
	glVertexAttribPointer(
        _Config.ID.texture2,
        VERTEX_TEXCOORD2_SIZE,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        VERTEX_TEXCOORD2_SIZE * sizeof(GLfloat),                  // stride
        texture2_buffer_data
		 );
	glVertexAttribPointer(
        _Config.ID.texture3,
        VERTEX_TEXCOORD3_SIZE,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        VERTEX_TEXCOORD3_SIZE * sizeof(GLfloat),        // stride
        texture3_buffer_data
		 );
	glVertexAttribPointer(
        _Config.ID.vertex, //vertexPosition_modelspaceID, // The attribute we want to _Configure
        VERTEX_POS_SIZE,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        sizeof(GLfloat)* VERTEX_POS_SIZE,
        g_vertex_buffer_data
		 );

	 glViewport(0, 0, _Config.Display.Width, _Config.Display.Height);
}


void ActivateTextures()
{
	int i = 0;

	for (auto it = _ImageInfo.TextureList.begin();
		it != _ImageInfo.TextureList.end();
		++it)
	{
		glActiveTexture(GL_TEXTURE0 + i++);
		glBindTexture(GL_TEXTURE_2D, it->TextureID);
		if (_Config.Verbose)
			std::cout << "Binding TextureID : "<< it->TextureID<<"\n";
	}
	glUniform1iv(_Config.ID.MappedTextures, 4, textureUnits);
}

bool DoRender()
{
	if (_Config.ViewPoint)
		std::cout << "RenderPoint : "<< _Config.RenderPoint<<"\n";

	if (_Config.RenderPoint > _Config.TurningPoint)
	{
		_Config.RenderPoint = 0;
	}
	glUniform4f(_Config.ID.Translation, -_Config.RenderPoint,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT );
	glDrawArrays(GL_TRIANGLES, 0, 6*(_Config.NumberOfTextures));
	updateScreen();

	_Config.RenderPoint += (GLfloat)_Config.Speed * (GLfloat)OFFSET;

	return true;
}

void LoadFiles(void)
{
	FreeImage_Initialise(1);

	std::string fileName;
	int width, height, scan_pitch, image_size, index = 0;
	GLuint assignedTextureID;
	FIBITMAP * bitmap;

	for (auto it = _Config.FileList.begin(); it != _Config.FileList.end(); ++it)
	{
		fileName = *it;
		bitmap = FreeImage_Load(FIF_PNG, fileName.c_str(), 0);

		width = FreeImage_GetWidth(bitmap);
		height = FreeImage_GetHeight(bitmap);

		scan_pitch = FreeImage_GetPitch(bitmap);
		image_size = scan_pitch * height;

		if(FreeImage_GetColorType(bitmap) == FIC_RGB)
		{
		//	fprintf(stderr,"[Render_png] FIC_RGB\n");
		}
		if( FreeImage_GetColorType(bitmap) == FIC_RGBALPHA)
		{
			//fprintf(stderr,"[Render_png] FIC_RGBALPHA\n");
		}

		FREE_IMAGE_TYPE image_type = FreeImage_GetImageType(bitmap);
		if(image_type == FIT_RGBF)
		{
		//	fprintf(stderr,"[Render_png] FIT_RGBF\n");
		}
		else if(FIT_BITMAP)
		{
		//	fprintf(stderr,"[Render_png] FIT_BITMAP\n");
		}
		//fprintf(stderr,"[Render_png] Is Trans ? %d \n", FreeImage_IsTransparent(bitmap));

		unsigned char * data = new unsigned char [image_size*4];

		glGenTextures(1, &assignedTextureID);
		glBindTexture(GL_TEXTURE_2D, assignedTextureID);

		data = (unsigned char *) FreeImage_GetBits(bitmap);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		FreeImage_Unload(bitmap);

		if (_Config.Verbose)
			fprintf(stdout,"[LoadFiles] Returning Texture ID :%d\n", assignedTextureID);

		TextureInfo textureInfo;

		textureInfo.TextureID 	= assignedTextureID;
		textureInfo.Width 		= width;
		textureInfo.Height		= height;

		switch(index++)
		{
			case 0:
			{
				textureInfo.MatrixID = _Config.ID.tex0Matrix;
				textureInfo.LeftMarginID = _Config.ID.tex0Left;
				break;
			}
			case 1:
			{
				textureInfo.MatrixID = _Config.ID.tex1Matrix;
				textureInfo.LeftMarginID = _Config.ID.tex1Left;
				break;
			}
			case 2:
			{
				textureInfo.MatrixID = _Config.ID.tex2Matrix;
				textureInfo.LeftMarginID = _Config.ID.tex2Left;
				break;
			}
			case 3:
			{
				textureInfo.MatrixID = _Config.ID.tex3Matrix;
				textureInfo.LeftMarginID = _Config.ID.tex3Left;
				break;
			}
		}

		_ImageInfo.TextureList.push_back(textureInfo);
	}

	_Config.NumberOfTextures = _ImageInfo.TextureList.size();

	FreeImage_DeInitialise();
}

void Close(std::string errorMsg)
{
	std::string cmd;
	cmd = "echo " + errorMsg + " > /var/run/ImageScroller.log";
	if (system(cmd.c_str()) == -1)
		exit(-1);
	GLuint  tex[] = {1,2,3,4};
	glDeleteProgram(_Config.ID.program);
	glDeleteTextures(4,tex);
	exit(-1);
}

void printUsage(const char *progr)
{
	printf("\n");
	printf("USAGE: %s [OPTIONS] --file fileName0,~,fileName3 \n", progr);
	printf("OPTIONS:\n\n");
	printf("    -h  --help                   Print this help\n");
	printf("    -v  --version                Show version info\n");
    printf("    -g  --debug                  Print general log through stdout\n");
	printf("    -p  --point                  Print point log through stdout\n");
    printf("    -s  --speed                  Set a speed value of Caption to scroll.(Integer 1 ~ 5)\n");
    printf("    -a  --align                  Set horizontal alignment of Caption.\n");
    printf("                                 Option -s and -a cannot be used together\n");
	printf("                                 If there is no -s and -a option, default mode '-s 2' will be set\n");
	printf("        --win      y,height      Set y coordinate, height\n");
    printf("        --layer    -99~99        Set layer value\n");
	printf("\n");
}

static void printVersion()
{
	printf("Build date: %s\n", __DATE__);
}
