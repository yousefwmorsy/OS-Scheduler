build:
	gcc process_generator.c -o ./Compiled/process_generator.out
	gcc clk.c -o ./Compiled/clk.out
	gcc scheduler.c -o ./Compiled/scheduler.out
	gcc process.c -o ./Compiled/process.out
	gcc test_generator.c -o ./Compiled/test_generator.out

clean:
	rm -f ./Compiled/*.out  processes.txt

all: clean build

run:
	./Compiled/process_generator.out
