#include "fit.h"

int parse_file(const char * path, struct fit_file * f)
{
	FILE * fp;
	uint8_t header, t;
	
	fp = fopen(path, "rb");
	
	if (fp == NULL)
		return -1;
	
	fread(&(*f).header_size, sizeof((*f).header_size), 1, fp);
	fread(&(*f).protocol_ver, sizeof((*f).protocol_ver), 1, fp);
	fread(&(*f).profile_version, sizeof((*f).profile_version), 1, fp);
	fread(&(*f).records_length, sizeof((*f).records_length), 1, fp);
	fread(&(*f).ascii, sizeof((*f).ascii), 1, fp);
	
	if ((*f).header_size >= 14)
		fread(&(*f).crc, sizeof((*f).crc), 1, fp);
		
	//Allocate memory for 16 possible local definitions.
	(*f).def = (struct fit_def *)malloc(sizeof(struct fit_def) * 16);
	
	while (ftell(fp) - (*f).header_size < (*f).records_length) {
		fread(&header, sizeof(header), 1, fp);
		
		if (IS_COMPRESSED(header)) {
			//Account for compressed header 
		} else {
			t = LOCALTYPE(header);
			if (MSGTYPE(header) == FIT_DEFINITION) {
				(*f).def[t].num_data = 0;
				(*f).def[t].data = NULL;
				read_def(fp, &(*f).def[t]);
				printf("%i\n", t);
				print_def(&(*f).def[t]);
			} else {
				read_data(fp, &(*f).def[t]);
			}
		}

	}

	return 0;
}

int read_def(FILE * fp, struct fit_def * d)
{
	uint8_t i;

	fread(&((*d).reserved), sizeof((*d).reserved), 1, fp);
	fread(&((*d).arch), sizeof((*d).arch), 1, fp);
	fread(&((*d).gid), sizeof((*d).gid), 1, fp);
	fread(&((*d).num_fields), sizeof((*d).num_fields), 1, fp);
	
	(*d).field = (struct field_def *)malloc(sizeof(struct field_def) * (*d).num_fields);
	
	for (i = 0; i < (*d).num_fields; i++) {
		fread(&((*d).field[i].def_num), sizeof((*d).field[i].def_num), 1, fp);
		fread(&((*d).field[i].size), sizeof((*d).field[i].size), 1, fp);
		fread(&((*d).field[i].type), sizeof((*d).field[i].type), 1, fp);
	}
	
	return 0;
}

int read_data(FILE * fp, struct fit_def * def)
{
	uint8_t i;
	
	uint32_t t = ++(*def).num_data;
	(*def).data = (struct fit_data *)realloc((*def).data, sizeof(struct fit_data) * t);
	(*def).data[t-1].field = (void **)malloc(sizeof(void**) * (*def).num_fields);
	
	for (i = 0; i < (*def).num_fields; i++) {
		(*def).data[t-1].field[i] = malloc((*def).field[i].size);
		fread(&(*def).data[t-1].field[i], 1, (*def).field[i].size, fp);
	}
	
	return 0;
}

int print_def(struct fit_def * d)
{
	uint8_t i;

	printf("arch = %i\n", (*d).arch);
	printf("gid = %i\n", (*d).gid);
	printf("num_fields = %i\n", (*d).num_fields);
	
	for (i = 0; i < (*d).num_fields; i++)
	{
		printf("\tdef_num = %i\n", (*d).field[i].def_num);
		printf("\tsize = %i\n", (*d).field[i].size);
		printf("\ttype = %.2X\n", (*d).field[i].type);
	}
	
	return 0;
}

int print_fit_type(FILE * out, uint32_t type, void * data)
{
	switch (type)
	{
		case fit_enum:
			break;
		case fit_sint8:
			printf("%i", *(int8_t*)data);
			break;
		case fit_sint16:
			printf("%i", *(int16_t*)data);
			break;
		case fit_sint32:
			printf("%i", *(int32_t*)data);
			break;
		case fit_uint8:
			printf("%u", *(uint8_t*)data);
			break;
		case fit_uint16:
			printf("%u", *(uint16_t*)data);
			break;
		case fit_uint32:
			printf("%u", *(uint32_t*)data);
			break;
		case fit_string:
			printf("%s", *(char*)data);
			break;
		case fit_float32:
			printf("%f", *(float*)data);
			break;
		case fit_float64:
			printf("%lf", *(double*)data);
			break;
		case fit_byte:
			break;
		default:
			printf("Unknown type %i", type);
}

//Should be print_def
int print_fit(FILE * out, struct fit_file * f)
{
	uint32_t i, j, k;

	for (i = 0; i < 16; i++)
	{
		fprintf(out, "Data message %i\n", i);
		for (j = 0; j < (*f).num_fields; j++)
		{
			fprintf(out, "%i,", (*f).def[i].field[j].def_num);
		}
		for (j = 0; j < (*f).def[i].num_data; j++)
		{
			for (k = 0; k < (*f).def[i].num_fields; k++)
				print_fit_type(out, (*f).def[i].field[k].type, (*f).def[i]);
		}
	}
	return 0;
}
