#import <UIKit/UIKit.h>
#import <dmzRuntimePlugin.h>

namespace dmz {
   
   class ObjectModule;
   class iPhonePluginNodeProperties;
};


@interface NAEditController : UIViewController <UITableViewDataSource, UITableViewDelegate, UITextFieldDelegate> {

   dmz::Handle _nodeHandle;
   dmz::iPhonePluginNodeProperties::NodeStruct *_nodeStruct;
   dmz::iPhonePluginNodeProperties *_plugin;
   
   IBOutlet UIBarButtonItem *_saveButton;
   IBOutlet UIBarButtonItem *_cancelButton;
   
   IBOutlet UITableView *_tableView;

   IBOutlet UITableViewCell *_nameTableViewCell;
   IBOutlet UITableViewCell *_eliminationCostTableViewCell;
   IBOutlet UITableViewCell *_consequenceTableViewCell;
   
   IBOutlet UILabel *_nameLabel;
   IBOutlet UILabel *_eliminationCostLabel;
   IBOutlet UILabel *_consequenceLabel;

   IBOutlet UITextField *_nameTextField;
   IBOutlet UITextField *_eliminationCostTextField;
   IBOutlet UITextField *_consequenceTextField;
   
   UITextField *_currentEditingTextField;
}

@property (nonatomic, retain) UITableView *tableView;

- (IBAction)cancelAction:(id)sender;
- (IBAction)saveAction:(id)sender;

- (void)nodeToEdit:(const dmz::Handle)TheHandle plugin:(dmz::iPhonePluginNodeProperties *)thePlugin;

@end
