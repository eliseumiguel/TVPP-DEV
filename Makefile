BIN_DIR=./bin

all:
	mkdir -p bin
	make -C src/
	
clean:
	find . -name "*.o" | xargs rm
	rm -f $(BIN_DIR)/client $(BIN_DIR)/bootstrap $(BIN_DIR)/sclient $(BIN_DIR)/sbootstrap

