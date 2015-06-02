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
    0x4D: 0x0D, # Disco mode
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
DISCO_CANCEL_COMMANDS = [e + 0x11 for e in range(10)] + [0x0F]

class RemoteState(object):
    def __init__(self, sender_id):
        self.sender_id = sender_id
        self.counter = 0
        self.val1 = 0
        self.val2 = 0
        self.disco_mode = None
    
    def handle_message(self, serial_fd, message):
        message = map(ord, message)
        
        if message[0] in STATIC_COMMAND_MAP:
            if message[0] == 0x4D:
                if self.disco_mode is None:
                    self.disco_mode = 0
                else:
                    self.disco_mode = (self.disco_mode + 1) % 9
            self.send_milight(serial_fd, STATIC_COMMAND_MAP[message[0]], 40)
        elif message[0] == 0x4E:
            self.val2 = (0x90 - (message[1] * 8) + 0x100) & 0xff
            self.send_milight(serial_fd, 0x0e, 30)
        elif message[0] == 0x40:
            self.val1 = (0xC8 - message[1] + 0x100) & 0xFF
            self.send_milight(serial_fd, 0x0f, 30)
    
    def send_milight(self, serial_fd, cmd, repetition=1):
        sender_id = self.sender_id
        if cmd in DISCO_CANCEL_COMMANDS:
            self.disco_mode = None
        if self.disco_mode is not None:
            sender_id = sender_id[0] + ("%i" % self.disco_mode) + sender_id[2:]
        self.counter = (self.counter + 1) & 0xff
        serial_fd.write(sender_id + "%02X%02X%02X%02X " % (self.val1, self.val2, cmd, self.counter))
        for i in range(repetition-1):
            serial_fd.write(".")
        serial_fd.write("\r\n")

class OpenMiLiDaemon(object):
    def __init__(self, args):
        self.args = args
        self.remotes = {}
    
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
                
                sender_id = "".join(sender_id.split()).upper()
                if sender_id in self.remotes:
                    remote = self.remotes[sender_id]
                else:
                    remote = RemoteState(sender_id)
                    self.remotes[sender_id] = remote
                
                if False: # Not implemented yet
                    tcpsock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                    tcpsock.bind(('', port_number))
                    tcpsock.listen(3)
                    sockets.append( (tcpsock, remote) )
                
                udpsock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
                udpsock.bind(('', port_number))
                sockets.append( (udpsock, remote) )
            
            while True:
                r, _, _ = select.select( [fd] + [e for (e, _) in sockets], [], [], 60)
                
                if fd in r:
                    print fd.read(1024)
                
                for sock, remote in sockets:
                    if sock in r:
                        message = sock.recv(1024)
                        
                        remote.handle_message(fd, message)
    
    

if __name__ == "__main__":
    
    parser = argparse.ArgumentParser(description='OpenMiLi daemon -- Listen for commands on the network and control Mi-Light bulbs.')
    parser.add_argument('--serial', '-s', help="Device node for the serial line to connect through", default="/dev/ttyUSB0")
    parser.add_argument('--baudrate', '-b', help="Baud rate for the serial line", type=int, default=115200)
    parser.add_argument('idports', nargs='*', default=["%s:B01234" % FIRST_PORT], 
        help="[PORT:]HEXID pairs, specifies which ports (both TCP and UDP) to listen on, and what ID (first three bytes of Mi-Light packet)" +
        "to use for sending radio commands. ID is in hexadecimal in transmission order. Port numbers are optional and will be assigned starting with %s." % FIRST_PORT)

    args = parser.parse_args()
    OpenMiLiDaemon( args ).start()
