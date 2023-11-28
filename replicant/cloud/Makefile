MODULE_NAME := cloud

CPPSOURCES :=  main.cpp Logger.cpp CloudAPI.cpp CloudDB.cpp CloudDB-IDMap.cpp CloudDB-Media.cpp CloudDB-Step.cpp CloudSocket.cpp CloudThread.cpp Config.cpp DevicesList.cpp post.cpp Pull-Announce.cpp Transaction-Announce.cpp TransactionQueue.cpp Transaction-Reset.cpp
CSOURCES := sha1.c sqlite/sqlite3.c
CPPSOURCES += JSON-Builder.cpp JSON-KeyValue.cpp JSON-Tree.cpp JSON-Value.cpp

LDFLAGS =-lc -lpthread -lrt -lz -fvisibility=hidden

LIBRARY_FILENAME := lib$(MODULE_NAME).w6c
OUTPUT_PATH := ../build/$(MODULE_NAME)
ARCHIVE_PATH := ../build/lib
LIBRARY_FILEPATH := ../build/lib/$(LIBRARY_FILENAME)

CPPOBJS := $(patsubst %.cpp,$(OUTPUT_PATH)/%.o,$(CPPSOURCES)) 
CPPDEPS := $(patsubst %.o,$(OUTPUT_PATH)/%.d,$(CPPOBJS))
COBJS := $(patsubst %.c,$(OUTPUT_PATH)/%.o,$(CSOURCES)) 
CDEPS := $(patsubst %.o,$(OUTPUT_PATH)/%.d,$(COBJS))

OBJS := $(CPPOBJS) $(COBJS)
DEPS := $(CPPDEPS) $(CDEPS)

CFLAGS=-I.. -fPIC -DSQLITE_THREADSAFE=2 -fvisibility=hidden
CPPFLAGS := ${CFLAGS}

build: build-dir $(LIBRARY_FILEPATH)

build-dir: 
	@mkdir -p $(OUTPUT_PATH) > /dev/null 2> /dev/null
	@mkdir -p $(ARCHIVE_PATH) > /dev/null 2> /dev/null

dep:
	@rm ${DEPS}

$(OUTPUT_PATH)/%.o: %.cpp
	@echo Compiling $*.cpp
	@$(CXX) $(CPPFLAGS) -MMD -MF $(OUTPUT_PATH)/$*.d -MT $(OUTPUT_PATH)/$*.o -c $*.cpp -o $(OUTPUT_PATH)/$*.o

$(OUTPUT_PATH)/%.o: %.c
	@echo Compiling $*.cpp
	@$(CC) $(CFLAGS) -MMD -MF $(OUTPUT_PATH)/$*.d -MT $(OUTPUT_PATH)/$*.o -c $*.c -o $(OUTPUT_PATH)/$*.o

$(LIBRARY_FILEPATH): ${OBJS}
	@$(CC) -Wl,-L,'../build/lib' -Wl,-rpath,'$$ORIGIN' -shared -Wl,-soname,libcloud.w6c -o $(LIBRARY_FILEPATH) ${OBJS} ${LDFLAGS} -lnswasabi -ljnet -lnx  -lnu -lyajl

clean:
	-rm -f ${OBJS} $(LIBRARY_FILENAME) ${DEPS} 

-include $(DEPS)
