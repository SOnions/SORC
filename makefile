ifeq ($(OS),Windows_NT)
	CLOPTIONS := -lws2_32
else
	CLOPTIONS := 
endif


all:
	g++ src/*.cpp $(CLOPTIONS) -o Debug/SORC.exe