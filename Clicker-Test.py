# 
# clicker I2c test
#
import hid
import sys

print "Connecting..."
h = hid.device( 0x4d8, 0x3f)
if not h:
    print "Failed!"
    sys.exit(0)

print "Clicker found!"
#h.set_nonblocking(1)

# configure I2C
h.write( [ 0x20])
print "Configured: ", h.read(1)

# write I2C command
h.write( [ 0x22, 0xA0, 1, 0])
print h.read(1)

# read I2C command
h.write( [ 0x21, 0xA1, 3])

# read back data requested
print h.read(4)

h.close()




