#ifndef _AIL_CSV_H_
#define _AIL_CSV_H_ 1

#include <stddef.h>
#include <stdbool.h>
#include "arena.h"

#define CSV_COMMA_DELIMITER ','

struct csv_row_t {
  char** content;
  size_t capacity;
  size_t count;
};

struct csv_t {
  struct csv_row_t rows;
  size_t column_count;
  size_t row_count;
};

int csv_parse_custom_delimiter(arena_t *arena,
                               struct csv_t *csv,
                               const char *const data,
                               unsigned char delimiter);

int csv_parse(arena_t* arena,
              struct csv_t* csv,
              const char* const data);

char** csv_row(struct csv_t* csv, size_t at);

void csv_free(struct csv_t* csv);

#endif // _AIL_CSV_H_
