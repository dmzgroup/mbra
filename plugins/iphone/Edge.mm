#import "Edge.h"
#import "Node.h"

#define GROW_ANIMATION_DURATION_SECONDS 0.15
#define SHRINK_ANIMATION_DURATION_SECONDS 0.15

@implementation Edge


- (id)initWithSourceNode:(Node *)theSourceNode destNode:(Node *)theDestNode {
   
	if (self = [super initWithFrame:CGRectZero]) {
      
      self.userInteractionEnabled = NO;
      self.contentMode = UIViewContentModeRedraw;
      self.backgroundColor = [UIColor clearColor];
      
      _sourceNode = theSourceNode;
      _destNode = theDestNode;
      
      [_sourceNode addEdge:self];
      [_destNode addEdge:self];
      
      [self adjust];
	}
   
	return self;
}


- (void)dealloc {
   
   [super dealloc];
}


- (void)removeFromSourceAndDest {

   [_sourceNode removeEdge:self];  
   [_destNode removeEdge:self];  

   _sourceNode = nil;
   _destNode = nil;
}


- (void)drawRect:(CGRect)rect {

   CGContextRef context = UIGraphicsGetCurrentContext ();
   CGContextSaveGState(context);

   CGContextSetRGBStrokeColor (context, 1.0, 1.0, 1.0, 1.0);
   CGContextSetLineWidth (context, 4.0);
   
   CGFloat sdx = _sourceNode.frame.size.width / 2.0;
   CGFloat sdy = _sourceNode.frame.size.height / 2.0;
   
   CGFloat ddx = _destNode.frame.size.width / 2.0;
   CGFloat ddy = _destNode.frame.size.height / 2.0;   
   
   CGPoint start;
   CGPoint end;

   if (_sourceNode.center.x < _destNode.center.x) {
      
      if (_sourceNode.center.y < _destNode.center.y) {
         
         start = CGPointMake (sdx, sdy);
         end = CGPointMake (self.frame.size.width - ddx, self.frame.size.height - ddy);
      }
      else {
       
         start = CGPointMake (sdx, self.frame.size.height - sdy);
         end = CGPointMake (self.frame.size.width - ddx, ddy);
      }      
   }
   else {

      if (_sourceNode.center.y < _destNode.center.y) {
         
         start = CGPointMake (self.frame.size.width - sdx, sdy);
         end = CGPointMake (ddx, self.frame.size.height - ddy);
      }
      else {
         
         start = CGPointMake (self.frame.size.width - sdx, self.frame.size.height - sdy);
         end = CGPointMake (ddx, ddy);
      }            
   }
      
   CGContextMoveToPoint (context, start.x, start.y);
   CGContextAddLineToPoint (context, end.x, end.y);
   CGContextStrokePath (context);      
   
   CGContextRestoreGState (context);
}


- (void)adjust {
 
   self.frame = CGRectUnion (_sourceNode.frame, _destNode.frame);
}


- (void)startGrowAnimation {
   
   [UIView beginAnimations:nil context:nil];
	[UIView setAnimationDuration:GROW_ANIMATION_DURATION_SECONDS];
   self.transform = CGAffineTransformIdentity;
   [UIView commitAnimations];
}


@end
