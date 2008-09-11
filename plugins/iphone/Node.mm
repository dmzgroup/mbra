#import "Edge.h"
#import "Node.h"

#define GROW_ANIMATION_DURATION_SECONDS 0.15
#define SHRINK_ANIMATION_DURATION_SECONDS 0.15


@interface Node (Private)
- (void)startGrowAnimation;
- (void)startResetAnimation;
- (void)startShrinkAnimation;
- (void)startRankGrowAnimation;
- (void)rankGrowAnimationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context;
- (void)startRankShrinkAnimation;
- (void)newNodeAnimationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context;
@end


@implementation Node

@synthesize overlayView = _overlayView;
@synthesize rankLabel = _rankLabel;


- (id)init {
   
   UIImage *nodeImage = [UIImage imageNamed:@"NA_Node.png"];
   UIImage *overlayImage = [UIImage imageNamed:@"NA_ResultsOverlay.png"];
   
	if (self = [super initWithImage:nodeImage]) {

      _overlayView = [[UIImageView alloc] initWithImage:overlayImage];
      _overlayOffset = CGPointMake (19.0, 2.0);
            
      CGRect rankRect = CGRectMake (24.0, 2.0, 28.0, 28.0);
      _rankLabel = [[UILabel alloc] initWithFrame:rankRect];
      _rankLabel.font = [UIFont boldSystemFontOfSize:24.0];
      _rankLabel.adjustsFontSizeToFitWidth = YES;
      _rankLabel.backgroundColor = [UIColor clearColor];
      _rankLabel.textColor = [UIColor whiteColor];
      _rankLabel.textAlignment = UITextAlignmentCenter;
      [_overlayView addSubview:_rankLabel];
      
      _overlayView.center = _overlayOffset;
      _overlayView.transform = CGAffineTransformMakeScale (0.008, 0.008);
      
      [self addSubview:_overlayView];   

      _edgeList = [[NSMutableArray alloc] initWithCapacity:3];
      
      [self setRank:0];
	}
   
	return self;
}


- (void)dealloc {
   
   [self removeAllEdges];
   [_edgeList release];
   [_overlayView release];
   [_rankLabel release];
	[super dealloc];
}


- (void)setRank:(NSUInteger)rank {
         
   _rankLabel.text = [[NSNumber numberWithInt:rank] stringValue];
   
   if (rank && !_rank) {
   
      [self startRankGrowAnimation];
      
   }
   else if (!rank && _rank) {
      
      [self startRankShrinkAnimation];
   }
   
   _rank = rank;
}

      
- (void)setPos:(CGPoint)pos {
   
   self.center = pos;
   
   for (Edge *edge in _edgeList) {

      [edge adjust];
   }
}


- (CGPoint)pos {
   
   return self.center;
}


- (void)addEdge:(Edge *)edge {
   
   [_edgeList addObject:edge];
}


- (void)removeEdge:(Edge *)edge {

   [_edgeList removeObject:edge];
}


- (void)removeAllEdges {

   NSMutableArray *tmpEdgeList = [[NSMutableArray alloc] initWithCapacity:[_edgeList count]];
   [tmpEdgeList addObjectsFromArray:_edgeList];
   
   for (Edge *edge in tmpEdgeList) {
   
      [edge removeFromSourceAndDest];
      [edge removeFromSuperview];
   }
   
   [_edgeList removeAllObjects];
   [tmpEdgeList release];
}


- (BOOL)pointInside:(CGPoint)point withEvent:(UIEvent *)event {

   BOOL result = NO;
   
   if (_rank) {
  
      CGPoint newPoint = [self convertPoint:point toView:_overlayView];
      result = [_overlayView pointInside:newPoint withEvent:event];
   }
   else {
      
      result = [super pointInside:point withEvent:event];
   }
   
   return result;
}


//- (UIView *)hitTest:(CGPoint)point withEvent:(UIEvent *)event {
//      
//   UIView *hit = [super hitTest:point withEvent:event];
//      
//   if (!hit) {
//
//      if (_rank) {
//         
//         hit = [_overlayView hitTest:point withEvent:event];
//      }
//   }
//   
//   return hit;
//}


- (void)setSelected:(BOOL)value {

   if (value) {
      
      [self startGrowAnimation];
   }
   else {
      
      [self startResetAnimation];
   }
}


- (void)startRankGrowAnimation {
   
   [self addSubview:_overlayView];

	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:GROW_ANIMATION_DURATION_SECONDS];
   [UIView setAnimationDelegate:self];
	[UIView setAnimationDidStopSelector:@selector(rankGrowAnimationDidStop:finished:context:)];
   _overlayView.transform = CGAffineTransformMakeScale (1.25, 1.25);
	[UIView commitAnimations];	
}


- (void)rankGrowAnimationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context {
   
   [UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:SHRINK_ANIMATION_DURATION_SECONDS];
	_overlayView.transform = CGAffineTransformIdentity;
   [UIView commitAnimations];
}


- (void)startRankShrinkAnimation {
   
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:SHRINK_ANIMATION_DURATION_SECONDS];
   [UIView setAnimationDelegate:_overlayView];
   [UIView setAnimationDidStopSelector:@selector(removeFromSuperview)];
   _overlayView.transform = CGAffineTransformMakeScale (0.008, 0.008);
	[UIView commitAnimations];	
}


- (void)startGrowAnimation {
   
   [UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:GROW_ANIMATION_DURATION_SECONDS];
   self.transform = CGAffineTransformMakeScale (1.5, 1.5);
   [UIView commitAnimations];
}


- (void)startResetAnimation {
   
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:SHRINK_ANIMATION_DURATION_SECONDS];
	self.transform = CGAffineTransformIdentity;
	[UIView commitAnimations];	
}


- (void)startNewNodeAnimation {
   
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:GROW_ANIMATION_DURATION_SECONDS];
   [UIView setAnimationDelegate:self];
	[UIView setAnimationDidStopSelector:@selector(newNodenimationDidStop:finished:context:)];
   self.transform = CGAffineTransformMakeScale (1.25, 1.25);
	[UIView commitAnimations];	
}


- (void)newNodenimationDidStop:(NSString *)animationID finished:(NSNumber *)finished context:(void *)context {
   
   [UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:SHRINK_ANIMATION_DURATION_SECONDS];
	self.transform = CGAffineTransformIdentity;
   [UIView commitAnimations];
}


- (void)shrinkAndRemoveFromSuperview {
   
	[UIView beginAnimations:nil context:NULL];
	[UIView setAnimationDuration:SHRINK_ANIMATION_DURATION_SECONDS];
   [UIView setAnimationDelegate:self];
   [UIView setAnimationDidStopSelector:@selector(removeFromSuperview)];
   self.transform = CGAffineTransformMakeScale (0.008, 0.008);
	[UIView commitAnimations];	
}


@end