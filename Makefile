all:
	make clean;
	mkdir build;
	gcc dumper/*.c kernel/*.c kernel/analysis/*.c kernel/runtime_parsing/*.c *.c *.m -o build/iokit-dumper -framework IOKit -framework Foundation

clean:
	rm -rf build;
