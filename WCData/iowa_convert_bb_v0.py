#!/usr/bin/python

import sys, getopt

def main(argv):
	inputfile = ''
	outputfile = ''
	opts, args = getopt.getopt(argv,"hi:o:",["ifile=","ofile="])
	for opt, arg in opts:
		if opt in ("-i", "--ifile"):
			inputfile = arg
		elif opt in ("-o", "--ofile"):
			outputfile = arg
			
	print 'Input file is "', inputfile
   	print 'Output file is "', outputfile
	
	# Read in the file
	fileIn = open(inputfile, 'r')
	
	# Outut file
	fileOut = open(outputfile, 'w')
	
	# vars we need
	vSpills = {}
	vEvents = {}
	currentSpill = 0
	currentEvent = 0
	eventData = 0
	module = 1
	channel = -1
	tdc = -1
	
	for line in fileIn:
		l = line.split()
		if l[0]=='SPILL':
			currentSpill = int(l[1])
			#    vSpills.insert(currentSpill, {})
			vSpills[currentSpill] = {}
			print 'Spill %d' % currentSpill
		if l[0]=='EVENT':
			currentEvent = int(l[1])
			#    vSpills[currentSpill].insert(currentEvent, {})
			(vSpills[currentSpill])[currentEvent] = {}
		#    print 'Event %d' % currentEvent
		if l[0] == 'Module':
			module = int(l[1])
			vSpills[currentSpill][currentEvent][module] = {}
			vChannels = []
		if l[0] == 'Channel':
			channel = int(l[1])
			tdc = int(l[2])
			if channel in vChannels:
				vSpills[currentSpill][currentEvent][module][channel].append(tdc)
			else:
				vSpills[currentSpill][currentEvent][module][channel] = []
				vSpills[currentSpill][currentEvent][module][channel].append(tdc)
				vChannels.append(channel)
			
	for spill in vSpills:
		for event in vSpills[spill]:
			for module in vSpills[spill][event]:
				for channel in vSpills[spill][event][module]:
					for tdc in vSpills[spill][event][module][channel]:
						str = '%d %d %d %d %d %d\n' % (spill, event, event, module, channel, tdc)
						fileOut.write(str)

if __name__ == "__main__":
	main(sys.argv[1:])

