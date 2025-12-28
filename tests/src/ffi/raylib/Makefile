S3C=../../build/s3c
LDFLAGS= \
		 -L$(HOME)/Programming/usr/src/Odin/vendor/raylib/linux/ -lraylib \
		 -L../../utilities/string/ -lstring \
		 -rpath=$(HOME)/Programming/usr/src/Odin/vendor/raylib/linux/:../../utilities/print/:../../utilities/string/

prog: main.3
	$(S3C) -o $@ $^ $(LDFLAGS)
