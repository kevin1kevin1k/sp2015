TARGET = merger

all: $(TARGET)

%: %.c
	gcc -pthread -Wall $< -o $@

run: $(TARGET)
	./$< 6 < input

rm:
	rm -f $(TARGET)
