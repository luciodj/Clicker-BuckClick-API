#!usr/bin/python
# 
# Click HIDAPI interface module
#
import hid

h = None

def Init():
    global h
    print "Connecting..."
    try:
        h = hid.device( 0x4d8, 0x3f)
    except( IOError):
        print "Failed!"
        return None
    print "Clicker found!"

    #print "Config the RST line", 
    h.write( [ 0x10])
    h.read( 1)

    #print "Send a reset pulse",
    h.write( [ 0x11])
    h.read( 1)
    #print "Clicker Reset"

def Close():
    global h
    if h:
        h.close()

#--------------------------------------------------------
def InitI2C():
    # init Clicker
    if not h:
        Init()
    # configure I2C
    h.write( [ 0x20])
    print "I2C port configured: ", h.read(1)[0] == 0x20
    return h

def ReadI2C( cmd):
#    print "Read", (cmd>>1)&7,":",
    h.write( [ 0x21, cmd, 2])
    return h.read(3) # read the data back
    

def WriteI2C( cmd, a, b):
#    print "Write", cmd, "(", a,",",b ,"):",
    h.write( [ 0x22, cmd, 2, a, b])
    return h.read(1)[0] == 0x22 # read the ack
        

#--------------------------------------------------------
def InitSPI():
    #init Clicker
    InitCS()
    # configure SPI
    h.write( [ 0x40])
    print "SPI port configured: ", h.read(1)[0] == 0x40
    return h

def WriteByteSPI( b):
    WriteCS( 0)
    h.write( [ 0x41, b])
    r = h.read(2)[1]
    WriteCS( 1)
    return r

def WriteWordSPI( w):
    WriteCS( 0)
    h.write( [ 0x41, w>>8])
    b = h.read(2)[1] << 8
    h.write( [ 0x41, w & 0xff])
    b += h.read(2)[1]
    WriteCS( 1)
    return b

#--------------------------------------------------------
def InitCS():
    if not h:
        Init()
    # init CS
    h.write( [0x14])
    print "CS pin configured" , h.read(1)[0] == 0x14
    return h

def WriteCS( d):
    h.write([0x16, d])
    return h.read(1)

#--------------------------------------------------------
def InitPWM():
    if not h:
        Init()
    h.write( [0x50])
    print "PWM configured", h.read(1)[0] == 0x50
    return h

def WritePWM( values):
    h.write( [0x52] + values)
    return h.read(1)

def ReadPWM():
    h.write( [0x51])
    return h.read(11)
