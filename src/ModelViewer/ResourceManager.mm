//
//  ResourceManager.mm
//  ModelViewer
//
//  Created by Robert Crosby on 7/6/11.
//  Copyright 2011 In-Con. All rights reserved.
//


#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <string>
#import <iostream>
#import <fstream>
#import "Interfaces.hpp"

using namespace std;

class ResourceManager : public IResourceManager {
public:
    ResourceManager();
    ~ResourceManager();
    string* GetResourcePath();
    TextureDescription LoadPngImage(string filename);
    void* GetImageData()
    {
        return (void*) CFDataGetBytePtr(m_imageData);
    }
    void UnloadImage()
    {
        CFRelease(m_imageData);
    }
    string* GetVertexShader(string vShaderFile);
    string* GetFragmentShader(string fShaderFile);
    void UnloadShaders()
    {
        if (m_vshader != NULL) {
            delete m_vshader;
            m_vshader = NULL;
        }
        if (m_fshader != NULL) {
            delete m_fshader;
            m_fshader = NULL;
        }
    }
private:
    ivec2 m_imageSize;
    CFDataRef m_imageData;
    string* m_path;
    string* m_vshader;
    string* m_fshader;
};

IResourceManager* CreateResourceManager() {
    return new ResourceManager();
}

ResourceManager::ResourceManager() {
    m_vshader = NULL;
    m_fshader = NULL;
    m_path = NULL;
}

ResourceManager::~ResourceManager() {
    free(m_vshader);
    free(m_fshader);
    free(m_path);
}


string* ResourceManager::GetResourcePath() {
    if (m_path == NULL) {
        NSString* bundlePath =[[NSBundle mainBundle] resourcePath];
        m_path = new string([bundlePath UTF8String]);
    }
    return m_path;
}

TextureDescription ResourceManager::LoadPngImage(string filename) {
    NSString *path = [[NSBundle mainBundle]bundlePath];
    NSString *name = [[NSString alloc] initWithUTF8String:filename.c_str()];
    NSString* fullPath = [path stringByAppendingPathComponent:name];
    [name release];
    
    NSLog(@"Loading PNG image %s", [fullPath UTF8String]);
    
    UIImage* uiImage = [[UIImage alloc] initWithContentsOfFile:fullPath];
    CGImageRef cgImage = uiImage.CGImage;
    m_imageData = CGDataProviderCopyData(CGImageGetDataProvider(cgImage));
    
    TextureDescription description;
    description.size.x = CGImageGetWidth(cgImage);
    description.size.y = CGImageGetHeight(cgImage);
    bool hasAlpha = CGImageGetAlphaInfo(cgImage) != kCGImageAlphaNone;
    CGColorSpaceRef colorSpace = CGImageGetColorSpace(cgImage);
    switch (CGColorSpaceGetModel(colorSpace)) {
        case kCGColorSpaceModelMonochrome:
            description.Format = hasAlpha ? TextureFormatGrayAlpha : TextureFormatGray;
            break;
        case kCGColorSpaceModelRGB:
            description.Format = hasAlpha ? TextureFormatRgba : TextureFormatRgb;
            break;
        default:
            assert(!"Unsupported color space.");
            break;
    }
    description.BitsPerComponent = CGImageGetBitsPerComponent(cgImage);
    
    [uiImage release];
    
    return description;
}

string* ResourceManager::GetVertexShader(string vShaderFile) {
    if (m_vshader != NULL)
        delete m_vshader;
    NSString* fileName = [[NSString alloc] initWithUTF8String:vShaderFile.c_str()];
    NSString* path = [[NSBundle mainBundle] pathForResource:fileName ofType:@"vert"];
    [fileName release];
    NSString* shader = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
    const char* cString = [shader UTF8String];
    m_vshader = new string(cString);
    return m_vshader;
}

string* ResourceManager::GetFragmentShader(string fShaderFile) {
    if (m_fshader != NULL)
        delete m_fshader;
    NSString* fileName = [[NSString alloc] initWithUTF8String:fShaderFile.c_str()];
    NSString* path = [[NSBundle mainBundle] pathForResource:fileName ofType:@"frag"];
    [fileName release];
    NSString* shader = [NSString stringWithContentsOfFile:path encoding:NSUTF8StringEncoding error:nil];
    const char* cString = [shader UTF8String];
    m_fshader = new string(cString);
    return m_fshader;
}