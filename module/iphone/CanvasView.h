#import <UIKit/UIKit.h>
#import <dmzTypesBase.h>
@class Canvas;
@class NAController;
//@class NAInfoController;


@interface CanvasView : UIScrollView <UIScrollViewDelegate> {

   NAController *_naController;
//   NAInfoController *_infoController;
   
   Canvas *_view;   
   NSInteger _mode;
   BOOL _handleTouch;
   dmz::Handle _objHandle;
   BOOL _objMoved;
}

@property (nonatomic, retain) Canvas *view;
@property (nonatomic, retain) NAController *naController;
//@property (nonatomic, retain) NAInfoController *infoController;

- (void)setMode:(NSInteger)mode;

- (dmz::Handle)objectAtPoint:(CGPoint)point withEvent:(UIEvent *)event;

@end
