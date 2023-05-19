#include <Foundation/Foundation.h>
#include <hoge.h>
#include <iostream>

@interface Test : NSObject
- (void)SayHello:(NSString*)msg;
@end

@implementation Test
- (void)SayHello:(NSString*)msg {
  std::cout << [msg UTF8String] << std::endl;
}
@end

void foo() {
  auto test = [[Test alloc] init];
  [test SayHello:@"Foo"];
  std::cout << "Hello" << std::endl;
}
