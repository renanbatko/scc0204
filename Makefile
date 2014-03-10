all:
	g++ -o shell shell.cc -lpcre

run:
	./shell

clean:
	rm -rf *~
