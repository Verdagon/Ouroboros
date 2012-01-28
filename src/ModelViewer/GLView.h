
#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>

struct IRenderingEngine;

@interface GLView : UIView {
@private
    EAGLContext* m_context;
    struct IApplicationEngine* m_applicationEngine;
    struct IRenderingEngine* m_renderingEngine;
    struct IResourceManager* m_resourceManager;
    float m_timestamp;
}

- (void) drawView: (CADisplayLink*) displayLink;
- (void) appWillResignActive;
- (void) AppWillBecomeActive;
- (void) AppWillEnterBackground;
- (void) AppWillEnterForeground;
- (void) AppWillTerminate;

@end