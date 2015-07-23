CC = gcc
CFLAGS = -I . -g -O0 -Wall -std=c99
TARGETS = anonymous_read anonymous_write anonymous_read_stress

all: $(TARGETS)

$(TARGETS): $(addsuffix .o,$@) hmm_test_framework.o

%.o: %.c hmm_test_framework.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGETS)
	rm -f *.o
