#include <stdlib.h>

#include "csv.h"
#include "arena.h"

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

static bool _textdata_character(unsigned char character, unsigned char delimiter) {
  return (character != delimiter &&
          (character == 32 ||
          character == 33 ||
          (character >= 35 && character <= 43) ||
           character >= 45));
}

static bool _quoted_string(char* position, unsigned char delimiter) {
  unsigned character = *position;
  return
    _textdata_character(character, delimiter) ||
    character == 44 ||
    character == 13 ||
    character == 10;
}

static int _push_entry(arena_t* arena,
                       struct csv_t* csv,
                       const char* const source,
                       size_t length) {

  if (source == NULL) {
    csv->rows.content[csv->rows.count++] = NULL;
    return 0;
  }

  if (csv->rows.count + 1 >= csv->rows.capacity) {
    size_t capacity = csv->rows.capacity * 2;
    csv->rows.content = realloc(csv->rows.content, sizeof(char*) * capacity * csv->column_count);
    csv->rows.capacity = capacity;
  }

  csv->rows.content[csv->rows.count++] =
    arena_string_with_null(arena, source, length + 1);

  return 0;
}

// "" | """" / must be consecutive/ | "\""
static char* _parse_quoted_item(const char* position, unsigned char delimiter) {
  char* next = (char*)position;
  while (_quoted_string(++next, delimiter) == true ||
         (*next++ == 0x22 && *next == 0x22));
  return next;
}

static char* _parse_item(const char* position, unsigned char delimiter) {
  while (*position++ != 0 && _textdata_character(*position, delimiter));
  return (char*)position;
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

  switch (*position) {
  case '\r': {
    if (*(position+1) != '\n') {
      *item = CSV_PARSER_INVALID;
      return 0;
    }
    *item = CSV_PARSER_EOL;
    return 2;
  } break;
  case '\n':
    *item = CSV_PARSER_EOL;
    return 1;
  case '"': {
    char* end = _parse_quoted_item(position, delimiter);
    *item = CSV_PARSER_QUOTED_ITEM;
    return end - position;
  } break;
  default: {
    char* end = _parse_item(position, delimiter);
    *item = CSV_PARSER_ITEM;
    return end - position;
  } break;
  }

  return 0;
}

static size_t _count_columns(const char* position, unsigned char delimiter) {
  size_t item_length = 0;
  char* next = NULL;
  enum csv_parser_item_t item = CSV_PARSER_INIT;
  size_t columns = 0;
  bool expecting_item = true;

  while ((item_length = _parse(position, &item, delimiter)), item != CSV_PARSER_EOF) {
    switch (item) {
    case CSV_PARSER_QUOTED_ITEM:
    case CSV_PARSER_ITEM: {
      ++columns;
      expecting_item = false;
    } break;
    case CSV_PARSER_SEPARATOR: {
      if (expecting_item) {
        ++columns;
      }
      expecting_item = true;
    } break;
    case CSV_PARSER_EOL: {
      if (expecting_item) {
        ++columns;
      }
      return columns;
    } break;
    default:;
    }

    position += item_length;
  }

  return columns + expecting_item;
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
  bool expecting_value = true;
  enum csv_parser_item_t item = CSV_PARSER_INIT;

  while ((item_length = _parse(position, &item, delimiter)), item != CSV_PARSER_EOF) {
    switch (item) {
    case CSV_PARSER_QUOTED_ITEM:
    case CSV_PARSER_ITEM: {
      _push_entry(arena, csv, position, item_length);
      expecting_value = false;
    } break;
    case CSV_PARSER_EOL: {
      if (expecting_value) {
        _push_entry(arena, csv, NULL, 0);
      }
      csv->row_count += *(position + item_length) != 0;
      expecting_value = true;
    } break;
    case CSV_PARSER_SEPARATOR: {
      if (expecting_value) {
        _push_entry(arena, csv, NULL, 0);
      } else {
        expecting_value = true;
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
