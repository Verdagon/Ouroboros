/*
 * Font.cpp
 *
 *  Created on: May 16, 2010
 *      Author: ilya
 */

#include <cstring>
#include <string>
#include <stdint.h>
#include <cstdlib>
#include "Font.h"
#include "../include/physfs.h"
#include "../Logging.h"
#include <sstream>
#include "glHelpers.h"

const std::string FONT_TEX_PATH = "media/fonts/";

using namespace Resources;

Font::Font(const char *path) :
		name(""),
		lineHeight(0),
		whitespace(0),
		textureSize(0, 0),
		numPages(0),
		texturePath(""),
		texture(NULL)
{
    memset(charData, 0, sizeof(CharData) * 256);

	loadFont(path);
}

int Font::getIntValue(std::string value)
{
	std::string valueString = value.substr(value.find('=') + 1);
	log() << "Value: " << valueString << std::endl;

	int returnValue = 0;
	std::istringstream(valueString) >> returnValue;
	log() << "Value: " << returnValue << std::endl;

	return returnValue;
}

std::string Font::getStringValue(std::string value)
{
	std::string valueString = value.substr(value.find('=') + 1);
	log() << "Value: " << valueString << std::endl;

	valueString = valueString.substr(1, valueString.length() - 2);
	log() << "Value: " << valueString << std::endl;

	return valueString;
}

void Font::loadFont(const char *path)
{
    std::string param;
    std::string junk;   // Read junk from the file

    std::ifstream file(path);

    readInfo(file);
    readCommon(file);
    readPageInfo(file);
    readCharInfo(file);

    whitespace = charData[' '].xadvance;

	log() << "Done? " << file.eof() << std::endl;
}

void Font::readInfo(std::ifstream& file)
{
	log() << "Reading Font Info." << std::endl;

	//info face="EleganTech™" size=24 bold=0 italic=0 charset="" unicode=1 stretchH=100 smooth=1 aa=1 padding=0,0,0,0 spacing=1,1 outline=0
	//1    2                  3       4      5        6          7         8            9        10   11              12          13

    std::string param;
    std::string junk;   // Read junk from the file

	//info
    file >> junk;

	//face="EleganTech™"
    file >> param;
    name = getStringValue(param);

	//size=24
    file >> junk;

	//bold=0
    file >> junk;

	//italic=0
    file >> junk;

	//charset=""
    file >> junk;

	//unicode=1
    file >> junk;

	//stretchH=100
    file >> junk;

	//smooth=1
    file >> junk;

	//aa=1
    file >> junk;

	//padding=0,0,0,0
    file >> junk;

	//spacing=1,1
    file >> junk;

	//outline=0
    file >> junk;

	log() << std::endl;
}

void Font::readCommon(std::ifstream& file)
{
	log() << "Reading Common Info." << std::endl;

	//common lineHeight=24 base=20 scaleW=256 scaleH=256 pages=1 packed=0 alphaChnl=1 redChnl=0 greenChnl=0 blueChnl=0
	//1      2             3       4          5          6       7        8           9         10          11

    std::string param;
    std::string junk;   // Read junk from the file

	//common
    file >> junk;

	//lineHeight=24
    file >> param;
    lineHeight = getIntValue(param);

	//base=20
    file >> junk;

	//scaleW=256
    file >> param;
    textureSize.x = getIntValue(param);

	//scaleH=256
    file >> param;
    textureSize.y = getIntValue(param);

	//pages=1
    file >> param;
    numPages = getIntValue(param);
    assert(numPages == 1);

	//packed=0
    file >> junk;

	//alphaChnl=1
    file >> junk;

	//redChnl=0
    file >> junk;

	//greenChnl=0
    file >> junk;

	//blueChnl=0
    file >> junk;

	log() << std::endl;
}

void Font::readPageInfo(std::ifstream &file)
{
	//page id=0 file="elegantech24_0.png"

	log() << "Reading Page Info." << std::endl;

    std::string param;
    std::string junk;   // Read junk from the file

	//page
    file >> junk;

	//id=0
    file >> junk;

	//file="elegantech24_0.png"
    file >> param;
    texturePath = getStringValue(param);
    texture = new Texture((FONT_TEX_PATH + texturePath).c_str());

	log() << std::endl;
}

void Font::readCharInfo(std::ifstream& file)
{
	//chars count=95
	//char id=32   x=10    y=18    width=1     height=1     xoffset=0     yoffset=20    xadvance=5     page=0  chnl=15
	//1    2       3       4       5           6            7             8             9              10      11

	log() << "Reading Char Info." << std::endl;

    std::string param;
    std::string junk;   // Read junk from the file

	//chars
    file >> junk;

	//count=95
    file >> junk;

	//char
    file >> param;

    while(param == "char" && !file.eof())
    {
    	//id=32
        file >> param;
        int id = getIntValue(param);

    	//x=10
        file >> param;
        charData[id].x = getIntValue(param);

    	//y=18
        file >> param;
        charData[id].y = getIntValue(param);

    	//width=1
        file >> param;
        charData[id].width = getIntValue(param);

    	//height=1
        file >> param;
        charData[id].height = getIntValue(param);

    	//xoffset=0
        file >> param;
        charData[id].xoffset = getIntValue(param);

    	//yoffset=20
        file >> param;
        charData[id].yoffset = getIntValue(param);

    	//xadvance=5
        file >> param;
        charData[id].xadvance = getIntValue(param);

    	//page=0
        file >> param;
        charData[id].page = getIntValue(param);

    	//chnl=15
        file >> param;
        charData[id].chnl = getIntValue(param);

        createFontVBO(id);

    	//char
        file >> param;
    }

	log() << std::endl;
}

void Font::createFontVBO(int id)
{
    float vertLeft   = charData[id].xoffset;
    float vertRight  = charData[id].xoffset + charData[id].width;
    float vertTop    = charData[id].yoffset;
    float vertBottom = charData[id].yoffset + charData[id].height;

    GLfloat verts[12] =
    {
        vertLeft,  vertTop,    0.0f,
        vertLeft,  vertBottom, 0.0f,
        vertRight, vertTop,    0.0f,
        vertRight, vertBottom, 0.0f
    };

    createVertexBuffer(charData[id].vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, charData[id].vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);


	float texLeft   = ((float)  charData[id].x / textureSize.x);
	float texRight  = ((float) (charData[id].x + charData[id].width) / textureSize.x);
	float texTop    = 1.0f - ((float)  charData[id].y / textureSize.y);
	float texBottom = 1.0f - ((float) (charData[id].y + charData[id].height) / textureSize.y);

    GLfloat texCoords[8] =
    {
        texLeft,  texTop,
        texLeft,  texBottom,
        texRight, texTop,
        texRight, texBottom
    };

    createVertexBuffer(charData[id].texCoordVBO);
    glBindBuffer(GL_ARRAY_BUFFER, charData[id].texCoordVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(texCoords), texCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Font::~Font()
{

}

void Font::print(GLfloat x, GLfloat y, const char * text) const
{

}

void Font::printDebug() const
{
//    unsigned char chars[256];
//
//    for(unsigned char charInd = 1; charInd < 255; charInd++)
//    {
//        chars[charInd - 1] = charInd;
//    }
//
//    chars[255] = 0;
//
//    print(0, 0, (const char *)chars);
}

int Font::getCharWidth(unsigned char character)
{
    return charData[character].xadvance;
}

int Font::getCharHeight()
{
    return lineHeight;
}

void Font::getPrintDimensions(GLfloat * width, GLfloat * height, const char * text) const
{
    GLfloat curX = 0;
    GLfloat maxX = 0;
    GLfloat curY = 0;

    //go through the string and get dimensions of every character
    while(*text)
    {
        curX = getLineWidth(text, &text);

        if(*text == '\n')
        {
            text++;
            curY += lineHeight;
        }

        if(curX > maxX)
        {
            maxX = curX;
        }
    }

    if (width != NULL)
        *width = curX > maxX ? curX : maxX;

    if (height != NULL)
        *height = curY + lineHeight;
}

GLfloat Font::getLineWidth(const char * text, const char ** textPos) const
{
    GLfloat curX = 0;

    //go through the string until a newline and get dimensions of every character
    while(*text && *text != '\n')
    {
        //parse special characters
        switch(*text)
        {
            case ' ':       //space
                curX += whitespace;
                text++;
                continue;
                break;

            case '\t':      //tab
                curX += whitespace << 2;
                text++;
                continue;
                break;

            case '^':       //color code
                if(*(text + 1) >= '0' && *(text + 1) <= '9')
                {
                    text += 2;
                    continue;
                }
                break;
        }

        //if a character doesn't exist in the font just do a space
        if(charData[(unsigned char)(*text)].vertexVBO == 0)
        {
            curX += whitespace;
            text++;
            continue;
        }

        curX += charData[(unsigned char)(*text)].xadvance;
        text++;
    }

    if(textPos)
    {
        *textPos = text;
    }

    return curX;
}
