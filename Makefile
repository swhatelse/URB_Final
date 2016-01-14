CC=gcc
CFLAGS=-g  
LDLIBS=`pkg-config --cflags --libs glib-2.0` -lpthread
BIN=bin
TEST_BIN=$(BIN)/tests
SRC=src
TEST_SRC=$(SRC)/tests

.PHONY: directories clean clean_tests

all: directories main tests

directories:
	mkdir -p $(BIN) $(TEST_BIN)

tests: test_server test_client

main: $(SRC)/main.c $(SRC)/common.c $(SRC)/group.c $(SRC)/listener.c $(SRC)/communication.c $(SRC)/node.c
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^ $(LDLIBS)

test_server: $(TEST_SRC)/test_server.c $(SRC)/listener.c $(SRC)/common.c $(SRC)/communication.c $(SRC)/group.c $(SRC)/node.c 
	$(CC) $(CFLAGS) -o $(TEST_BIN)/$@ $^ $(LDLIBS)

test_client: $(TEST_SRC)/test_client.c $(SRC)/group.c $(SRC)/listener.c $(SRC)/common.c $(SRC)/communication.c $(SRC)/node.c 
	$(CC) $(CFLAGS) -o $(TEST_BIN)/$@ $^ $(LDLIBS)

test_ack: $(TEST_SRC)/test_ack.c $(SRC)/group.c $(SRC)/common.c $(SRC)/communication.c $(SRC)/node.c 
	$(CC) $(CFLAGS) -o $(TEST_BIN)/$@ $^ $(LDLIBS)
clean:
	rm -rf $(BIN)/main

clean_tests:
	rm -rf $(TEST_BIN)/test_server $(TEST_BIN)/test_client $(TEST_BIN)/test_ack
