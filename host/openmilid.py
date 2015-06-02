#!/usr/bin/env python
# -*- encoding: utf-8 -*-

import argparse, serial, socket, select

FIRST_PORT = 8899

STATIC_COMMAND_MAP = {
    0x41: 0x02, # ALL OFF
    0x42: 0x01, # ALL ON
    0x43: 0x0C, # Disco slower
    0x44: 0x0B, # Disco faster
    0x45: 0x03, # Group 1 on
    0x46: 0x04, # Group 1 off
    0x47: 0x05, # Group 2 on
    0x48: 0x06, # Group 2 off
    0x49: 0x07, # Group 3 on
    0x4A: 0x08, # Group 3 off
    0x4B: 0x09, # Group 4 on
    0x4C: 0x0A, # Group 4 off
    0xC2: 0x11, # Set color white all
    0xC5: 0x13, # Set color white group 1
    0xC7: 0x15, # Set color white group 2
    0xC9: 0x17, # Set color white group 3
    0xCB: 0x19, # Set color white group 4
    0xC1: 0x12, # Night mode all
    0xC6: 0x14, # Night mode group 1
    0xC8: 0x16, # Night mode group 2
    0xCA: 0x18, # Night mode group 3
    0xCC: 0x1A, # Night mode group 4
}

class OpenMiLiDaemon(object):
    def __init__(self, args):
        self.args = args
        self.counter = {}
    
    def start(self):
        with serial.Serial(self.args.serial, self.args.baudrate, timeout=0.1) as fd:
            sockets = []
            for offset, idport in enumerate(args.idports):
                if ":" in idport:
                    port_number, sender_id = idport.split(":", 1)[:2]
                    port_number = int(port_number)
                else:
                    sender_id = idport
                    port_number = FIRST_PORT + offset
                sender_id = sender_id.upper()
                
                if False: # Not implemented yet
                    tcpsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    tcpsock.bind(('', port_number))
                    tcpsock.listen(3)
                    sockets.append( (tcpsock, sender_id) )
                
                udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                udpsock.bind(('', port_number))
                sockets.append( (udpsock, sender_id) )
            
            while True:
                r, _, _ = select.select( [fd] + [e for (e, i) in sockets], [], [], 60)
                
                if fd in r:
                    print fd.read(1024)
                
                for sock, sender_id in sockets:
                    if sock in r:
                        message = sock.recv(1024)
                        
                        self.handle_message(fd, sender_id, message)
    
    def handle_message(self, serial_fd, sender_id, message):
        counter = self.counter.get(sender_id, 0)
        self.counter[sender_id] = (counter + 1) & 0xff
        
        message = map(ord, message)
        
        if message[0] in STATIC_COMMAND_MAP:
            self.send_milight(serial_fd, sender_id, 0, 0, STATIC_COMMAND_MAP[message[0]], counter, 40)
    
    def send_milight(self, serial_fd, sender_id, val1, val2, cmd, counter, repetition=1):
        serial_fd.write(sender_id + "%02X%02X%02X%02X " % (val1, val2, cmd, counter))
        for i in range(repetition-1):
            serial_fd.write(".")
        serial_fd.write("\r\n")
    

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(description='OpenMiLi daemon -- Listen for commands on the network and control Mi-Light bulbs.')
    parser.add_argument('--serial', '-s', help="Device node for the serial line to connect through", default="/dev/ttyUSB0")
    parser.add_argument('--baudrate', '-b', help="Baud rate for the serial line", type=int, default=115200)
    parser.add_argument('idports', nargs='*', default=["%s:B01234" % FIRST_PORT], 
        help="[PORT:]HEXID pairs, specifies which ports (both TCP and UDP) to listen on, and what ID (first three bytes of Mi-Light packet)" +
        "to use for sending radio commands. ID is in hexadecimal in transmission order. Port numbers are optional and will be assigned starting with %s." % FIRST_PORT)

    args = parser.parse_args()
    OpenMiLiDaemon( args ).start()
