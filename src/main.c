#include <getopt.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/common.h"
#include "../include/file.h"
#include "../include/parse.h"

void print_usage(char *argv[]) {
  printf("Usage: %s -n -f <database file>\n", argv[0]);
  printf("\t -n - create new database file\n");
  printf("\t -f - (required) path to database file\n");
  return;
}

int main(int argc, char *argv[]) {
  char *filepath = NULL;
  bool newfile = false;
  char c;

  int dbfd = -1;
  struct dbheader_t *dbhdr;

  while (-1 != (c = getopt(argc, argv, "nf:"))) {
    switch (c) {
    case 'n':
      newfile = true;
      break;
    case 'f':
      filepath = optarg;
      break;
    case '?':
      printf("Unknown option -%c\n", c);
      break;
    default:
      return -1;
    }
  }

  if (filepath == NULL) {
    printf("Filepath is a required field\n");
    print_usage(argv);
    return 0;
  }

  if (newfile) {
    dbfd = create_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to create database file\n");
      return -1;
    }

    if (create_db_header(dbfd, &dbhdr) == STATUS_ERROR) {
      printf("Failed to create database header\n");
      return -1;
    }
  } else {
    dbfd = open_db_file(filepath);
    if (dbfd == STATUS_ERROR) {
      printf("Unable to open database file\n");
      return -1;
    }

    int val = validate_db_header(dbfd, &dbhdr);

    if (val == STATUS_ERROR) {
      printf("Failed to validate database header\n");
      return -1;
    }
  }

  output_file(dbfd, dbhdr);

  return 0;
}
