#import "dmzMBRAModuleiPhone.h"
#import "MBRAController.h"
#import "NAController.h"

@implementation MBRAController

@synthesize naTableView;


- (id)initWithModule:(dmz::MBRAModuleiPhone *)module
      nibName:(NSString *)nibNameOrNil
      bundle:(NSBundle *)nibBundleOrNil {
   
	if (self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil]) {
      
      self.title = [[[NSBundle mainBundle] infoDictionary] objectForKey:@"CFBundleName"];

      self.view.autoresizesSubviews = YES;
      self.view.autoresizingMask = UIViewAutoresizingFlexibleHeight | UIViewAutoresizingFlexibleWidth;

      _module = module;

      UIBarButtonItem *addButton = [[UIBarButtonItem alloc]
         initWithBarButtonSystemItem:UIBarButtonSystemItemAdd
         target:self
         action:@selector(newNetworkAnalysis)];
      
      self.navigationItem.rightBarButtonItem = addButton;
      
      [addButton release];
      
      [naTableView reloadData];      
	}
   
	return self;
}


/*
 Implement loadView if you want to create a view hierarchy programmatically
- (void)loadView {
}
 */

/*
 If you need to do additional setup after loading the view, override viewDidLoad.
- (void)viewDidLoad {
}
 */


- (BOOL)shouldAutorotateToInterfaceOrientation:
      (UIInterfaceOrientation)interfaceOrientation {
   
	return (interfaceOrientation == UIInterfaceOrientationPortrait);
}


- (void)didReceiveMemoryWarning {
   
	[super didReceiveMemoryWarning]; // Releases the view if it doesn't have a superview
	// Release anything that's not essential, such as cached data
}


- (void)dealloc {
   
   [naTableView release];
   
	[super dealloc];
}


- (void)newNetworkAnalysis {
   
   NSLog (@"New Network Analysis clicked");
   [naTableView reloadData];      
}

#pragma mark UIViewController delegates


- (void)viewWillAppear:(BOOL)animated {
   
	NSIndexPath *tableSelection = [naTableView indexPathForSelectedRow];
NSLog (@"viewWillAppear, row selected: %d", tableSelection.row);
	[naTableView deselectRowAtIndexPath:tableSelection animated:NO];
}


- (void)viewDidAppear:(BOOL)animated {
   
}


#pragma mark UITableView delegates

- (UITableViewCellAccessoryType)tableView:(UITableView *)tableView
      accessoryTypeForRowWithIndexPath:(NSIndexPath *)indexPath {
   
	return UITableViewCellAccessoryDisclosureIndicator;
}


- (void)tableView:(UITableView *)tableView
      didSelectRowAtIndexPath:(NSIndexPath *)indexPath {

   if (_module) {
      
//      _module->show_network_analysis (indexPath.row);
   }
}


#pragma mark UITableView datasource methods

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
   
	return 1;
}

//- (NSArray *)sectionIndexTitlesForTableView:(UITableView *)tableView {
//   
//}


- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
   
   NSInteger result (0);
   
   if (_module) {
      
//      result = _module->get_na_count ();
   }
   
	return result;
}


- (UITableViewCell *)tableView:(UITableView *)tableView
      cellForRowAtIndexPath:(NSIndexPath *)indexPath {
   
   static NSString *NACellIdentifier = @"NACell";
   
   UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:NACellIdentifier];

	if (cell == nil) {
      
      cell = [[[UITableViewCell alloc]
         initWithFrame:CGRectZero reuseIdentifier:NACellIdentifier] autorelease];
	}
   
   if (_module) {

//      cell.text = _module->get_na_name (indexPath.row);
   }
	
	return cell;
}

@end
