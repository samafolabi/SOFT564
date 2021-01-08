import socket
 
s = socket.socket()         
 
s.bind(('192.168.43.210', 9000 ))
s.listen(2)                 
 
while True:
 
    client, addr = s.accept()
 
    while True:
        content = client.recv(32)
 
        if len(content) ==0:
           break
 
        else:
            print(content)
 
    print("Closing connection")
    client.close()