.PHONY: clean

ansitee: ansitee.c
	$(CC) -o ansitee $<
clean:
	rm -f ansitee
