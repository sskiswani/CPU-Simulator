	lw	0	1	five	// This file should fail
	lw	1	2	3	// because start is used
start	add	1	2	1	// twice, as a label.
	beq	0	1	2	// 
	beq	0	0	start	// 
	noop
done	halt				// 
five	.fill	5			//
neg1	.fill	-1			// 
start	.fill	start			// 