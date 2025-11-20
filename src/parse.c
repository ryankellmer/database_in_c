#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "../include/common.h"
#include "../include/parse.h"

int list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {
  int employeeCount = dbhdr->count;
  for (int i = 0; i < employeeCount; i++) {
    printf("Employee %d\n", i);
    printf("\tName: %s\n", employees[i].name);
    printf("\tAddress: %s\n", employees[i].address);
    printf("\tHours: %d\n", employees[i].hours);
  }
  return STATUS_SUCCESS;
}

int add_employee(struct dbheader_t *dbhdr, struct employee_t **employees,
                 char *addstring) {
  if (NULL == dbhdr)
    return STATUS_ERROR;
  if (NULL == employees)
    return STATUS_ERROR;
  if (NULL == *employees)
    return STATUS_ERROR;
  if (NULL == addstring)
    return STATUS_ERROR;

  char *name = strtok(addstring, ",");
  if (NULL == name)
    return STATUS_ERROR;

  char *address = strtok(NULL, ",");
  if (NULL == address)
    return STATUS_ERROR;

  char *hours = strtok(NULL, ",");
  if (NULL == hours)
    return STATUS_ERROR;

  struct employee_t *e = *employees;

  e = realloc(e, sizeof(struct employee_t) * dbhdr->count + 1);
  if (e == NULL) {
    return STATUS_ERROR;
  }

  dbhdr->count++;

  printf("%s %s %s\n", name, address, hours);

  strncpy(e[dbhdr->count - 1].name, name, sizeof(e[dbhdr->count - 1].name) - 1);
  strncpy(e[dbhdr->count - 1].address, address,
          sizeof(e[dbhdr->count - 1].address) - 1);
  e[dbhdr->count - 1].hours = atoi(hours);

  *employees = e;

  return STATUS_SUCCESS;
}

int read_employees(int fd, struct dbheader_t *dbhdr,
                   struct employee_t **employeesOut) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  int count = dbhdr->count;

  struct employee_t *employees = calloc(count, sizeof(struct employee_t));

  if (employees == NULL) {
    printf("Malloc failed to create a database header\n");
    return STATUS_ERROR;
  }

  int rd = read(fd, employees, count * sizeof(struct employee_t));
  if (rd != (count * sizeof(struct employee_t))) {
    perror("read");
    free(employees);
    return STATUS_ERROR;
  }

  for (int i = 0; i < count; i++) {
    employees[i].hours = ntohl(employees[i].hours);
  }

  *employeesOut = employees;

  return STATUS_SUCCESS;
}

int output_file(int fd, struct dbheader_t *dbhdr,
                struct employee_t *employees) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  int realCount = dbhdr->count;

  dbhdr->magic = htonl(dbhdr->magic);
  dbhdr->filesize = htonl(sizeof(struct dbheader_t) +
                          (sizeof(struct employee_t) * realCount));
  dbhdr->count = htons(dbhdr->count);
  dbhdr->version = htons(dbhdr->version);

  lseek(fd, 0, SEEK_SET);

  write(fd, dbhdr, sizeof(struct dbheader_t));

  for (int i = 0; i < realCount; i++) {
    employees[i].hours = ntohl(employees[i].hours);
    write(fd, &employees[i], sizeof(struct employee_t));
  }

  return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0) {
    printf("Got a bad FD from the user\n");
    return STATUS_ERROR;
  }

  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    printf("Malloc failed to create a database header\n");
    return STATUS_ERROR;
  }

  int rd = read(fd, header, sizeof(struct dbheader_t));
  if (rd != sizeof(struct dbheader_t)) {
    perror("read");
    free(header);
    return STATUS_ERROR;
  }

  // NOTE: network byte order to host byte order.
  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->magic = ntohl(header->magic);
  header->filesize = ntohl(header->filesize);

  if (header->magic != HEADER_MAGIC) {
    printf("Improper header magic\n");
    free(header);
    return STATUS_ERROR;
  }

  if (header->version != 1) {
    printf("Improper header version\n");
    free(header);
    return STATUS_ERROR;
  }

  struct stat dbstat = {0};

  if (fstat(fd, &dbstat) < 0) {
    perror("fstat");
    free(header);
    return STATUS_ERROR;
  }

  if (header->filesize != dbstat.st_size) {
    printf("Corrupted database\n");
    free(header);
    return STATUS_ERROR;
  }

  *headerOut = header;

  return STATUS_SUCCESS;
}

int create_db_header(struct dbheader_t **headerOut) {
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == NULL) {
    printf("Malloc failed to create a database header\n");
    return STATUS_ERROR;
  }

  header->version = 0x1;
  header->count = 0;
  header->magic = HEADER_MAGIC;
  header->filesize = sizeof(struct dbheader_t);

  *headerOut = header;

  return STATUS_SUCCESS;
}
