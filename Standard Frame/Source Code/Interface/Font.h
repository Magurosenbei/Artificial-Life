#pragma once

/* Scroll all the way down for some in-depth tutorials. */


class Font
{
public:

	Font(const std::string& filename, const float& spacing = 10.0f);
	~Font();

	int AddFaceTex(const std::string& filename);
	void Print(
		const std::string& what,					// Message
		const float x, const float y,				// Text position
		const float scale = 1.0f,					// Scale factor
		const float width = 0.0f,					// Wrap width
		const int id = 0) const;					// Multi-texture id (if any, zero is none)


private:

	std::vector< std::string > SplitStrings(const std::string& s, const float& scale, const float& width) const;
	void DrawChar(char x) const;

	Texture mTexManager;
	const float mFontSpace;
};

extern Font * g_Font;


/* How to Create and Use Font
******************************
Font f("font.png");
f.Print("Hello World!"); // draw font scale 1 at (0,0)
*/


/* How to Create and Use Personalized Font
*******************************************
Font f("font.png");
int brickfont = f.AddFaceTex("brick.png");
f.Print("My Brick Font!", 50, 50, 0, 2, brickfont);
*/