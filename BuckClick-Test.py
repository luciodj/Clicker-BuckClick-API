#!usr/bin/python
# 
# Buck Click test
#
import Click 

WR_PTR = 0x10 # set pointer bank, add
WR_IND = 0x12 # read indirect *p
WR_ERR = 0x14 # unimplemented command 

RD_PTR = 0x11 # read pointer 
RD_IND = 0x13 # read indirect *p
RD_ERR = 0x1f # read error flag and clear it

        
def ReadReg( reg):
    print "Read Reg [", reg,"] =",
    if Click.WriteI2C( WR_PTR, reg>>7, reg & 0x7f):
        r = Click.ReadI2C( RD_IND)
        if r[0] == 0x21:
            print r[1]
	    return
    #else 
    print "Fail"


def WriteReg( reg, data):
    print "Write Reg [",reg,"] =",data,
    if Click.WriteI2C( WR_PTR, reg>>7, reg & 0x7F):       
        if Click.WriteI2C( WR_IND, data, 0):
            print "OK"
            return
    #else
    print "Fail"         


# init the Click I2C interface
Click.InitI2C()

# test the error cmd 
#Write( WR_ERR, 0, 0)
#Read( RD_ERR)

#write to CCPR2L reg
WriteReg( 0x298, 0x5A)

#write to DACCON1 reg
WriteReg( 0x119, 0xA5)

# read register CCPR2L back
ReadReg( 0x298)

# read register DACCON1 back
ReadReg( 0x119)

Click.Close()




