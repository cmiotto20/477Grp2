# 477Grp2
MOUSE is a mobile surveillance robot intended for indoor use, in corporate office or residential environments. MOUSE will be powered by four motors, each controlling its own wheel for mobility. There will be a large 12V battery allowing MOUSE to perform surveillance overnight without needing to have its battery replaced. When the battery does die, it will need to be recharged manually. MOUSE will have the ability for the motors to have variable speed control through the use of PWM signals. MOUSE will also have a custom circuit to step down the 12V from the battery to 5V for motors, sensors and ICs, aswell as 5V to 3.3V for the microprocessor.


To complete its surveillance functionality, MOUSE will have ultrasonic sensors on all four sides, allowing for the detection of movement. While stationary, MOUSE will use these sensors to detect any movement in its vicinity. Finally, there will be a viewable UI on a webpage to use MOUSE movement controls and display if any objects have been detected. MOUSE and the web interface will send data wirelessly through WiFi via a web server. The block diagram below represents the connections between the described functional components of MOUSE.


Additionally, we have the following stretch goals for the functionality of MOUSE. Mouse may have a series of LEDs which will be used to display diagnostics such as battery life. There may be an ability to measure data from a lidar sensor to provide redundancy and confirm the readings from the ultrasonic sensors. Finally, MOUSE may be able to follow a predetermined path set by the user.
