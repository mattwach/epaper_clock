void text_init(struct Text* text, const void* font, struct OLEDM* display) {
  text->font = font;
  text->display = display;
  text->row = 0;
  text->column = 0;
  text->options = 0;
}

void text_str(struct Text* text, const char* str) {
  text_strLen(text, str, strlen(str));
}
