# 
# testing REDclick (SPI)
#

import Click
import time

Click.InitSPI()

# test all LEDs 
# print Click.WriteWordSPI( 0x0F01)
# print Click.WriteWordSPI( 0x0F00)

# exit shutdown mode
Click.WriteWordSPI( 0x0C01)
# set scan limit to 0-7
Click.WriteWordSPI( 0x0B07)

a = [ 1, 2, 4, 8, 16, 32, 64, 128];

def displayArray( a):
	for i, b in enumerate( a):
		Click.WriteWordSPI( i*256 + (b&0xff) + 256)
		#print "0x%X" % (i*256 + b + 256),

while True:
	displayArray( a )
	a = map( (lambda x: x<<1), a)
	time.sleep( 0.1)

