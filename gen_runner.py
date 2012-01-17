#Nanolife - Simple artificial life simulator

#Copyright 2011 Mateus Zitelli <zitellimateus@gmail.com>

#This program is free software; you can redistribute it and/or modify
#it under the terms of the GNU General Public License as published by
#the Free Software Foundation; either version 2 of the License, or
#(at your option) any later version.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY; without even the implied warranty of
#MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#GNU General Public License for more details.

#You should have received a copy of the GNU General Public License
#along with this program; if not, write to the Free Software
#Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#MA 02110-1301, USA.
from random import choice
def run(prog, debug = False):
        mem_size = len(prog)
        times = [0 for i in range(mem_size)]
        ptr = 0
        pos = 0
        memory = [0 for i in range(mem_size)]
        new_genoma = [0 for i in range(mem_size)]
        last_adr = 0
        loops = []
        position = [250,250]
        rotation = choice([0,1,2,3])
        for i in range(2000):
	        if ptr >= len(prog) - 1 or pos >= len(prog) - 1: break
	        if prog[pos] == 1:
		        ptr += 1
		        #print 'ptr++;'
	        elif prog[pos] == 2:
		        ptr -= 1
		        #print 'ptr--;'
	        elif prog[pos] == 3:
		        memory[ptr] += 1
		        #print 'memory[ptr]++;'
	        elif prog[pos] == 4:
		        memory[ptr] -= 1
		        #print 'memory[ptr]--;'
	        elif prog[pos] == 5:
		        if(memory[ptr]):
			        loops.append((pos, ptr))
		        #print 'while(memory[ptr]){'
	        elif prog[pos] == 6:
		        if(len(loops) and memory[loops[-1][1]]):
			        pos = loops[-1][0]
		        elif(len(loops)):
			        del loops[-1]
		        #print '}'
	        elif prog[pos] == 7:
		        memory[ptr] = choice([0,1,2])
		        if(memory[ptr] == 1 and debug): print "Incompatible bot in front at", ptr
		        elif(memory[ptr] == 2 and debug): print "Compatible bot in front at", ptr
		        elif(debug): print "Null cell at", ptr
	        elif prog[pos] == 8:
		        memory[ptr] = rotation
		        if(debug): print 'Rotation =', rotation, 'at', ptr
	        elif prog[pos] == 9:
		        if(last_adr < mem_size): new_genoma[last_adr] = memory[ptr]
		        last_adr += 1
	        elif prog[pos] == 19:
		        if(last_adr < mem_size): new_genoma[last_adr] = prog[pos]
		        pos += 1
		        last_adr += 1
	        elif prog[pos] == 10:
		        rotation = (rotation + 1) % 4
		        if(debug): print "RC", rotation ,";"
	        elif prog[pos] == 11:
		        rotation = (rotation - 1) % 4
		        if(debug): print "RAC", rotation ,";"
	        elif prog[pos] == 12:
		        if(debug): print "MF",
		        if rotation == 0:
			        position[0] += 1
		        elif rotation == 1:
			        position[1] += 1
		        elif rotation == 2:
			        position[0] -= 1
		        elif rotation == 3:
			        position[1] -= 1
		        if(debug): print position, ';'
	        elif prog[pos] == 13:
		        if(debug): print "MB",
		        if rotation == 0:
			        position[0] -= 1
		        elif rotation == 1:
			        position[1] -= 1
		        elif rotation == 2:
			        position[0] += 1
		        elif rotation == 3:
			        position[1] += 1
		        if(debug): print position, ';'
	        elif prog[pos] == 14:
		        if(debug): print "REP;"
	        elif prog[pos] == 16:
		        if(debug): print "ATK;"
	        elif prog[pos] == 17:
		        if(debug): print "GIVE;"
	        elif prog[pos] == 15:
		        if(debug): print "SEX_REP;"
	        elif prog[pos] == 18:
		        if(debug): print "Create bot:", new_genoma
	        times[pos] += 1
	        pos += 1
	        #memory[ptr] %= 9
	        #print memory
	return times

if __name__ == "__main__":
        prog = [eval("0x" + k) for k in open('creat', 'r').read().split('\n')[0].split(',')]
        print prog
        print run(prog, debug = True)
