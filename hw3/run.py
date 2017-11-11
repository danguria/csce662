#!/usr/bin/python

import sys
import os

print 'Number of arguments:', len(sys.argv), 'arguments.'
print 'Argument List:', str(sys.argv)

if len(sys.argv) != 2:
    print "usage: ./run.py machine_ip" 
    sys.exit()



ids = []
master = "0"

f = open("server-cfg.txt", "r")
for line in f: 
    id, ip, p2pport, chatport = line.split(":")
    if ip == sys.argv[1]:
        ids.append(id)
        if int(id) > int(master):
            master = id

if os.path.exists("clock.txt"):
    os.remove("clock.txt")
#if os.path.exists("cmd.txt"):
#    os.remove("cmd.txt")
for id in ids:
    os.system('./bin/fbsd %s %s %s &' % (sys.argv[1], id, master))
