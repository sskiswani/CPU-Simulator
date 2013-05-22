	lw	0	1	five	// This file should fail
	lw	1	2	3333333	// because an offset label
start	add	1	2	1	// is insanely large
	beq	0	1	2	// 
	beq	0	0	2621441	// specifically this one.
	noop
done	halt				// 
five	.fill	5			//
neg1	.fill	-1			// 
	.fill	start			// 