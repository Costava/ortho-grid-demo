# makefile

CC:=clang
MAIN_EXE:=main.bin

###################################################################################################

run: build
	./$(MAIN_EXE)

build: $(MAIN_EXE)

clean:
	rm -f $(MAIN_EXE)

# `-lm` was added after needing `round` function in <math.h> in order to avoid a compilation error.
# Add `-fopenmp` if OpenMP is used.
$(MAIN_EXE): ./main/main.c ./src/*.c ./src/*.h
	$(CC) ./main/main.c ./src/*.c \
	      --output $@ \
	      -std=c11 -g -I ./src \
	      -Wall -Wextra -Wconversion \
	      -lm -lSDL2 -lSDL2_image
