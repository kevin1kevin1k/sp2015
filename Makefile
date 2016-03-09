all: test

%: %.c
	gcc $< -o $@

rm:
	rm -f test
