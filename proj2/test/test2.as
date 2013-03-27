		lw	0	1 	val 	// load reg[1]
		lw	0	2 	val2 	// load reg[2]
		lw	0	3	val3 	// load reg[3]
		cmov	1	1 	4 	// test cmov into reg[4].
		beq	4	1 	more 	// confirm result of cmov.
		lw 	0 	0 	1 		// if reg[0] == 1 then something went wrong.
done	halt 				
val		.fill	4
val2	.fill	27
val3	.fill	1991
val4	.fill 	7214
more	sw	0	1 	val 	// test store word.
		sw 	0 	1 	val2
		sw 	0 	1 	val3 	// exhaustively test store word.
		lw	0	7 	val 	// load saved word into reg[7]
		lw 	0	6	val2 	// test another saved word.
		beq 	6 	7 	done 	// if reg[6] == reg[7] then you succeeded in storing/loading.
		lw 	0	0	1 		// if reg[0] == 1 then something went wrong. 	
		halt