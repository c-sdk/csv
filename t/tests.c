#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "utf8.h"
#include "arena.h"
#include "csv.h"

#include "assertion-macros.h"

void test_parse_empty_value(void) {
  const char* data = "";
  const char* end = csv_parse_value(data, ',');
  assert_equal(*data, *end);
}

void test_parse_value(void) {
  const char* data = "abc";
  const char* end = csv_parse_value(data, ',');
  assert_equal(*(data + 3), *end);
}

void test_parse_quoted_value(void) {
  const char* data = "\"abc\"";
  const char* end = csv_parse_quoted(data, ',');
  assert_equal(*(data + 5), *end);
}

void test_parse_separator(void) {
  const char* data = ",";
  const char* end = csv_parse_separator(data, ',');
  assert_equal(*(data + 1), *end);
}

void test_fail_to_parse_separator(void) {
  const char* data = "a,";
  const char* end = csv_parse_separator(data, ',');
  assert_null(end);
}

void test_parse_crlf(void) {
  const char* data = "\r\n";
  const char* end = csv_parse_crlf_eol(data, ',');
  assert_equal(*(data + 2), *end);
}

void test_fail_to_parse_crlf(void) {
  const char* data = "a";
  const char* end = csv_parse_crlf_eol(data, ',');
  assert_null(end);
}

void test_parse_cr(void) {
  const char* data = "\n";
  const char* end = csv_parse_cr_eol(data, ',');
  assert_equal(*(data + 1), *end);
}

void test_fail_to_parse_cr(void) {
  const char* data = "a";
  const char* end = csv_parse_cr_eol(data, ',');
  assert_null(end);
}

void test_empty_string(void) {
  const char *csv_content = "";

  struct csv_t csv = {0};

  csv_parse(NULL, &csv, csv_content);
  assert(csv.row_count == 0);
}

void test_single_value_without_header_without_eol(void) {
  struct csv_t csv = {0};

  arena_t arena = {0};
  arena_create(&arena, 4096);

  const char *csv_content =
    "ok";
  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 1);
  assert(csv.row_count == 1);
  assert(memcmp(csv_row(&csv, 0)[0], "ok", 3) == 0);

  arena_free(&arena);
}

void test_single_quoted_value(void) {
  struct csv_t csv = {0};

  arena_t arena = {0};
  arena_create(&arena, 4096);

  const char *csv_content =
    "\"ok\"";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 1);
  assert(csv.row_count == 1);
  assert(memcmp(csv_row(&csv, 0)[0], "\"ok\"", 5) == 0);

  arena_free(&arena);
}

void test_single_quoted_value_with_new_line(void) {
  struct csv_t csv = {0};

  arena_t arena = {0};
  arena_create(&arena, 4096);

  const char *csv_content =
    "\"o\nk\"";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 1);
  assert(csv.row_count == 1);
  assert(memcmp(csv_row(&csv, 0)[0], "\"o\nk\"", 6) == 0);

  arena_free(&arena);
}

void test_single_quoted_value_with_comma(void) {
  struct csv_t csv = {0};

  arena_t arena = {0};
  arena_create(&arena, 4096);

  const char *csv_content =
    "\"o,k\"";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 1);
  assert(csv.row_count == 1);
  assert(memcmp(csv_row(&csv, 0)[0], "\"o,k\"", 6) == 0);

  arena_free(&arena);
}

void test_single_value_without_header_with_crlf_eol(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "ok\r\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 1);
  assert(csv.row_count == 1);
  assert(memcmp(csv_row(&csv, 0)[0], "ok", 3) == 0);

  arena_free(&arena);
}

void test_single_value_without_header_with_lf_eol(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "ok\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 1);
  assert(csv.row_count == 1);
  assert(memcmp(csv_row(&csv, 0)[0], "ok", 3) == 0);

  arena_free(&arena);
}

void test_single_line_with_2_values_sep_by_comma_without_eol(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "ok,fail";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 2);
  assert(csv.row_count == 1);
  const char** const row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "ok", 3) == 0);
  assert(memcmp(row[1], "fail", 5) == 0);

  arena_free(&arena);
}

void test_single_value_with_crlf_eol(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "ok,fail\r\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 2);
  assert(csv.row_count == 1);
  const char** const row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "ok", 3) == 0);
  assert(memcmp(row[1], "fail", 5) == 0);

  arena_free(&arena);
}

void test_single_line_with_2_values_sep_by_comma_with_lf_eol(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "ok,fail\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 2);
  assert(csv.row_count == 1);
  const char** const row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "ok", 3) == 0);
  assert(memcmp(row[1], "fail", 5) == 0);

  arena_free(&arena);
}

void test_single_line_with_2_values_sep_by_comma_with_quoted(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "ok,\"fail\"\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 2);
  assert(csv.row_count == 1);
  const char** const row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "ok", 3) == 0);
  assert(memcmp(row[1], "\"fail\"", 7) == 0);

  arena_free(&arena);
}

void test_single_line_with_2_values_sep_by_comma_with_scaped_quotes(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "ok,\"fa\"\"il\"\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 2);
  assert(csv.row_count == 1);
  const char** const row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "ok", 3) == 0);
  assert(memcmp(row[1], "\"fa\"\"il\"", 9) == 0);

  arena_free(&arena);
}

void test_single_line_with_3_values_with_escaped_quotes_in_the_middle(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "ok,\"fa\"\"il\",meh\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 3);
  assert(csv.row_count == 1);
  const char** const row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "ok", 3) == 0);
  assert(memcmp(row[1], "\"fa\"\"il\"", 9) == 0);
  assert(memcmp(row[2], "meh", 4) == 0);

  arena_free(&arena);
}

void test_2_rows(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "A,B,C\r\n"
    "D,E,F\r\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 3);
  assert(csv.row_count == 2);
  const char** row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "A", 2) == 0);
  assert(memcmp(row[1], "B", 2) == 0);
  assert(memcmp(row[2], "C", 2) == 0);
  row = (const char** const)csv_row(&csv, 1);
  assert(memcmp(row[0], "D", 2) == 0);
  assert(memcmp(row[1], "E", 2) == 0);
  assert(memcmp(row[2], "F", 2) == 0);

  arena_free(&arena);
}

void test_single_row_with_first_item_empty(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    ",A,B\r\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 3);
  assert(csv.row_count == 1);
  const char** row = (const char** const)csv_row(&csv, 0);
  assert(row[0] == NULL);
  assert(memcmp(row[1], "A", 2) == 0);
  assert(memcmp(row[2], "B", 2) == 0);

  arena_free(&arena);
}

void test_single_row_with_empty_item_before_eol(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "A,B,\r\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 3);
  assert(csv.row_count == 1);
  const char** row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "A", 2) == 0);
  assert(memcmp(row[1], "B", 2) == 0);
  assert(row[2] == NULL);

  arena_free(&arena);
}

void test_single_row_with_empty_item_before_eol_without_proper_eol(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "A,B,";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 3);
  assert(csv.row_count == 1);
  const char** row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "A", 2) == 0);
  assert(memcmp(row[1], "B", 2) == 0);
  assert(row[2] == NULL);

  arena_free(&arena);
}

void test_2_rows_with_empty_item_before_eol_first_row(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "A,B,\r\n"
    "C,D,E\r\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 3);
  assert(csv.row_count == 2);
  const char** row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "A", 2) == 0);
  assert(memcmp(row[1], "B", 2) == 0);
  assert(row[2] == NULL);
  row = (const char** const)csv_row(&csv, 1);
  assert(memcmp(row[0], "C", 2) == 0);
  assert(memcmp(row[1], "D", 2) == 0);
  assert(memcmp(row[2], "E", 2) == 0);

  arena_free(&arena);
}

void test_2_rows_with_empty_item_before_eol_last_row(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content =
    "A,B,C\r\n"
    "D,E,\r\n";

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 3);
  assert(csv.row_count == 2);
  const char** row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], "A", 2) == 0);
  assert(memcmp(row[1], "B", 2) == 0);
  assert(memcmp(row[2], "C", 2) == 0);
  row = (const char** const)csv_row(&csv, 1);
  assert(memcmp(row[0], "D", 2) == 0);
  assert(memcmp(row[1], "E", 2) == 0);
  assert(row[2] == NULL);

  arena_free(&arena);
}

void test_reading_unicode(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content = utf8_encode(
    "Á,B\r\n"
    "Ç,D\r\n");

  csv_parse(&arena, &csv, csv_content);
  assert(csv.column_count == 2);
  assert(csv.row_count == 2);
  const char** row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], utf8_encode("Á"), 4) == 0);
  assert(memcmp(row[1], "B", 2) == 0);
  row = (const char** const)csv_row(&csv, 1);
  assert(memcmp(row[0], utf8_encode("Ç"), 4) == 0);
  assert(memcmp(row[1], "D", 2) == 0);

  arena_free(&arena);
}

void test_parse_with_custom_delimiter(void) {
  arena_t arena = {0};
  arena_create(&arena, 4096);

  struct csv_t csv = {0};

  const char *csv_content = utf8_encode(
    "Á;B\r\n"
    "Ç;D\r\n");

  csv_parse_custom_delimiter(&arena, &csv, csv_content, ';');
  assert(csv.column_count == 2);
  assert(csv.row_count == 2);
  const char** row = (const char** const)csv_row(&csv, 0);
  assert(memcmp(row[0], utf8_encode("Á"), 4) == 0);
  assert(memcmp(row[1], "B", 2) == 0);
  row = (const char** const)csv_row(&csv, 1);
  assert(memcmp(row[0], utf8_encode("Ç"), 4) == 0);
  assert(memcmp(row[1], "D", 2) == 0);

  arena_free(&arena);
}

int main(void) {
  printf("test csv\n");
  test_parse_empty_value();
  test_parse_value();
  test_parse_quoted_value();
  test_parse_separator();
  test_fail_to_parse_separator();
  test_parse_crlf();
  test_fail_to_parse_crlf();
  test_parse_cr();
  test_fail_to_parse_cr();

  test_empty_string();
  test_single_value_without_header_without_eol();
  test_single_quoted_value();
  test_single_quoted_value_with_new_line();
  test_single_quoted_value_with_comma();
  test_single_line_with_2_values_sep_by_comma_without_eol();
  test_single_value_without_header_with_crlf_eol();
  test_single_value_with_crlf_eol();
  test_single_line_with_2_values_sep_by_comma_with_lf_eol();
  test_single_line_with_2_values_sep_by_comma_with_quoted();
  test_single_line_with_2_values_sep_by_comma_with_scaped_quotes();
  test_single_line_with_3_values_with_escaped_quotes_in_the_middle();
  test_2_rows();
  test_single_row_with_first_item_empty();
  test_single_row_with_empty_item_before_eol();
  test_single_row_with_empty_item_before_eol_without_proper_eol();
  test_2_rows_with_empty_item_before_eol_first_row();
  test_2_rows_with_empty_item_before_eol_last_row();
  test_reading_unicode();
  test_parse_with_custom_delimiter();
  printf("done\n");
  return 0;
}
