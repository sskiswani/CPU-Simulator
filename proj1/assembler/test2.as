store	noop				// will become 1
store2	noop				// will stay nothing.
	beq 	0	0	start	// branching test
	lw	0	0	one	// reg[0] == 1 if branch doesn't work.
done	halt				// end. reg[1] == 1 == reg[2] if everything worked
	noop				// and if it didn't, reg[0] == 1 will happen.
	noop
one	.fill	1			// should not interfere with branching
	noop				
start	lw 	0	1	one	// reg[1] == 1
	cmov	1	0	0	// test of cmov
	beq 	0	0	jump	
	halt				// reg[2] == 0 if branch didn't work
jump	sw	0	1	store 	// put 1 somewhere
	lw	0	2	store 	// load 2 from somewhere
	beq	1	2	done	// terminate.