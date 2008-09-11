#import <UIKit/UIKit.h>

@class Edge;


@interface Node : UIImageView {

   UIImageView *_overlayView;
   CGPoint _overlayOffset;
   UILabel *_rankLabel;
   
   NSMutableArray *_edgeList;
   
   CGPoint _pos;
   NSUInteger _rank;
}

@property (nonatomic, retain) UIImageView *overlayView;
@property (nonatomic, retain) UILabel *rankLabel;
@property (nonatomic, readwrite) CGPoint pos;

- (void)addEdge:(Edge *)edge;
- (void)removeEdge:(Edge *)edge;
- (void)removeAllEdges;

- (void)setRank:(NSUInteger)rank;
- (void)setSelected:(BOOL)value;

- (void)startNewNodeAnimation;
- (void)shrinkAndRemoveFromSuperview;

@end
