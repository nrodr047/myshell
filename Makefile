myshell: myshell.c
	cc myshell.c  -o myshell
clean:
	rm -f myshell *.o core *~
