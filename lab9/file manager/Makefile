all: file_manager

file_manager: app.c panel.c file_sys.c
	gcc app.c panel.c file_sys.c -lncurses -o file_manager

clean:
	rm -f file_manager
