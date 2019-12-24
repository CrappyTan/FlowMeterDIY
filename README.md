"# FlowMeterDIY" 

This uses a flow meter such as this: https://www.banggood.com/DN20-G34-Copper-Water-Flow-Sensor-Pulse-Output-1_75Mpa-245Lmin-Flowmeter-p-1266296.html?rmmds=buy&cur_warehouse=CN
It measures the current flow rate and publishes, via mqtt, the flow rate in litres/minute.
There is a calibration value at the top of the code called calibrationFactor. This needs adjusting to compensate for your meter.

This is all run on a Wemos Mini with a soldering board backpack. 


1. Get everything hooked up and running.
2. Ensure #define _DEBUG is set.
3. Connect the flow meter up to a hose and get a measuring jug.
4. Slowly open the tap and measure out 500mL of water and compare that to what you're seeing in the debug.
5. Compare the difference and adjust the 8.0 up or down to suit.



The temperature measurements are done using the Dallas one-wire sensors and also published. At startup it polls the number of sensors and runs through them.
The published topic is configurable (in code) and the sensors will publish to that + serialnumber. 
If you do not put any sensors in it will use find them and not publish. No harm, no foul.


Pins:
	One-wire Sensor pins on are D2
	Flow hall-Effect is on pin zero (Sorry, don't know what pin mapping that is)