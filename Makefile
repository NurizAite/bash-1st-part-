%: %.c
	gcc $@.c -o $@
	./$@
