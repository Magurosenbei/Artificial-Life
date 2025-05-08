#include "stdafx.h"
#include "texture.h"

#include "il/ilu.h"

// Initialize Static Members
bool Texture::mLocked = false;
bool Texture::mFirstTime = true;
std::map< std::string, TextureData* > *Texture::mpList = new std::map< std::string, TextureData* >;
std::map< TextureData*, int > *Texture::mpCount = new std::map< TextureData*, int >;
std::vector< TextureData* > *Texture::mpBuildList = new std::vector< TextureData* >;


Texture::Texture()
{
	// Initialize our internal texture loader
	if( mFirstTime )
	{
		ilInit();
		iluInit();

		// Flip images to the OpenGL coordinate system
		ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
		ilEnable(IL_ORIGIN_SET);

		mFirstTime = false;
	}

	// Initalize next id stack starting with 0
	mNextId.push(0);
}

Texture::~Texture()
{
	ClearAllTextures();
}


void Texture::Build()
{
	// In case we add textures while building
	while( mLocked ) {}
	mLocked = true;

	std::vector< TextureData* >::iterator it = mpBuildList->begin();
	std::vector< TextureData* >::iterator end = mpBuildList->end();

	// Iterate over our build list to build our textures
	for( ; it != end; ++it )
	{
		// Get additional information about (*it)
		GLint channels = ((*it)->bpp == 32 ? 4 : 3);

		// Build texture in OpenGL with texture data provided
		(*it)->id = 0;
		glGenTextures(1, &(*it)->id);
		ASSERT(  (*it)->id != 0  , "Texture: OpenGL context not loaded!");
		glBindTexture(GL_TEXTURE_2D, (*it)->id);

		// Standard creation
		if( !(*it)->mipmapped )
		{
			// Without mipmaps
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexImage2D(  GL_TEXTURE_2D, 0, channels, (*it)->width,  (*it)->height,
				0, (*it)->type, GL_UNSIGNED_BYTE, (*it)->data  );
		}
		else
		{
			// Mipmaps
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			gluBuild2DMipmaps(  GL_TEXTURE_2D, channels, (*it)->width,  (*it)->height,
				(*it)->type, GL_UNSIGNED_BYTE, (*it)->data  );
		}
		free((*it)->data);
		(*it)->data = NULL;
	}
	// Don't clear our allocated memory
	// We want it to create other textures if necessary
	mpBuildList->resize(0);
	mLocked = false;
}


int Texture::AddTexture(const std::string& file, const bool mipmaps)
{
	// Do a check if our texture exists
	bool build = ( Retrieve(file) == NULL ? true : false);

	// Load texture using our sister function
	TextureData *texture = NULL;
	int index = AddTexture(texture, file);

	// Don't build our textures
	if( !build )	return index;

	// Store additional information about texture
	texture->mipmapped = mipmaps;
	texture->id = 0;

	// Put into build list for rendering
	// Check if rendering anything, in case we add to build list while creating textures
	while( mLocked ) {}
	mLocked = true;

	mpBuildList->push_back(texture);
	mLocked = false;
	return index;
}

void Texture::RemoveTexture(const int index)
{
	// Find out of bounds error
	ASSERT(  (int)( mTextureList.size() ) > index  , "Texture: Invalid index!");

	// Remove texture data from our map if necessary
	Delete( mTextureList[index] );

	// Make current id available
	mNextId.push(index);
}

void Texture::ClearAllTextures()
{
	int end = (int)( mTextureList.size() ) - 1;

	// Go through all image data, manually delete it to prevent memory leak
	for( int i = 0; i <= end; ++i )
		RemoveTexture(i);
}


int Texture::AddTexture(TextureData*& texture, const std::string& file)
{
	// Don't load our texture if it already exists in our map
	texture = Retrieve(file);
	if( texture != NULL )
		return AddTexture(file, texture);

	// Create our texture
	texture = new TextureData();

	// Use DevIL to load the images
	ILuint iid = 0;
	ilGenImages(1, &iid);

	ilBindImage(iid);
	VERIFY(  ilLoadImage( file.c_str() ) == IL_TRUE  , "Texture: Unable to load file!");

	// Convert every colour component to unsigned byte
	GLint channels = 0;
	if(  ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL) == 3  )
	{
		ASSERT(  ilConvertImage(IL_RGB, IL_UNSIGNED_BYTE) == TRUE  , "Texture: Unable to load file!");
		channels = 3;
	}
	else if(  ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL) == 4  )
	{
		ASSERT(  ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE) == TRUE  , "Texture: Unable to load file!");
		channels = 4;
	}
	else
	{
		ASSERT(0, "Texture: Unable to load file!");
		return 0;
	}

	// Convert DevIL's internal format to OpenGL
	texture->id = 0;
	texture->data = 0;
	texture->width = ilGetInteger(IL_IMAGE_WIDTH);
	texture->height = ilGetInteger(IL_IMAGE_HEIGHT);
	texture->bpp = (channels == 3 ? 24 : 32);
	texture->type = (ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL) == 3 ? GL_RGB : GL_RGBA);
	texture->mipmapped = false;

	// Do a deep copy for the image data
	size_t imgsize = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);
	texture->data = (GLubyte*)(  malloc(imgsize)  );

	ASSERT(  texture->data != 0  , "Texture: Not enough memory!");

	for( size_t i = 0; i<imgsize; ++i )
		*(texture->data+i) = *(ilGetData()+i);

	ilDeleteImages(1, &iid);
	return AddTexture(file, texture);
}

void Texture::SetTextureData(const int index, TextureData* texture)
{
	ASSERT(  (int)( mTextureList.size() ) > index  , "Texture: Subscript out of range!");
	ASSERT(  texture != NULL  , "Texture: Null texture recieved!");

	// Prevent self-assignment
	if( mTextureList[index] == texture )	return;

	// Free up old texture data
	// This is because we are changing the pointer to a new location

	// We also free the old texture's image data if it is not in the new texture data
	// Generally, we usually only append extra data to our new texture data, so this is not needed
	// But for extensibility, the image data is deleted to allow other image loaders to use this class as well
	if( mTextureList[index]->data != texture->data )	free(mTextureList[index]->data);
	if( mTextureList[index] != NULL )					delete mTextureList[index];
	mTextureList[index] = texture;
}


void Texture::ApplyTexture(const int& index)
{
	glBindTexture(GL_TEXTURE_2D, mTextureList[index]->id);
}

void Texture::DrawXY(const int& index, const int& frame, float width)
{
	if(width == 0)
		width = float(mTextureList[index]->width);
	glBindTexture(GL_TEXTURE_2D, mTextureList[index]->id);

	float SpritePiece = width / mTextureList[index]->width;
	float SpritePiecePos = SpritePiece * frame;
	float H_Value = mTextureList[index]->height / 2.0f;
	float W_Value = width / 2.0f;

	glBegin(GL_QUADS);		
		glTexCoord2f(SpritePiecePos, 0);				glVertex3f(-W_Value, -H_Value, 0.0f);	
		glTexCoord2f(SpritePiecePos + SpritePiece, 0);	glVertex3f(W_Value, -H_Value, 0.0f);	
		glTexCoord2f(SpritePiecePos + SpritePiece, 1);	glVertex3f(W_Value, H_Value, 0.0f);		
		glTexCoord2f(SpritePiecePos, 1);				glVertex3f(-W_Value, H_Value, 0.0f);	
	glEnd();
}


void Texture::Insert(const std::string& filename, TextureData*& d)
{
	// Query our map for our texture to check if it exists
	// Increment it if it exists
	TextureData *dd = Retrieve(filename);
	if( dd != NULL )
	{
		++mpCount->find(dd)->second; d = dd;
	}

	// Just in case, we don't want to add NULL data
	// Since our texture does not exist, we register our texture into our map
	else if( d != NULL )
	{
		// Parse our string to remove any mistakes
		std::string f = Trim( Lowercase(filename), true, "\\/" );
		mpList->insert( std::pair< std::string, TextureData* >(f, d) );
		mpCount->insert( std::pair< TextureData*, int>(d, 1) );
	}
}

void Texture::Delete(TextureData*& d)
{
	// Delete textures if it no longer exists on any texture loader
	if( d != NULL && --mpCount->find(d)->second == 0 )
	{
		// Remove our texture data from our map
		mpCount->erase( mpCount->find(d) );

		std::map< std::string, TextureData* >::iterator it = mpList->begin();
		std::map< std::string, TextureData* >::iterator end = mpList->end();

		for( ; it != end; ++it )
		{
			if( it->second != d )	continue;
			mpList->erase(it);
			break;
		}

		// Remove our texture data from build list if applicable
		std::vector< TextureData* >::iterator vit = mpBuildList->begin();
		std::vector< TextureData* >::iterator vend = mpBuildList->end();

		for( ; vit != vend; ++vit )
		{
			if( (*vit) != d )	continue;
			mpBuildList->erase(vit);
			break;
		}
		// Free up texture memory
		glDeleteTextures(1, &d->id);

		// Free up stored image data
		// There is no destructor for GLubyte*, so manually destroy it
		//if(d->data != NULL)	free(d->data);
		d->data = NULL;
	}

	// Don't let wild pointers roam
	//if(d != NULL)	delete d;
	d = NULL;
}

TextureData* Texture::Retrieve(const std::string& filename)
{
	std::string f = Trim( Lowercase(filename), true, "\\/" );
	if( mpList->count(f) == 0 )	return NULL;
	else						return mpList->find(f)->second;
}


int Texture::AddTexture(const std::string& file, TextureData*& texture)
{
	// Register texture with the index
	int index = mNextId.top();
	size_t index_size = mNextId.size();

	if( index_size == 1 )
	{
		// Insert a new copy into the index
		if( texture != NULL )	mTextureList.push_back(texture);
		else					mTextureList.push_back( Retrieve(file) );

		// Increment index
		++mNextId.top();
	}
	else
	{
		// Do a shallow copy
		if( texture != NULL )	mTextureList[index] = texture;
		else					mTextureList[index] = Retrieve(file);

		// Remove used index
		mNextId.pop();
	}

	Insert(file, texture);
	return index;
}

std::string Texture::Lowercase(const std::string& what) const
{
	std::string tmp = what;
	int size = (int)tmp.size();

	// Change all uppercase letters to lowercase
	for( int i=0; i<size; ++i )
	{
		if(tmp[i] >= 'A' && tmp[i] <= 'Z')
			tmp[i] += ('a' - 'A');
	}

	return tmp;
}

std::string Texture::Trim(const std::string& x, const bool do_middle, const std::string& symbol) const
{
	std::string copy = x, tmp = "";
	int count = 0, i = 0;
	bool flag = false;

	// Remove trailing whitespaces
	count = (int)( copy.find_last_not_of(symbol) );
	copy.erase(count+1);

	count = (int)( copy.find_first_not_of(symbol) );
	copy.erase(0, count);

	// Return the copy instead of tmp because tmp was not touched
	if( !do_middle )
		return copy;

	// Remove double whitespaces in middle of string, if requested
	count = (int)( copy.size() );
	for( i=0; i < count; ++i )
	{
		if(  flag && isspace( copy[i] )  )
			continue;

		flag = false;
		tmp += copy[i];

		if(  isspace( copy[i] )  )
			flag = true;
	}
	
	return tmp;
}

TextureData* Texture::operator[](const int index) const
{
	ASSERT(  (int)( mTextureList.size() ) > index  , "Texture: Subscript out of range!");
	ASSERT(  mTextureList[index] != NULL  , "Texture: Non-existent texture!");
	return mTextureList[index];
}

TextureData* Texture :: GetTexData(const int index) const
{
	ASSERT(  (int)( mTextureList.size() ) > index  , "Texture: Subscript out of range!");
	ASSERT(  mTextureList[index] != NULL  , "Texture: Non-existent texture!");
	return mTextureList[index];
}

int Texture::Size() const
{
	return (int)mTextureList.size();
}
