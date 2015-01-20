#
# graph.py
#
# reads potentiometer value from PIC16F1 HID Simple demo
#
import hid
import time

# clear screen
print "\x1b[2J\x1b[1;1H"   

# diagnostics
#for d in hid.enumerate(0, 0):
#    keys = d.keys()
#    keys.sort()
#    for key in keys:
#        print "%s : %s" % (key, d[key])
#    print ""

h = hid.enumerate( 0x4d8, 0x3f)	# returns a list 
# note: multiple devices can have the same vid/pid
print "Opening device"

try:
    print h
    print "Product: %s" % h[0]['product_string']
    print "Manufacturer: %s" % h[0]['manufacturer_string']
    #print "Serial No: %s" % h[0]['serial_number_string']
except:
    print "Failed to get the strings!"
    pass

try:
    h = hid.device(0x4d8, 0x3f)
   

    # try non-blocking mode by uncommenting the next line
    #h.set_nonblocking(1)

    # try writing some data to the device
    #for k in range(10):
    #    for i in [0, 1]:
    #        for j in [0, 1]:
    #            h.write([0x80, i, j])

    while True:
	# test the button status
	h.write( [0x37])
	d = h.read(3)
	if d:
	    #print d
            #print"\x1b["
            print "*".rjust( (d[1]+d[2]*256) >> 4 )
        time.sleep(0.1)

    print "Closing device"
    h.close()

except IOError, ex:
    print ex
    print "You probably don't have the hard coded test hid. Update the hid.device line"
    print "in this script with one from the enumeration list output above and try again."

print "Done"




