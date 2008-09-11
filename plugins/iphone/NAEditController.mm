#import "dmziPhonePluginNodeProperties.h"
#import <dmzObjectModule.h>
#import "NAEditController.h"
#import "Node.h"

@interface NAEditController (PrivateMethods)
-(void)_get_node;
-(void)_update_node;
@end


@implementation NAEditController

@synthesize tableView = _tableView;


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil {
   
	if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
   
      self.title = @"Edit Node";   
      _nodeStruct = new dmz::iPhonePluginNodeProperties::NodeStruct;
   }
   
	return self;
}


- (void)dealloc {

   if (_nodeStruct) { delete _nodeStruct; _nodeStruct = 0; }
   
   [_tableView release];
	[super dealloc];
}



/*
 Implement loadView if you want to create a view hierarchy programmatically
- (void)loadView {
}
 */


- (void)viewDidLoad {
   
   [super viewDidLoad];

   _tableView.sectionHeaderHeight = 10.0f;
   _tableView.sectionFooterHeight = 10.0f;

   _nameTextField.font = [UIFont fontWithName:@"Helvetica" size:15];
   _eliminationCostTextField.font = [UIFont fontWithName:@"Helvetica" size:15];
   _consequenceTextField.font = [UIFont fontWithName:@"Helvetica" size:15];
   
   _nameLabel.font = [UIFont boldSystemFontOfSize:17];
   _eliminationCostLabel.font = [UIFont boldSystemFontOfSize:17];
   _consequenceLabel.font = [UIFont boldSystemFontOfSize:17];
   
   self.navigationItem.rightBarButtonItem = _saveButton;   
   self.navigationItem.leftBarButtonItem = _cancelButton;  
}


- (void)viewWillAppear:(BOOL)animated {
   
	NSIndexPath *tableSelection = [_tableView indexPathForSelectedRow];
	[_tableView deselectRowAtIndexPath:tableSelection animated:NO];
   
   [_tableView reloadData];
}


- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation {
   
	// Return YES for supported orientations
	return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)didReceiveMemoryWarning {
   
	[super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
	// Release anything that's not essential, such as cached data
}


- (void)nodeToEdit:(const dmz::Handle)TheHandle plugin:(dmz::iPhonePluginNodeProperties *)thePlugin {
   
   _nodeHandle = TheHandle;
   _plugin = thePlugin;
   
   if (_plugin && _nodeStruct) {
   
      _plugin->get_node (_nodeHandle, *_nodeStruct);
   }
   
//   self.title = [NSString stringWithFormat:@"Edit Node: %d", node.tag];
//   self.node = node;
}


- (IBAction)cancelAction:(id)sender {
   
	[_currentEditingTextField resignFirstResponder];	
   
   [self.navigationController dismissModalViewControllerAnimated:YES];

   if (_nodeStruct) {
      
      _nodeStruct->name.flush ();
      _nodeStruct->eliminationCost = 0.0;
      _nodeStruct->consequence = 0.0;
   }
   
   _plugin = 0;
   _nodeHandle = 0;
}


- (IBAction)saveAction:(id)sender {

	[_currentEditingTextField resignFirstResponder];	

   [self.navigationController dismissModalViewControllerAnimated:YES];

   if (_plugin && _nodeStruct) {

      _nodeStruct->name = [_nameTextField.text UTF8String];
      _nodeStruct->eliminationCost = [_eliminationCostTextField.text floatValue];
      _nodeStruct->consequence = [_consequenceTextField.text floatValue];

      _plugin->update_node (_nodeHandle, *_nodeStruct);
      
      _nodeStruct->name.flush ();
      _nodeStruct->eliminationCost = 0.0;
      _nodeStruct->consequence = 0.0;
   }
   
   _plugin = 0;
   _nodeHandle = 0;
}


- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
   
	return 1;
}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {

   NSInteger rows = 3;   
//   if (section == 1) { rows = 1; }
	return rows;
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath {
	
//   UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"NAEditCell"];
//   
//   if (cell == nil) {
//      
//   	cell = [[[UITableViewCell alloc] initWithFrame:CGRectZero reuseIdentifier:@"NAEditCell"] autorelease];
//   }
//	
//	cell.text = @"NA Edit Cell";

   UITableViewCell *cell = nil;
   
   if (_nodeStruct) {
      
      switch (indexPath.row) {
            
         case 0:
            _nameTextField.text = @"";
            const char *txtPtr = _nodeStruct->name.get_buffer ();

            if (txtPtr) {
               
               _nameTextField.text = [NSString stringWithUTF8String:_nodeStruct->name.get_buffer ()];
            }
            
            _nameTextField.clearButtonMode = UITextFieldViewModeWhileEditing;
            cell = _nameTableViewCell;        
            break;
            
         case 1:
            _eliminationCostTextField.text = @"";
            if (_nodeStruct->eliminationCost > 0.0) {
               
               _eliminationCostTextField.text = [NSString stringWithFormat:@"%1.2f", _nodeStruct->eliminationCost];
            }
            _eliminationCostTextField.clearButtonMode = UITextFieldViewModeWhileEditing;
            cell = _eliminationCostTableViewCell;
            break;
            
         case 2:
            _consequenceTextField.text = @"";
            if (_nodeStruct->consequence > 0.0) {
               
               _consequenceTextField.text = [NSString stringWithFormat:@"%1.2f", _nodeStruct->consequence];
            }
            _consequenceTextField.clearButtonMode = UITextFieldViewModeWhileEditing;
            cell = _consequenceTableViewCell;
            break;
            
         default:
            break;
      }
   }
   
	return cell;
}


- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField {
   
   BOOL result = NO;

   if ((textField.tag == 111) || (textField.tag == 112) || (textField.tag == 113)) {
   
      result = YES;
   }
   
   _currentEditingTextField = textField;
   
	return result;
}


- (BOOL)textFieldShouldReturn:(UITextField *)textField {

   _currentEditingTextField = nil;
   
   [textField resignFirstResponder];
	return YES;
}


- (void)textFieldDidEndEditing:(UITextField *)textField {
	
   if (textField.tag == 111) {
      
//      NSLog (@"Name: %@", textField.text);
   }
   else if (textField.tag == 112) {
      
//      NSLog (@"Elimination Cost: %@", textField.text);
   }
   else if (textField.tag == 113) {
      
//      NSLog (@"Consequence: %@", textField.text);
   }
}


@end
