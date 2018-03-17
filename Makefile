##
# Project Title
#
# @file
# @version 0.1

CFLAGS := -pg -Wall -Wextra -Wpedantic
LDFLAGS := -pg -lSDL2 -lSDL2_image -lpthread

all: main list listnode context desktop rect window button textbox
	gcc *.o $(LDFLAGS) -o sdltest

main: main.c
	gcc -c $(CFLAGS) main.c

list: list.c list.h
	gcc -c $(CFLAGS) list.c

listnode: listnode.c listnode.h
	gcc -c $(CFLAGS) listnode.c

context: context.c context.h
	gcc -c $(CFLAGS) context.c

desktop: desktop.c desktop.h
	gcc -c $(CFLAGS) desktop.c

rect: rect.c rect.h
	gcc -c $(CFLAGS) rect.c

window: window.c window.h
	gcc -c $(CFLAGS) window.c

button: button.c button.h
	gcc -c $(CFLAGS) button.c

textbox: textbox.c textbox.h
	gcc -c $(CFLAGS) textbox.c

run: all
	./sdltest

run-debug: all
	sudo ddd ./sdltest --gdb -command=dddcmd

run-profile: all
	sudo gprof ./sdltest gmon.out > prof_output

clean:
	rm -f *.o
	rm -f sdltest

# end
