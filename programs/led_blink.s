ldr w2, gpioselect_addr
ldr w4, waittime
ldr w5, high_addr
ldr w7, low_addr
ldr w8, gpout_config

setout: 
	ldr w1, gpio_config	
	str w1, [x2]

mov w6, w8

sethigh:
	str w6, [x5]
	mov w9, #0x1
	b wait

setlow:
	str w6, [x7]
	mov w9, #0x0
	b wait

wait:
	mov w3, #0x0
	
	waitloop:
		add w3, w3, #1
		cmp w4, w3
		b.ge waitloop

	cmp w9, wzr
	b.eq sethigh
	b setlow

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
gpout_config:
	.int #0x8
