JAVAC ?= javac
JAVA ?= java

SRC := Main.java
OUT_DIR := out

.PHONY: all run clean

all: $(OUT_DIR)/Main.class

$(OUT_DIR)/Main.class: $(SRC)
	@mkdir -p $(OUT_DIR)
	$(JAVAC) -d $(OUT_DIR) $(SRC)

run: all
	$(JAVA) -cp $(OUT_DIR) Main

clean:
	rm -rf $(OUT_DIR)
