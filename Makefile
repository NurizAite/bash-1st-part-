%: %.c
	gcc $@.c -o $@
	./$@