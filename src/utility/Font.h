/*
 * Font.h
 *
 *  Created on: May 16, 2010
 *      Author: ilya
 */

#ifndef FONT_H_
#define FONT_H_

#include "Texture.h"

namespace Resources {

    class Font
    {
    public:
        struct CharData
        {
        	int x;
			int y;
			int width;
			int height;
			int xoffset;
			int yoffset;
			int xadvance;
			int page;
			int chnl;

            GLuint vertexVBO;
            GLuint texCoordVBO;
        };

    	std::string name;

    	int lineHeight;
        int whitespace;

    	ivec2 textureSize;
    	int numPages;
    	std::string texturePath;
    	Texture *texture;

    	CharData charData[256];


    	int getIntValue(std::string value);
    	std::string getStringValue(std::string value);

    	void createFontVBO(int id);

        GLfloat getLineWidth(const char *text, const char **textPos = NULL) const;

    public:
        Font(const char *path);
        virtual ~Font();

        void loadFont(const char *path);
        void readInfo(std::ifstream &file);
        void readCommon(std::ifstream &file);
        void readPageInfo(std::ifstream &file);
        void readCharInfo(std::ifstream &file);

        int getCharWidth(unsigned char character);
        int getCharHeight();

        void getPrintDimensions(GLfloat *width, GLfloat *height, const char *text) const;

        void print(GLfloat x, GLfloat y, const char * text) const;

        void printDebug() const;
    };
}

#endif /* FONT_H_ */
