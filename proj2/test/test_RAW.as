	lw	0	1	data1	# R1 = 1
	lw	0	2	data2	# R2 = 2
	add	1	2	3	# R3 = R1 + R2
	add	2	3	4	# R4 = R2 + R3
	halt
data1	.fill	1
data2	.fill	2
