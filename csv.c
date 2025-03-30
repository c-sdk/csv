#include <stdlib.h>

#include "status.h"
#include "arena.h"

#include "csv.h"

// "" 0 ok
// "\r\n" 0
// "a\r\n" 1
// "a" 1
// "a,b" 2

enum csv_parser_item_t {
  CSV_PARSER_INVALID = -1,
  CSV_PARSER_INIT,
  CSV_PARSER_QUOTED_ITEM,
  CSV_PARSER_ITEM,
  CSV_PARSER_SEPARATOR,
  CSV_PARSER_EOL,
  CSV_PARSER_EOF
};

static int _textdata_character(unsigned char character, unsigned char delimiter) {
  return (character != delimiter &&
          (character == 32 ||
           character == 33 ||
           (character >= 35 && character <= 43) ||
           character >= 45));
}

static int _quoted_string(char* position, unsigned char delimiter) {
  unsigned character = *position;
  return (_textdata_character(character, delimiter) ||
          character == 10 ||
          character == 13 ||
          character == 44);
}

static enum status_t _push_entry(arena_t* arena,
                                 struct csv_t* csv,
                                 const char* const source,
                                 size_t length) {
  if (source == NULL) {
    csv->rows.content[csv->rows.count++] = NULL;
    return STATUS_OK;
  }

  if (csv->rows.count + 1 >= csv->rows.capacity) {
    size_t capacity = csv->rows.capacity * 2;
    csv->rows.content = realloc(csv->rows.content,
                                sizeof(char*) * capacity * csv->column_count);
    csv->rows.capacity = capacity;
  }

  csv->rows.content[csv->rows.count++] =
    arena_string_with_null(arena, source, length + 1);

  return STATUS_OK;
}

// "" | """" / must be consecutive/ | "\""
static char* _parse_quoted_item(const char* position, unsigned char delimiter) {
  char* next = (char*)position;
  while (_quoted_string(++next, delimiter) ||
         (*next == 0x22 && *++next == 0x22));
  return next;
}

static char* _parse_item(const char* position, unsigned char delimiter) {
  while ((*position != 0) &&
         _textdata_character(*position, delimiter)) {
    ++position;
  }
  return (char*)position;
}

const char* csv_parse_value(const char* position, unsigned char delimiter) {
  return _parse_item(position, delimiter);
}

const char* csv_parse_quoted(const char* position, unsigned char delimiter) {
  return _parse_quoted_item(position, delimiter);
}

const char* csv_parse_separator(const char* position, unsigned char delimiter) {
  return (*position == delimiter) ? position + 1 : NULL;
}

const char* csv_parse_crlf_eol(const char* position, unsigned char delimiter) {
  (void)delimiter;
  return (*position == '\r' && *(position + 1) != 0 && *(position + 1) == '\n') ? position + 2 : NULL;
}

const char* csv_parse_cr_eol(const char* position, unsigned char delimiter) {
  (void)delimiter;
  return *position == '\n' ? position + 1 : NULL;
}

static size_t _parse(const char* const data,
                     enum csv_parser_item_t* item,
                     unsigned char delimiter) {
  char* position = (char*)data;

  if (*position == 0) {
    *item = CSV_PARSER_EOF;
    return 0;
  }

  if (*position == delimiter) {
    *item = CSV_PARSER_SEPARATOR;
    return 1;
  }

  const char* end = NULL;

  switch (*position) {
  case '\r': {
    end = csv_parse_crlf_eol(position, delimiter);
    *item = CSV_PARSER_EOL;
  } break;
  case '\n':
    *item = CSV_PARSER_EOL;
    return 1;
  case '"': {
    end = csv_parse_quoted(position, delimiter);
    *item = CSV_PARSER_QUOTED_ITEM;
  } break;
  default: {
    end = csv_parse_value(position, delimiter);
    *item = CSV_PARSER_ITEM;
  } break;
  }

  return (end == NULL) ? (*item = CSV_PARSER_INVALID, 0) : end - position;
}

static size_t _count_columns(const char* position, unsigned char delimiter) {
  size_t item_length = 0;
  enum csv_parser_item_t item = CSV_PARSER_INIT;
  size_t columns = 0;
  enum status_t expecting_item = STATUS_OK;

  while ((item_length = _parse(position, &item, delimiter)), item != CSV_PARSER_EOF) {
    switch (item) {
    case CSV_PARSER_QUOTED_ITEM:
    case CSV_PARSER_ITEM: {
      ++columns;
      expecting_item = STATUS_FAILURE;
    } break;
    case CSV_PARSER_SEPARATOR: {
      if (status_is_ok(expecting_item)) {
        ++columns;
      }
      expecting_item = STATUS_OK;
    } break;
    case CSV_PARSER_EOL: {
      if (status_is_ok(expecting_item)) {
        ++columns;
      }
      return columns;
    } break;
    default:;
    }

    position += item_length;
  }

  return columns + status_is_ok(expecting_item);
}

int csv_parse_custom_delimiter(arena_t* arena,
                               struct csv_t* csv,
                               const char *const data,
                               unsigned char delimiter) {

  char* position = (char*)data;

  if (*position == 0) {
    return 0;
  }

  csv->column_count = _count_columns(position, delimiter);

  csv->rows.content = (char**)malloc(sizeof(char*) * csv->column_count * 64);
  csv->rows.capacity = 64;
  csv->row_count = 1;

  size_t item_length = 0;
  enum status_t expecting_value = STATUS_OK;
  enum csv_parser_item_t item = CSV_PARSER_INIT;

  while ((item_length = _parse(position, &item, delimiter)), item != CSV_PARSER_EOF) {
    switch (item) {
    case CSV_PARSER_QUOTED_ITEM:
    case CSV_PARSER_ITEM: {
      (void)_push_entry(arena, csv, position, item_length);
      expecting_value = STATUS_FAILURE;
    } break;
    case CSV_PARSER_EOL: {
      if (status_is_ok(expecting_value)) {
        (void)_push_entry(arena, csv, NULL, 0);
      }
      csv->row_count += *(position + item_length) != 0;
      expecting_value = STATUS_OK;
    } break;
    case CSV_PARSER_SEPARATOR: {
      if (status_is_ok(expecting_value)) {
        (void)_push_entry(arena, csv, NULL, 0);
        expecting_value = STATUS_FAILURE;
      } else {
        expecting_value = STATUS_OK;
      }
    } break;
    default:;
    }

    position += item_length;
  }

  return csv->row_count;
}

int csv_parse(arena_t *arena, struct csv_t *csv, const char *const data) {
  return csv_parse_custom_delimiter(arena, csv, data, CSV_COMMA_DELIMITER);
}

char** csv_row(struct csv_t* csv, size_t at) {
  return csv->rows.content + csv->column_count * at;
}

void csv_free(struct csv_t *csv) {
  free(csv->rows.content);
}
