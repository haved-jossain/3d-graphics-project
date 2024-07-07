NAME=Hossain

EX=introspection

# Set up the appropriate flags
CFLAGS = -Wall -O3 
LDFLAGS = -lglut -lGLU -lGL -lm -lSDL2 -lSDL2_mixer
RM=rm
CLEAN = rm -f $(EX) *.o *.a

ifeq ($(OS),Windows_NT)
	LDFLAGS = -lglut32cu -lglu32 -lopengl32
	CLEAN = del *.exe *.o *.a
	RM=del
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		LDFLAGS = -framework GLUT -framework OpenGL -framework SDL2 -framework SDL2_mixer
	endif
endif

#  Main target
all: $(EX)

#  Generic compile rules
.c.o:
	gcc $(CFLAGS) -c $<

#  Generic compile and link
%: %.c CSCIx229.a
	gcc $(CFLAGS) -o $@ $^ $(LDFLAGS)

#  Delete unwanted files
clean:;$(CLEAN)

cleanall:clean
	$(RM) $(NAME)_$(EX).tar.gz

#  Make upload file
submit: clean
	tar cvfz $(NAME)_$(EX).tar.gz --exclude="$(NAME)_$(EX).tar.gz" ./*

#  Create archive
CSCIx229.a:fatal.o loadtexbmp.o print.o project.o errcheck.o object.o
	ar -rcs CSCIx229.a $^

#  Obligatory Unix Joke
love:
	@echo "not war?"
