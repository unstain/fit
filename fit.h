#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define FIT_BIG    1
#define FIT_LITTLE 0

#define FIT_DEFINITION 1
#define FIT_DATA       0

typedef enum {
	fit_enum    = 0x00,
	fit_sint8   = 0x01,
	fit_uint8   = 0x02,
	fit_sint16  = 0x83,
	fit_uint16  = 0x84,
	fit_sint32  = 0x85,
	fit_uint32  = 0x86,
	fit_string  = 0x07, /* Null terminated UTF-8 string.*/
	fit_float32 = 0x88,
	fit_float64 = 0x89,
	fit_uint8z  = 0x0A,
	fit_uint16z = 0x8B,
	fit_uint32z = 0x8C,
	fit_byte    = 0x0D, /* Array of bytes. */
} fit_type;

/* Returns 0 if the file is not a compressed timestamp, 1 if it is. */
#define IS_COMPRESSED(X) ((X & 0b10000000) >> 7)
/* The message type, either definition or data. */
#define MSGTYPE(X)      ((X & 0b01000000) >> 6)
/* The local type corresponding to the message. */
#define LOCALTYPE(X)     (X & 0x0F)
/* Compressed timestamp local message type. */
#define COMPTYPE(X)     ((X & 0b01100000) >> 5)
/* Compressed timestamp time offset, in seconds. */
#define TIMEOFFSET(X)    (X & 0x1F)

struct fit_file {
	uint8_t  header_size; /* The length of the file header in bytes. */
	uint8_t  protocol_ver;
	uint16_t profile_version;
	uint32_t records_length; /* Length of the data records section in bytes. */
	char     ascii[4]; /* Should contain the ascii string ".FIT" */
	uint16_t crc; /* CRC of the first 12 bytes. */
	struct fit_def * def; //Maximum of 16 local message types.
};

struct field_def {
	uint8_t  def_num;
	uint8_t  size;
	uint8_t  type;
};

struct fit_def {
	uint8_t  header;
	uint8_t  reserved;
	uint8_t  arch;
	uint16_t gid;
	uint8_t  num_fields;
	struct field_def * field;
	uint32_t num_data;
	struct fit_data * data;
};

struct fit_data {
	uint8_t  header;
	void     ** field;
};


int parse_file(const char * path, struct fit_file * f);
int parse_def(FILE * fd, struct fit_file * f);
int read_def(FILE * fd, struct fit_def * d);
int read_data(FILE * fd, struct fit_def * d);
int print_def(struct fit_def * d);
int print_fit(struct fit_file * f);
