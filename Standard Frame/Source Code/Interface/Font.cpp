
// Font.cpp
#include "stdafx.h"
#include "Font.h"


Font * g_Font;


Font::Font(const std::string& filename, const float& spacing)
	:	mFontSpace(spacing)
{
	// Load our font texture
	mTexManager.AddTexture(filename, false);

	// We no longer use display lists for this, because it does
	// not give a significant speed increase, nor does it give
	// any convenience when loading textures
}

Font::~Font()
{
	mTexManager.ClearAllTextures();
}


int Font::AddFaceTex(const std::string& filename)
{
	return mTexManager.AddTexture(filename, false);
}

void Font::Print(const std::string& what, const float x, const float y, const float s, const float w, const int id) const
{
	// Save current matrices
	//glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	//glLoadIdentity();
	//gluOrtho2D(0, 800, 0, 600);

	//glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	//glLoadIdentity();

	// Wrap string if necessary
	std::vector< std::string > v;

	if( w != 0.0f )		v = SplitStrings(what, s, w);
	else				v.push_back(what);

	// Apply requested coordinates
	glTranslatef(x, y, 0);
	glScalef(s, s, 1.0f);

	// Our default font texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTexManager[0]->id);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// Multi-texture if necessary
	// Zero is our main font texture
	if( id > 0 && id < mTexManager.Size() )
	{
		glActiveTexture(GL_TEXTURE1);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, mTexManager[id]->id);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	}

	// Render our strings
	std::vector< std::string >::iterator it = v.begin();
	std::vector< std::string >::iterator end = v.end();
	for( ; it != end; ++it )
	{
		glPushMatrix();

		int size = (int)it->size();
		for( int i=0; i<size; ++i )
			DrawChar( (*it)[i] );

		glPopMatrix();

		// Move one line down
		glTranslatef(0, -16, 0);
	}

	// Reset texture
	glActiveTexture(GL_TEXTURE1);
	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);

	// Restore matrices
	//glMatrixMode(GL_PROJECTION);
	//glPopMatrix();

	//glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
}


std::vector< std::string > Font::SplitStrings(const std::string& s, const float& scale, const float& width) const
{
	int size = (int)s.size();
	std::vector< std::string > v;
	float line = scale * mFontSpace; // Each character size
	float w = width - line * 2.0f; // The width to compare with

	// String fits in width perfectly
	// Don't let width be smaller than 5 characters too
	if( size * line < w || w < line * 5.0f )
	{
		v.push_back(s);
		return v;
	}

	// Split strings accounting for their spaces
	// This counter is used to prevent the loop from looping forever due to unable to split text (no spaces)
	int counter = (int)(size * line / w + 0.5f) * 2; // Round up
	int last_pos = 0;
	for( int i=0; i<size; ++i )
	{
		if( (i-last_pos) * line < w )		continue;

		// Get our raw cutted string
		std::string newstr = s.substr(last_pos, i-last_pos+1);
		int cutpos = (int)newstr.find_last_of(' ');

		// Cut our string further aligning with the space
		newstr = newstr.substr(0, cutpos+1);
		v.push_back(newstr);

		// Record our new position
		i = last_pos + cutpos;
		last_pos = i+1;

		// Check if counter goes out of scope
		if( --counter != 0 )				continue;

		// Manually escape loop
		v.resize(0);
		last_pos = 0;
		break;
	}

	// Don't forget to include leftovers
	if( last_pos != size-1 )
		v.push_back( s.substr(last_pos, size) );

	return v;
}

void Font::DrawChar(char x) const
{
	// Get position of current character
	float cx = (float)(x % 16) / 16.0f;
	float cy = (float)(x / 16) / 16.0f;

	glBegin(GL_QUADS);

	// Bottom left
	glMultiTexCoord2f(GL_TEXTURE0, cx, 1.0f-cy-0.0625f);
	glMultiTexCoord2f(GL_TEXTURE1, 0, 1);
	glVertex2i(0, 0);

	// Bottom right
	glMultiTexCoord2f(GL_TEXTURE0, cx+0.0625f, 1.0f-cy-0.0625f);
	glMultiTexCoord2f(GL_TEXTURE1, 1, 1);
	glVertex2i(16, 0);

	// Top right
	glMultiTexCoord2f(GL_TEXTURE0, cx+0.0625f, 1.0f-cy-0.001f);
	glMultiTexCoord2f(GL_TEXTURE1, 1, 0);
	glVertex2i(16, 16);

	// Top left
	glMultiTexCoord2f(GL_TEXTURE0, cx, 1.0f-cy-0.001f);
	glMultiTexCoord2f(GL_TEXTURE1, 0, 0);
	glVertex2i(0, 16);

	glEnd();
	glTranslatef(mFontSpace, 0, 0); // Move to right of character
}