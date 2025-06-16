ldr w1, gpio_config
ldr w2, gpioselect_addr
ldr w4, waittime
ldr w5, high_addr
ldr w6, low_addr
ldr w7, gpout_config
ldr w8, high_flag

setout: 
	str w1, [x2]

sethigh:
	str w7, [x5]
	mov w9, w8
	b wait

setlow:
	str w7, [x6]
	mov w9, wzr
	b wait

wait:
	mov w3, wzr
	
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
        .int #0x240
gpout_config:
	.int #0x8
high_flag:
	.int #0x1
