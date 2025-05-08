#pragma once

#include <map>
#include <stack>
#include <vector>

#include "Vector3D.h"

struct TextureData
{
	virtual ~TextureData() {}; // In case you want to extend Texture Data

	GLint width; // Image Width
	GLint height; // Image Height
	GLubyte *data; // Image Data
	GLenum type; // Image Type (GL_RGB, etc)
	GLint bpp; // Image Color Depth (24 bits per pixel, 32, etc)
	GLuint id; // OpenGL Texture ID
	bool mipmapped; // Was this texture generated via mipmaps?
};


class Texture
{
public:

	Texture();
	~Texture();

	// Render
	// This creates our textures for OpenGL
	// Doesn't work for Advanced mode, because we don't create OpenGL textures there
	static void Build();

	// Standard
	// Take note: All of the following functions uses Texture's Internal ID.
	int AddTexture(const std::string& file, const bool mipmap = false);
	void RemoveTexture(const int what);
	void ClearAllTextures();

	// Advanced
	int AddTexture(TextureData*& texture, const std::string& file); // pass in empty pointer or it will be overridden
	void SetTextureData(const int index, TextureData* texture); // shallow copy

	// Retrieve
	TextureData* operator[](const int index) const;
	TextureData* GetTexData(const int index) const;
	int Size() const;

	// Misc
	void ApplyTexture(const int& index);
	void DrawXY(const int& index, const int& frame = 0, float width = 0);


private:

	// Locked
	Texture(const Texture&);
	const Texture& operator=(const Texture&);

	// Internal Resource Allocator
	void Insert(const std::string& file, TextureData*& d);
	void Delete(TextureData*& d); // returns true if no copies of texture is left
	TextureData* Retrieve(const std::string& file); // returns NULL if texture doesn't exist

	// Tools
	int AddTexture(const std::string& file, TextureData*& texture); // returns texture index
	std::string Lowercase(const std::string& what) const;
	std::string Trim(const std::string& x, const bool do_middle = true, const std::string& symbol = " \f\n\r\t\v") const;


private:

	// Texture Manager Local Identity
	std::stack <int> mNextId;
	std :: vector <TextureData*> mTextureList;

	// States
	static bool mLocked;
	static bool mFirstTime;

	// Resource Allocator
	// All these variables are pointers because something weird is going on with the compiler
	// It deletes static variables before calling class destructors
	// We don't bother freeing up these static variables because it will
	// be taken of care by the operating system on program exit anyway
	static std::map< std::string, TextureData* > *mpList;
	static std::map< TextureData*, int > *mpCount;

	// Creation List
	static std::vector< TextureData* > *mpBuildList;

	//TextureData* operator[](const int index) const;
	//int Size() const;
};

// Retrieve functions are more optimized
//TextureData* Texture::operator[](const int index) const;
//int Texture::Size() const;

/* How to Load and Use Texture
*******************************
Texture t;
GLuint id = t.Add("test.png");
glBindTexture(GL_TEXTURE_2D, id);
*/


/* How to Set and Use Your Own Texture Data
********************************************
struct MyData : TextureData
{
	std::string name;
};

Texture t;
int id = t.AddTexture("skydome.png");

MyData *td = new MyData();

td = (MyData*)t[x]; // copy existing data
td->name = "Skydome"; // set extra data

t.SetTextureData(x, td);

glBindTexture(GL_TEXTURE_2D, t[x].id);
*/


/* How to Use Other Texture Modes
**********************************
Texture t;
TextureData *texture;
int id = t.AddTexture(texture, "height.png");

GLint channels = (texture->bpp == 32 ? 4 : 3); // Get additional information about texture
texture->mipmapped = true;

texture->id = 0;
glGenTextures(1, &texture->id);
glBindTexture(GL_TEXTURE_1D, texture->id);

glTexParameterf(GL_TEXTURE_1D, ...);
glTexImage1D( ... );
t.SetTextureData(index, texture); // Insert updated information into the index

glBindTexture(GL_TEXTURE_1D, t[x].id);
*/