CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -I./include
TARGET  = promptforge
SRCDIR  = src
SRCS    = $(SRCDIR)/main.c \
          $(SRCDIR)/color.c \
          $(SRCDIR)/segments.c \
          $(SRCDIR)/config.c \
          $(SRCDIR)/prompt.c \
          $(SRCDIR)/init.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean install uninstall preview

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.c include/promptforge.h
	$(CC) $(CFLAGS) -c -o $@ $<

# インストール (~/bin または /usr/local/bin)
install: $(TARGET)
	@mkdir -p $(HOME)/bin
	cp $(TARGET) $(HOME)/bin/$(TARGET)
	@echo "インストール完了: $(HOME)/bin/$(TARGET)"
	@echo "PATH に $(HOME)/bin が含まれていることを確認してください"

uninstall:
	rm -f $(HOME)/bin/$(TARGET)
	@echo "アンインストール完了"

# ターミナルでプレビュー表示
preview: $(TARGET)
	./$(TARGET) preview

clean:
	rm -f $(OBJS) $(TARGET)
