Handling interrupts from GPIO In many cases, a GPIO input can be configured to generate an interrupt when it changes state, 
which allows you to wait for the interrupt rather than polling in an inefficient software loop. 
If the GPIO bit can generate interrupts, the file edge exists. Initially, it has the value none , meaning that it does not generate interrupts
To enable interrupts, you can set it to one of these values: 
• rising: Interrupt on rising edge 
• falling: Interrupt on falling edge 
• both: Interrupt on both rising and falling edges 
• none: No interrupts (default) You can wait for an interrupt using the poll() function with POLLPRI as the event. 
If you want to wait for a rising edge on GPIO 48, you first enable interrupts:

#echo 48 > /sys/class/gpio/export
#echo rising > /sys/class/gpio/gpio48/edge

