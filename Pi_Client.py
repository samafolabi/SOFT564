#!/usr/bin/python3

import RPi.GPIO as GPIO
import I2C_LCD_Driver as LCD
import time
import socket
import select

lcd = LCD.lcd()

L1 = 5
L2 = 6
L3 = 13
L4 = 19

C1 = 12
C2 = 16
C3 = 20
C4 = 21

GPIO.setmode(GPIO.BCM)

GPIO.setup(L1, GPIO.OUT)
GPIO.setup(L2, GPIO.OUT)
GPIO.setup(L3, GPIO.OUT)
GPIO.setup(L4, GPIO.OUT)

GPIO.setup(C1, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(C2, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(C3, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(C4, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)

def readLine(line, characters):
    x = 0
    GPIO.output(line, GPIO.HIGH)
    if(GPIO.input(C1) == 1):
        x = characters[0]
    if(GPIO.input(C2) == 1):
        x = characters[1]
    if(GPIO.input(C3) == 1):
        x = characters[2]
    if(GPIO.input(C4) == 1):
        x = characters[3]
    GPIO.output(line, GPIO.LOW)
    return x

def getChar():
    x = 0
    x = readLine(L1, ["1","2","3","A"])
    x = readLine(L2, ["4","5","6","B"])
    x = readLine(L3, ["7","8","9","C"])
    x = readLine(L4, ["*","0","#","D"])
    time.sleep(0.1)
    return x

socked = False
def send(msg,s):
    if socked: s.sendall(msg.encode())

try:
    lcd.lcd_clear()
    lcd.lcd_display_string("Connected", 0,0)
    input = ""
    angle = ""
    ang = False
    start = False
    socked = False
    t = None
    with socket.socket() as s:
        while True:
            if t is not None:
                if t[0]:
                    da = s.recv(128)
                    lcd.lcd_clear()
                    lcd.lcd_display_string("Server:", 0,0)
                    lcd.lcd_display_string(da.decode(),1,0)

            """get server message"""
            x = getChar()
            if x == 0:
                continue
            elif x == "*":
                if start:
                    start = True
                    input = ""
                else:
                    start = True
            elif start:
                if x == "A":
                    input = "FWD"
                    lcd.lcd_clear()
                    lcd.lcd_display_string("Forward", 0, 0)
                elif x == "B":
                    input = "BWD"
                    lcd.lcd_clear()
                    lcd.lcd_display_string("Backward", 0, 0)
                elif x == "C":
                    input = "LFT"
                    lcd.lcd_clear()
                    lcd.lcd_display_string("Left", 0, 0)
                elif x == "D":
                    input = "RGT"
                    lcd.lcd_clear()
                    lcd.lcd_display_string("Right", 0, 0)
                elif x == "#":
                    if not input:
                        s.connect(('192.168.43.21', 9000 ))
                        s.setblocking(0)
                        t = select.select([s], [], [], 1)
                        socked = True
                        send("_S_SUBDRSU_S_",s)
                        lcd.lcd_clear()
                        lcd.lcd_display_string("Subscribed", 0, 0)
                    else:
                        send(input,s)
                        input = ""
                        lcd.lcd_clear()
                        lcd.lcd_display_string("Sent", 0, 0)
            elif x == "A":
                input = "ROL"
                lcd.lcd_clear()
                lcd.lcd_display_string("Rotate Left", 0, 0)
            elif x == "B":
                input = "ROR"
                lcd.lcd_clear()
                lcd.lcd_display_string("Rotate Right", 0, 0)
            elif x == "C":
                input = "ULT"
                lcd.lcd_clear()
                lcd.lcd_display_string("Ultra Distance", 0, 0)
            elif x == "D":
                input = "SRV"
                angle = ""
                lcd.lcd_clear()
                lcd.lcd_display_string("Servo Angle:", 0, 0)
            elif x == "#":
                y = input
                if input == "D": y += angle
                send(y,s)
                input = ""
                lcd.lcd_clear()
                lcd.lcd_display_string("Sent", 0, 0)
                angle = ""
                input = ""
            elif (x == "1" or x == "2" or x == "3" or
                x == "4" or x == "5" or x == "6" or
                x == "7" or x == "8" or x == "9" or x == "0"):
                if len(angle)<3:
                    angle += x
                    lcd.lcd_display_string(x, 0, 12+len(angle))
        print("Closing connection")
except KeyboardInterrupt:
    print("\nApplication stopped!")

