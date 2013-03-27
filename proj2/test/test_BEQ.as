	lw	0	1	five	
	lw	0	2	neg1
start	add	1	2	1
	beq	0	1	done
	beq	0	0	start
done	noop	
	halt
five	.fill	5
neg1	.fill	-1