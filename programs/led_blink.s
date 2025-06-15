ldr w2, gpioselect_addr
ldr w4, waittime
ldr w5, high_addr
ldr w7, low_addr

setout: 
	ldr w1, gpio_config	
	str w1, [x2]

mvn w6, #0x0

sethigh:
	str w6, [x5]

mov w3, #0x0

wait1:
	add w3, w3, #1
	cmp w4, w3
	b.ge wait1 

setlow:
	str w6, [x7]

mov w3, #0

wait2:
	add w3, w3, #1
	cmp w4, w3
	b.ge wait2 

b sethigh

and x0, x0, x0

gpioselect_addr:
	.int #0x3f200000
high_addr:
	.int #0x3f20001c
low_addr:
	.int #0x3f200028
waittime:
	.int #0x00050000
gpio_config:
        .int #0x200
