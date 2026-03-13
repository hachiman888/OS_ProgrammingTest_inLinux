src = $(wildcard ./*.cpp)
obj = $(patsubst %.cpp,%.out,$(src))

%.out:%.cpp
	g++ $< -o $@


clean:
	-rm -rf $(obj)


.PHONY: clean


