#import <Cocoa/Cocoa.h>
#include <stdio.h>

int main(void)
{
	@autoreleasepool
	{
		NSScreen* s = [NSScreen mainScreen];
		if (s == nil)
			return 1;
		NSRect f = [s frame];
		printf("%d %d\n", (int) f.size.width, (int) f.size.height);
	}
	return 0;
}
