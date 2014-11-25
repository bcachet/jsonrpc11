OUTPUT_DIR=build
MAKE_CMD=cd $(OUTPUT_DIR) && cmake --build . --target

all: tests
.PHONY: tests docs

tests: $(OUTPUT_DIR)/CMakeFiles
	@$(MAKE_CMD) tests
	bin/tests -r console

clean: $(OUTPUT_DIR)/CMakeFiles
	@$(MAKE_CMD) clean
	@rm -rf $(OUTPUT_DIR)/reports/tests/* $(OUTPUT_DIR)/reports/coverage/*
	@rm -rf $(OUTPUT_DIR)/html $(OUTPUT_DIR)/doxygen

docs: $(OUTPUT_DIR)/CMakeFiles
	@sphinx-build -b html docs $(OUTPUT_DIR)/html
	@doxygen docs/Doxyfile

prepare: $(OUTPUT_DIR)/CMakeFiles
	@$(MAKE_CMD) catch
	@$(MAKE_CMD) hippomocks
	@-pip install -r docs/requirements.txt

$(OUTPUT_DIR)/CMakeFiles:
	@mkdir -p $(OUTPUT_DIR)
	@cd $(OUTPUT_DIR) && cmake ..
	@make prepare
