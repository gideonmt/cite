# Makefile for cite project
# This is a convenience wrapper around CMake

BUILD_DIR = build
BINARY = $(BUILD_DIR)/cite

.PHONY: all clean rebuild install test example help

# Default target
all: $(BINARY)

# Build the project
$(BINARY):
	@echo "Building cite..."
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. && make
	@echo "Build complete: $(BINARY)"

# Clean build artifacts
clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete."

# Rebuild from scratch
rebuild: clean all

# Install to /usr/local/bin (requires sudo)
install: $(BINARY)
	@echo "Installing cite to /usr/local/bin..."
	@sudo cp $(BINARY) /usr/local/bin/cite
	@echo "Installation complete. You can now run 'cite' from anywhere."

# Run with example file
example: $(BINARY)
	@echo "Exporting example bibliography..."
	@$(BINARY) export example.json chicago

# Show detailed example output
example-html: $(BINARY)
	@echo "Creating example HTML bibliography..."
	@$(BINARY) export example.json chicago example_output.html
	@echo "Created example_output.html"

example-md: $(BINARY)
	@echo "Creating example Markdown bibliography..."
	@$(BINARY) export example.json chicago example_output.md
	@echo "Created example_output.md"

# Test the add command (interactive)
test-add: $(BINARY)
	@echo "Testing add command (will prompt for input)..."
	@$(BINARY) add test_bibliography.json

# Help message
help:
	@echo "cite - Citation Management Tool"
	@echo ""
	@echo "Available targets:"
	@echo "  make              - Build the project"
	@echo "  make clean        - Remove build artifacts"
	@echo "  make rebuild      - Clean and rebuild"
	@echo "  make install      - Install to /usr/local/bin (requires sudo)"
	@echo "  make example      - Run with example.json (terminal output)"
	@echo "  make example-html - Generate example HTML output"
	@echo "  make example-md   - Generate example Markdown output"
	@echo "  make test-add     - Test the add command interactively"
	@echo "  make help         - Show this help message"
	@echo ""
	@echo "Usage after building:"
	@echo "  ./build/cite add <file.json>"
	@echo "  ./build/cite export <file.json> chicago [output.html|output.md]"
