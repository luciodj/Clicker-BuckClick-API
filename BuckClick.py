#!usr/bin/python
#
# Buck-click control functions  
#

import Click

#
# define I2C BuckClick specific commands
# 
WR_PTR = 0x10 # set pointer bank, add
WR_IND = 0x12 # read indirect *p
WR_ERR = 0x14 # unimplemented command

RD_PTR = 0x11 # read pointer
RD_IND = 0x13 # read indirect *p
RD_ERR = 0x1f # read error flag and clear it

#---------------------------------------------------
# define 178x specific registers

DAC      = 0x119	# 8-bit dac 

PSMC1CON = 0x811	# main control register
PSMC1PRL = 0x825	# period register
PSMC1PRH = 0x826	
PSMC1DBR = 0x829	# dead band rising
PSMC1DBF = 0x82A	# dead band falling

#---------------------------------------------------

# flag if I2C connection with BuckClick is established
I2C = False	

def Init():
    global I2C
    I2C = (Click.InitI2C() != None)
    return I2C


def Close():
    Click.Close()


def ReadReg( reg):
    print "Read Reg [", reg,"] =",
    if Click.WriteI2C( WR_PTR, reg>>7, reg & 0x7f):
        r = Click.ReadI2C( RD_IND)
        if r[0] == 0x21:
            print r[1]
            return r[1]
		
    #else
    print "Fail"
    return 0


def WriteReg( reg, data):
    print "Write Reg [",reg,"] =",data,
    if Click.WriteI2C( WR_PTR, reg>>7, reg & 0x7F):
        if Click.WriteI2C( WR_IND, data, 0):
            print "OK"
            return
    #else
    print "Fail"
      

#=====================================================
# higher level commands
#
#----- DAC
def writeDAC( value):	# to DACCON1 reg
    if I2C:
    	print 'write DAC', int(value)
    	WriteReg( DAC, int(value))


def readDAC():	 	# read from DACCON1 reg
    if I2C:
	return ReadReg( DAC)
    else:
	return 0


#----- freq (PR)
def writeFreq( value):	# translate f (100-500kH) to PSMC1PRL
    f = int(value)
    if f<100: f = 100
    if f>500: f = 500
    period = (64000 / f ) -1
    if I2C:
    	print 'write PSMC1PRL', period
	WriteReg( PSMC1PRL, period)

        	
def readFreq():
    if I2C:
	period = ReadReg( PSMC1PRL)+1
	return 64000 / period	# returns frequency in kHz
    else:
	return 100

#----- DeadBand 
def writeDB( value):	# translate delay (16-160ns) to PSMC1DBR/F
    delay = int( value)
    if delay<16:   delay = 16
    if delay>160:  delay = 160
    ticks = delay / 16 
    if I2C:
    	print 'write PSMC1DBx', ticks
	WriteReg( PSMC1DBR, ticks)
	WriteReg( PSMC1DBF, ticks)

        	
def readDB():
    if I2C:
	ticks = ReadReg( PSMC1DBR)
	return 16 * ticks	# returns delay in ns
    else:
	return 16


#------- PSMC update command
def updatePSMC():	# synchronous write of all PSMC registers
    if I2C:
	print 'PSMC update'
	r = ReadReg( PSMC1CON)		# read current value (mode)
        WriteReg( PSMC1CON, r | 0x40)	# load register values

#------- Check the error register
def readError():
    if I2C:
   	return Click.ReadI2C( RD_ERR)
    else:
	return 0
 
