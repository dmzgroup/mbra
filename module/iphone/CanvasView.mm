#import "CanvasView.h"
#import "Canvas.h"
#import "Constants.h"
#import "dmzMBRAModuleiPhone.h"
#import "NAController.h"
//#import "NAEditController.h"
//#import "NAInfoController.h"
//#import "MBRAAppDelegate.h"

//@interface CanvasView (Private)
//- (void) handle_mouse_event (NSSet *touches, UIEvent *event);
//@end

@implementation CanvasView

//static int localNodeCount = 100;
//static int localEdgeCount = 1100;

@synthesize view = _view;
@synthesize naController = _naController;
//@synthesize infoController = _infoController;


- (void)awakeFromNib {

   _view = [[Canvas alloc] initWithFrame:CGRectMake (0, 0, 1000, 1000)];

   self.contentSize = _view.bounds.size;
   self.minimumZoomScale = 0.5;
   self.maximumZoomScale = 5.0;
   self.delegate = self;
   
   [self setMode:kModeAdd];   
   [self addSubview:_view];
}


- (void)dealloc {
   
//   [_infoController release];
   [_naController release];
   [_view release];
	[super dealloc];
}


- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView {
   
   return _view;
} 


- (void)scrollViewDidEndZooming:(UIScrollView *)scrollView withView:(UIView *)view atScale:(float)scale {
   
}


//- (void)discover_module:(const dmz::Plugin *)PluginPtr withMode:(const dmz::PluginDiscoverEnum)Mode {
//   
//   if (Mode == dmz::PluginDiscoverAdd) {
//      
//      if (!_inputModule) {
//         
//         dmz::String inputModuleName ("dmzInputModuleBasic");
//         _inputModule = dmz::InputModule::cast (PluginPtr, inputModuleName);
//      }
//   }
//   else if (Mode == dmz::PluginDiscoverRemove) {
//      
//      if (_inputModule && (_inputModule == dmz::InputModule::cast (PluginPtr))) {
//         
//         _inputModule = 0;
//      }
//   }
//}


- (void)setMode:(NSInteger)mode {

   _mode = mode;
   
   dmz::MBRAModuleiPhone *module (dmz::MBRAModuleiPhone::get_instance ());
   if (module) { module->set_mode (_mode); }      
}


- (dmz::Handle)objectAtPoint:(CGPoint)point withEvent:(UIEvent *)event {
   
   dmz::Handle result (0);
   
   for (UIView *view in [_view subviews]) {

      if (view.tag) {
         
         CGPoint location = [_view convertPoint:point toView:view];
         
         if ([view pointInside:location withEvent:event]) {
            
            result = view.tag;
            break;
         }
      }
   }
   
   return result;
}


- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event {

   UITouch *touch = [touches anyObject];

   if ([touch view] == _view) {

      CGPoint touchLocation = [touch locationInView:_view];
      
      _objHandle = [self objectAtPoint:touchLocation withEvent:event];

      if (_mode == kModeAdd) {
         
         _handleTouch = YES;
      }
      else if (_mode == kModeEdit) {
         
         if (_objHandle) {
            
            _handleTouch = YES;
         }
      }
      else if (_mode == kModeLink) {
         
         if (_objHandle) {
            
            _handleTouch = YES;
         }
      }
      else if (_mode == kModeDelete) {
         
         if (_objHandle) {
            
            _handleTouch = YES;
         }
      }
   }
   
   if (_handleTouch) {

      dmz::MBRAModuleiPhone *module (dmz::MBRAModuleiPhone::get_instance ());
      if (module) { module->touches_began (touches, event); }
   }
   else {

      [super touchesBegan:touches withEvent:event];
   }
}


- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event {

//   BOOL handleTouch = NO;
   
//   UITouch *touch = [touches anyObject];
   
//   if (_mode == kModeEdit) {
    
//      if (_objHandle) {
         
//         _objMoved = YES;
//         handleTouch = YES;
//      }
//   }
//   else if (_mode == kModeLink) {
//   
//      if (_objHandle) {
//       
//         handleTouch = YES;
//      }
//   }
   
   if (_handleTouch) {
      
      dmz::MBRAModuleiPhone *module (dmz::MBRAModuleiPhone::get_instance ());
      if (module) { module->touches_moved (touches, event); }
   }
   else {
      
      [super touchesMoved:touches withEvent:event];
   }
}


- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event {
   
//   BOOL handleTouch = NO;

//   UITouch *touch = [touches anyObject];

//   if (_mode == kModeEdit) {
//   
//      if (_objHandle) {
//         
//         if (!_objMoved) {
//
//            handleTouch = YES;
//         }
//      }
//   }
//   else if (_mode == kModeLink) {
//
//      if (_objHandle) {
//         
//         handleTouch = YES;
//      }
//   }

   if (_handleTouch) {
            
      dmz::MBRAModuleiPhone *module (dmz::MBRAModuleiPhone::get_instance ());
      if (module) { module->touches_ended (touches, event); }
   }
   else {
      
      [super touchesEnded:touches withEvent:event];
   }

   _objHandle = 0;   
   _objMoved = NO;
   _handleTouch = NO;
}


- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event {

   if (_handleTouch) {
      
      dmz::MBRAModuleiPhone *module (dmz::MBRAModuleiPhone::get_instance ());
      if (module) { module->touches_ended (touches, event); }
   }
   else {
      
      [super touchesCancelled:touches withEvent:event];
   }
   
   _objHandle = 0;   
   _objMoved = NO;
   _handleTouch = NO;
}


@end
