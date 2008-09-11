#import <UIKit/UIKit.h>

@class Node;

@interface Edge : UIView {

   Node *_sourceNode;
   Node *_destNode;
}

- (id)initWithSourceNode:(Node *)sourceNode destNode:(Node *)destNode;

- (void)removeFromSourceAndDest;

- (void)adjust;

- (void)startGrowAnimation;

@end
