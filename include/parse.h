#ifndef PARSE_H
#define PARSE_H

#define HEADER_MAGIC 0x4c4c4144
#define NAME_LEN 256
#define ADDRESS_LEN 256

struct dbheader_t {
  unsigned int magic;
  unsigned short version;
  unsigned short count;
  unsigned int filesize;
};

struct employee_t {
  char name[NAME_LEN];
  char address[ADDRESS_LEN];
  unsigned int hours;
};

int create_db_header(struct dbheader_t **headerOut);
int validate_db_header(int fd, struct dbheader_t **headerOut);
int read_employees(int fd, struct dbheader_t *dbhdr,
                   struct employee_t **employeesOut);
int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees,
                 char *addstring);
int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees);

#endif
