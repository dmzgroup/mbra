#import <UIKit/UIKit.h>

@class CanvasView;
//@class Node;
//@class NAEditController;


@interface NAController : UIViewController <UIActionSheetDelegate> {

   IBOutlet CanvasView *_canvas;
//   NAEditController *_naEditController;
}


@property (nonatomic, retain) CanvasView *canvas;
//@property (nonatomic, retain) NAEditController *naEditController;

- (IBAction)respondsToModeChange:(id)sender;
- (IBAction)calculateAction:(id)sender;

//- (void)editNode:(Node *)node;

@end
