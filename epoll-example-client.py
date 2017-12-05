#py27
import socket

if __name__ == '__main__':
    for i in range(10):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect(('127.0.0.1',8888))
        s.send("NiHao: %s" % i )
        s.close()
