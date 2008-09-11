#import <UIKit/UIKit.h>

namespace dmz { class MBRAModuleiPhone; }

@class NAController;


@interface MBRAController : UIViewController <UINavigationBarDelegate, UITableViewDelegate, UITableViewDataSource> {

   IBOutlet UITableView *naTableView;
   dmz::MBRAModuleiPhone *_module;
}

@property (nonatomic, retain) UITableView *naTableView;

- (id)initWithModule:(dmz::MBRAModuleiPhone *)module
      nibName:(NSString *)nibNameOrNil
      bundle:(NSBundle *)nibBundleOrNil;

@end
