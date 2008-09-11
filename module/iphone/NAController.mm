#import "CanvasView.h"
//#import "Constants.h"
#import "dmzMBRAModuleiPhone.h"
#import "NAController.h"
//#import "NAEditController.h"
//#import "Node.h"


@implementation NAController

@synthesize canvas = _canvas;
//@synthesize naEditController = _naEditController;


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
   
	if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {

      self.title = @"MBRA";
      
//      _naEditController = [[NAEditController alloc] initWithNibName:@"NAEditController" bundle:nibBundleOrNil];
	}
   
	return self;
}


- (void)dealloc {
   
//   [_naEditController release];
   
   _canvas.naController = nil;
   [_canvas release];
   
	[super dealloc];
}


/*
- (void)loadView {

}
*/


- (void)viewDidLoad {

   self.navigationController.navigationBarHidden = YES;
   _canvas.naController = self;
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
   
	return YES;
}


- (void)didReceiveMemoryWarning {
   
	[super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
	// Release anything that's not essential, such as cached data
}


- (void)respondsToModeChange:(id)sender {
   
//   NSLog (@"NAController respondsToModeChange: %d", [sender selectedSegmentIndex]);
   [_canvas setMode:[sender selectedSegmentIndex]];
}


- (void)calculateAction:(id)sender {
 
   UIActionSheet *actionSheet = [[UIActionSheet alloc] initWithTitle:@"Calculate & Identify Critical Nodes"
                                                            delegate:self
                                                   cancelButtonTitle: nil
                                              destructiveButtonTitle: nil
                                                   otherButtonTitles:@"Start", @"Stop", nil];
   
	actionSheet.actionSheetStyle = UIActionSheetStyleBlackOpaque;
	actionSheet.destructiveButtonIndex = 1;

	[actionSheet showInView:self.view];
	[actionSheet release];   
}


- (void)actionSheet:(UIActionSheet *)actionSheet clickedButtonAtIndex:(NSInteger)buttonIndex {
   
   BOOL calculate = NO;
	if (buttonIndex == 0) { calculate = YES; }
   
   dmz::MBRAModuleiPhone *module (dmz::MBRAModuleiPhone::get_instance ());
   if (module) { module->calculate (calculate); }   
}


#if 0
- (void)editNode:(Node *)node {

   UINavigationController *nc = [[UINavigationController alloc] initWithRootViewController:_naEditController];
   
   [_naEditController nodeToEdit:node];
   
   [self.navigationController presentModalViewController:nc animated:YES];

   [nc release];
}
#endif

@end
