all: 
	$(MAKE) -C Client
	$(MAKE) -C Server
clean:
	cd Client; make clean
	cd Server; make clean
	rm WTFTest 
test:
	gcc -o WTFTest WTFTest.c
