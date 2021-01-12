# SOFT564

This project contains an Arduino/ESP32 component, a Web Application, an Android app, and a Python program, all of which have different requirements but the one thing in common is **all the devices must be on the same WiFi**.

The Arduino and ESP32 both only need the Arduino IDE and appropriate wires. If there are any libraries missing, they only need to be installed in the IDE's Manage Libraries menu. It is also important to use a wireless network that allows for these higher ports (9000 and 9001), but also, you can change the SSID and password defined in the ESP32 code to match your own network.

The web application is hosted on a Node.js server, so a local Node.js installation is required to run the code as is. However, it is not crucial to the project, so the files in the public folder can be placed in any web server and hosted there to work. Also make sure your browser supports WebSockets, which the app will let you know.

The Android app is the hardest one, as you will need Android Studio to build then run the app. From there you can use a real phone or a phone emulator on the computer to run the app.

Finally, the Python program only needs an installation fo Python and access to a Terminal or Command Prompt to input commands and receive messages.
