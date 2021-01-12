import socket
import select
import sys

#From: https://stackoverflow.com/questions/34984443/using-select-method-for-client-server-chat-in-python

while True:
	#Start socket with auto closing at the end
	with socket.socket() as s:
		#Connect to server
		s.connect(('192.168.43.21', 9000 ))
		print("Connected")

		#Switch between the user input and server
		#If there is data on the server, print the message
		#If there is data on the client, send it
		while True:
			list = [sys.stdin, s]
			r, w, e = select.select(list, [], [])

			for k in r:
				if k == s:
					data = k.recv(128)
					if not data:
						print('Closing connection')
						break
					else:
						print("New message: " + str(data))
				else:
					com = input("")
					s.sendall(com.encode())

		s.close()
