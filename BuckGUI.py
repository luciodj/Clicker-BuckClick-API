#!usr/bin/python
#
# Buck Click GUI
#

import BuckClick

from Tkinter import *


###################################################################
# graphing dialog window definition
#
class Graphs():

  def __init__(self, parent = None):
    win = Toplevel()
    win.title( 'Graphs')
    win.protocol( 'WM_DELETE_WINDOW', win.quit)	# intercept red button
    win.bind( sequence='<Control-q>', func= win.quit)
    self.win = win
    self.running = False

    Label( win, text='Register').grid(
	padx=10, pady=1, row=0, column=0, sticky=W)

    register = StringVar()	# register to be logged
    register.set( '0x09c')	#ADRESH
    self.register = register
    e=Entry( win, width=8, takefocus=YES, textvariable=register)
    e.grid(  padx=10, pady=10, row=1, column=0, sticky=W)


    Label( win, text='Log Period (ms)').grid(
	padx=10, pady=1, row=0, column=1, sticky=W)

    logf = IntVar()		# samples per second
    logf.set( 20)
    self.logf = logf
    Entry( win, width=6, takefocus=YES, textvariable=logf).grid(
	padx=10, pady=10, row=1, column=1, sticky=W)

    Button( win, text='Start', takefocus=NO, command=self.cmdStart ).grid(
        padx=10, pady=10, row=1, column=2)

    Button( win, text='Stop', takefocus=NO, command=self.cmdStop ).grid(
	padx=10, pady=10, row=1, column=3)

    #------- graph canvas ----------------------------
    wx = 480; wy = 300; 
    self.wx = wx; self.wy = wy
    brd = 5
    graph = Canvas( win, width=wx, height=wy, relief='ridge', bd=brd, bg='black')
    graph.grid( padx=10, row=2, columnspan=4)
    self.graph = graph

    ox= brd+20; oy=brd+10
    self.ox = ox; self.oy = oy     
    x_axis=graph.create_line( ox-5, wy-oy, wx, wy-oy, arrow='last', fill='lightgreen')
    for i in xrange( 25, wx-ox-ox, 25):
        x = ox+i
	graph.create_line( x, wy-oy+5, x, oy+5, fill = 'green') 
	
    y_axis=graph.create_line( ox, wy-oy+5, ox, oy, arrow='last', fill='lightgreen')   
    for i in xrange( 1,9):
        y = wy-(oy+i*32)
	graph.create_line( ox-5, y, wx-ox, y, fill = 'green') 
    
    self.length = wx-ox*2
    #self.vector=[ 128 for x in xrange(self.length)]	# init the grap vector
    self.vector=[]
    self.drawLine()				# draw line

    #------- close -----------------------------------
    Button( win, text='Close', takefocus=NO, command=win.quit).grid(
	padx=10, pady=10, row=3, column=3)

    # set focus and cursor
    e.focus_set()	# take over input from other windows, select address field
    e.icursor(END)	# set cursor after last digit

    # make modal
    win.grab_set()	# disable other objects while this is open
    win.mainloop()      # run private main loop
    win.destroy()       # close on exit 


#-------------------- Graphs methods
  def drawLine( self):
    pointList=[]		# create a new list of points
    for x, v in enumerate( self.vector):
	pointList.append( (self.ox+ x, self.wy-(self.oy+v)) )
    if len(pointList) > 1:
        self.gline = self.graph.create_line( pointList, fill='white') 
    else:
	self.gline = None


  def cmdSample( self):
    # obtain a new sample
    try:			
        reg = int( self.register.get(), 0)
	r = BuckClick.ReadReg( reg )
    except:
    	r = 128

    # scroll the vector
    if len( self.vector) > self.length: 
	self.vector.pop(0)	        
    self.vector.append( r)

    # replace the line
    if self.gline: 
	self.graph.delete( self.gline)	# erase the old line
    self.drawLine()			# draw a new one
    
    if self.running:			# if no stop received, set next timeout
	self.win.after( self.logf.get(), self.cmdSample)

    
  def cmdStart( self):
    """ activate periodic sampling of register
    """
    self.running = True
    self.win.after( self.logf.get(), self.cmdSample)


  def cmdStop( self):
    	self.running = False



###################################################################
# advanced dialog window definition
#
class Advanced():
  def __init__( self, parent = None):
    win = Toplevel()
    win.title( 'Advanced')
    win.protocol( 'WM_DELETE_WINDOW', win.quit)	# intercept red button
    win.bind( sequence='<Control-q>', func= win.quit)

    register = StringVar()
    register.set( '0x119')
    self.register = register
    e=Entry( win, width=8, takefocus=YES, textvariable=register)
    e.grid(  padx=10, pady=10, row=1, column=0, sticky=W)

    Button( win, text='Read', takefocus=NO, command=self.cmdRead).grid(
        padx=10, pady=10, row=1, column=1)

    data = StringVar()
    data.set( '-')
    self.data = data
    Entry( win, width=6, takefocus=YES, textvariable=data).grid(
	padx=10, pady=10, row=2, column=0, sticky=W)

    Button( win, text='Write', takefocus=NO, command=self.cmdWrite).grid(
	padx=10, pady=10, row=2, column=1)

    Button( win, text='PSMC', takefocus=NO, command=BuckClick.updatePSMC).grid(
	padx=10, pady=10, row=2, column=2)

    Button( win, text='Close', takefocus=NO, command=win.quit).grid(
	padx=10, pady=10, row=3, column=3)

    # set focus and cursor
    e.focus_set()	# take over input from other windows, select address field
    e.icursor(END)	# set cursor after last digit

    # make modal
    win.grab_set()	# disable other objects while this is open
    win.mainloop()      # run private main loop
    win.destroy()       # close on exit 

#------------------- Advanced commands
  def cmdRead( self):
    i = int( self.register.get(), 0)
    try:
	r =  BuckClick.ReadReg( i)
    	self.data.set( "0x%X" % r)
    except:
	self.data.set( "Error")
 

  def cmdWrite( self):
    i = int( self.register.get(), 0)
    d = int( self.data.get(), 0)
    print "cmdRead", i, d
    try:
	BuckClick.WriteReg( i, d)

    except:
	self.data.set( "Error")

###################################################################
# main window definition
#
class MainWindow():

    def __init__( self):
	root = Tk()
	root.title( "Buck Click GUI")
	#root.geometry( '400x100')
	root.protocol( 'WM_DELETE_WINDOW', root.quit)	# intercept red button
	root.bind( sequence='<Control-q>', func= root.quit)

	root.grid_columnconfigure( 1, minsize=320)
	rowc = 0

    	#------- status bar --------------------------------------
	rowc += 1
	Label( root, text="Status:").grid(
         	padx=10, pady=10, row=rowc, sticky=W)

	Status = StringVar()
	Status.set( 'Uninitialized')
	self.Status = Status

	Label( root, textvariable=Status).grid( 
		padx=10, pady=10, row=rowc, column=1, sticky=W)

	#------- top buttons
	rowc += 1
	Button( root, text='Init', command=self.cmdInit).grid(
        	padx=10, pady=10, row = rowc, column=0, sticky=N+W)

	Button( root, text='PSMC', command=BuckClick.updatePSMC).grid(
		padx=10, pady=10, row=rowc, column=1)

	#------- DAC slider --------------------------------------
	rowc += 1
	dac = IntVar()
	dac.set(0)
	self.dac = dac

	Label( root, text="DAC:").grid( 
        	padx=10, pady=10, row=rowc, sticky=W)

	Scale( root, label = '', length=256, orient='horizontal',
		from_ = 0, to = 255,
		showvalue = YES,
		command = BuckClick.writeDAC,
		variable = dac
		).grid( row=rowc, column=1)

	#------- freq slider --------------------------------------
	rowc +=1
	freq = IntVar()
	freq.set(0)
	self.freq = freq

	Label( root, text="Freq:").grid( 
        	padx=10, pady=10, row=rowc, sticky=W)

	Scale( root, label = '', length=256, orient='horizontal',
		from_ = 100, to = 500, 
		resolution = 10,
		showvalue = YES,
		command = BuckClick.writeFreq,
		variable = freq
		).grid( row=rowc, column=1)

	#------- deadband control slider --------------------------
	rowc += 1
	db = IntVar()
	db.set(0)
	self.db = db
	Label( root, text="DeadBand:").grid( 
        	padx=10, pady=10, row=rowc, sticky=W)

	Scale( root, label = '', length=256, orient='horizontal',
		from_ = 16, to = 160, 
		resolution = 16,
		showvalue = YES,
		command = BuckClick.writeDB,
		variable = db
		).grid( row=rowc, column=1)

	#------- bottom row
	rowc += 1 
	Button( root, text='Advanced', command=Advanced).grid(
		padx=10, pady=10, row=rowc, column=0)

	Button( root, text='Graphs', command=Graphs).grid(
		padx=10, pady=10, row=rowc, column=1)

	Button( root, text='Quit', command=root.quit).grid(
        	padx=10, pady=10, row=rowc, column=2, sticky=E+S)

	#Graphs()

	#------------------ main commands
    def cmdInit( self):
    	if BuckClick.Init():
		self.Status.set('BuckClick connected')
		# get the sliders initial values
		self.dac.set( BuckClick.readDAC())
		self.freq.set( BuckClick.readFreq())
		self.db.set( BuckClick.readDB())
	else:
	        self.Status.set('BuckClick Connection Failed')
		# set sliders to min
		self.dac.set( 0)
		self.freq.set( 0)
		self.db.set( 0)


if __name__ == '__main__': 
	MainWindow()
	mainloop()
	BuckClick.Close()








