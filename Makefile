CC = gcc
CFLAGS = -I . -g -O0 -Wall -std=c99
TARGETS = sanity_anon001 sanity_anon002 \
	sanity_file001 sanity_file002 \
	stress_anon001 stress_anon002

all: $(TARGETS)

$(TARGETS): $(addsuffix .o,$@) hmm_test_framework.o

%.o: %.c hmm_test_framework.h
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(TARGETS)
	rm -f *.o
