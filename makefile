PLACE=place
PathOf=$(GET)/cloud/programming/$(USERNAME)/primary/c/source/engine/$(PLACE)
SelfOf=$(GET)/cloud/programming/$(USERNAME)/primary/c/release
PeopleOf=$(GET)/cloud/programming/people/$(SYSTEM)/c/release/plugin/x$(PLATFORM)

Include=-I"$(PeopleOf)/include"\
	    -I"$(SelfOf)/include"  \
		-I"$(PathOf)/include"

Lib=-L$(PeopleOf)/lib

Dependency=-lallegro 	   		\
		   -lallegro_image 		\
		   -lallegro_font  		\
		   -lallegro_ttf   		\
		   -lallegro_primitives \
		   -lallegro_video

.SILENT: $(PLACE)
all: $(PLACE)

$(PLACE):
	gcc $(Include) *.c "$(PathOf)/src/"*.c "$(PathOf)/src/plugin/"*.c "$(PathOf)/src/plugin/graphic/"*.c -o $(PLACE)_x$(PLATFORM) $(Lib) $(Dependency)

clean:
	rm -rf *.o $(PLACE) *~