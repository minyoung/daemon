import socket
import struct

MAX_MESSAGE_LENGTH = 1024
MAX_PACKET_LENGTH = MAX_MESSAGE_LENGTH + 4

class DaemonPacket(object):
    VERSION = 1
    FORMAT = struct.Struct("b c h %ss" % MAX_MESSAGE_LENGTH)
    def __init__(self, packet):
        data = DaemonPacket.FORMAT.unpack(packet)
        self.version, self.type, self.message_length, self.message = data
        self.message = self.message[:self.message_length]

    @staticmethod
    def pack(message_type, message):
        message = message[:MAX_MESSAGE_LENGTH]
        return DaemonPacket.FORMAT.pack(DaemonPacket.VERSION, message_type, len(message), message)

    def __repr__(self):
        return repr((self.version, self.type, self.message))

class DaemonClient(object):
    def __init__(self, server_ip, server_port):
        self.version = 1
        self.server = (server_ip, server_port)
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    def send(self, message):
        outgoing = DaemonPacket.pack('E', message)
        self.__send_packet(outgoing)
        data, addr = self.socket.recvfrom(MAX_PACKET_LENGTH)
        incoming = DaemonPacket(data)
        return (incoming, addr)

    def __send_packet(self, packet):
        self.socket.sendto(packet, self.server)


if __name__ == '__main__':
    client = DaemonClient("127.0.0.1", 10180)
    packet, client = client.send("Hello world")
    print packet
    print repr(client)
