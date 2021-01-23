CC := clang
CCFLAGS := `pkg-config --cflags gtk4`
LDFLAGS := -lm `pkg-config --libs gtk4`

TARGETS:= main pisim-ui
MAIN   := main.o pisim-ui.o
OBJ    := pisim-ui.o pisim.o main.o
DEPS   :=

.PHONY: all clean

all: $(TARGETS)

clean:
		rm -f $(TARGETS) $(OBJ)

$(OBJ): %.o : %.c $(DEPS)
		$(CC) -c -o $@ $< $(CCFLAGS)

$(TARGETS): % : $(filter-out $(MAIN), $(OBJ)) %.o
		$(CC) -o $@ $(LIBS) $^ $(CCFLAGS) $(LDFLAGS)
