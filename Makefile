CC = gcc
CFLAGS = -I . -g -O0 -Wall -std=c99
TARGETS = sanity_anonr sanity_anonw stress_anonr

all: $(TARGETS)

$(TARGETS): $(addsuffix .o,$@) hmm_test_framework.o

%.o: %.c hmm_test_framework.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGETS)
	rm -f *.o
