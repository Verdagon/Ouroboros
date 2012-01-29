
#import "GLView.h"
#import "Interfaces.h"
#import <OpenGLES/ES2/gl.h>


@implementation GLView

const bool ForceES1 = false;

+ (Class) layerClass
{
    return [CAEAGLLayer class];
}
 
- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        
        //Determine which device the app is running on.
        DeviceType deviceType = DeviecTypeUnknown;
        NSString* deviceModel = [UIDevice currentDevice].model;
        if ([deviceModel hasPrefix:@"iPhone"]) {
            deviceType = DeviceTypeIphone;
        } else if([deviceModel hasPrefix:@"iPod"]) {
            deviceType = DeviceTypeIpod;
        } else if([deviceModel hasPrefix:@"iPad"]) {
            deviceType = DeviceTypeIpad;
        }
        
        CAEAGLLayer* eaglLayer = (CAEAGLLayer*) super.layer;
        eaglLayer.opaque = YES;
        
        m_context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
        
        EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
        m_context = [[EAGLContext alloc] initWithAPI:api];
        
        if (!m_context || ForceES1) {
            api = kEAGLRenderingAPIOpenGLES1;
            m_context = [[EAGLContext alloc] initWithAPI:api];
        }
        
        if (!m_context || ![EAGLContext setCurrentContext:m_context]) {
            [self release];
            return nil;
        }
        
        m_resourceManager = CreateResourceManager();
        
        if (api == kEAGLRenderingAPIOpenGLES1) {
            NSLog(@"This game does not support OpenGL ES 1.1");
            exit(1);
            //m_renderingEngine = ES1::CreateRenderingEngine(m_resourceManager);
        } else {
            NSLog(@"Using OpenGL ES 2.0");
            //m_renderingEngine = ES2::CreateRenderingEngine(m_resourceManager);
            m_renderingEngine = CreateRenderingEngine(m_resourceManager);
        }
        
        m_applicationEngine = CreateApplicationEngine(deviceType, m_renderingEngine, m_resourceManager);
        
        [m_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:eaglLayer];
        
        m_applicationEngine->Initialize(CGRectGetWidth(frame), CGRectGetHeight(frame));
        
        [self drawView:nil];
        m_timestamp = CACurrentMediaTime();
        
        CADisplayLink* displayLink;
        
        displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(drawView:)];
        
        [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
    }
    
    return self;
}

- (void) drawView: (CADisplayLink*) displayLink
{
    if(displayLink != nil) {
        float elapsedSeconds = displayLink.timestamp - m_timestamp;
        m_timestamp = displayLink.timestamp;
        m_applicationEngine->UpdateAnimations(elapsedSeconds);
    }
    
    m_applicationEngine->Render();
    [m_context presentRenderbuffer:GL_RENDERBUFFER];
}

- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch* touch = [touches anyObject];
    CGPoint location = [touch locationInView:self];
    m_applicationEngine->OnFingerDown(vec2(location.x, location.y));
}

- (void) touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch* touch = [touches anyObject];
    CGPoint location = [touch locationInView:self];
    m_applicationEngine->OnFingerUp(vec2(location.x, location.y));
}

- (void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    NSArray *allTouches = [[event allTouches] allObjects];
    
    std::vector<vec2> touchvecs;
    for (UITouch *touch in allTouches)
    {
        CGPoint previous = [touch previousLocationInView:self];
        CGPoint current = [touch locationInView:self];
        touchvecs.push_back(vec2(previous.x, previous.y));
        touchvecs.push_back(vec2(current.x, current.y));
    }
    
    m_applicationEngine->OnFingerMove(touchvecs);
}

- (void) appWillResignActive
{
    m_applicationEngine->AppWillResignActive();
}

- (void) AppWillBecomeActive
{
    m_applicationEngine->AppWillBecomeActive();
}

- (void) AppWillEnterBackground
{
    m_applicationEngine->AppWillEnterBackground();
}

- (void) AppWillEnterForeground
{
    m_applicationEngine->AppWillEnterForeground();
}

- (void) AppWillTerminate
{
    m_applicationEngine->AppWillTerminate();
}

- (void)dealloc
{
    [super dealloc];
}

@end