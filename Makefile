brun:
	$(MAKE) build
	$(MAKE) run

build:
	cmake -S . -G "Unix Makefiles" -B out
	cd out; make

run:
	out/renderer

