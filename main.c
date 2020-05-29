#include "rge_campaign.h"

// hardcoded DE2 dependency list
static int32_t de2_depencencies[RGE_DE2_DEPENDENCY_NUM] = { 2, 3, 4, 5, 6, 7 };
static int32_t de2_dependency_num = RGE_DE2_DEPENDENCY_NUM;
static uint16_t de_string_id = RGE_STRING_ID;

// exits if it's not a DE string
static size_t fread_de_string_id(FILE *f_i)
{
	size_t read = fread(&de_string_id, sizeof(de_string_id), 1, f_i);

	if (de_string_id != RGE_STRING_ID)
	{
		printf("error: not a proper DE1 string, file broken\n");
		exit(1);
	}

	return read;
}

static size_t fwrite_de_string_id(FILE *f_o)
{
	return fwrite(&de_string_id, sizeof(de_string_id), 1, f_o);
}

void RGE_Campaign_read(char *in_filename, char *out_folder)
{
	RGE_Campaign *campaign = calloc_d(1, sizeof(*campaign));
	RGE_Campaign_Header *campaign_header = &campaign->campaign_header;
	RGE_Scenario_Offset *scenario_offset;
	FILE *f_i, *f_o;
	RGE_Scenario *scenario = NULL;
	file_time_info ti;
	char out_path[UINT16_MAX];
	char scenario_tmp_name[UINT16_MAX];

	f_i = fopen_d(in_filename, "rb");

	fread(&campaign_header->version, sizeof(campaign_header->version), 1, f_i);

	if (campaign_header->version == RGE_CAMPAIGN_VERSION)
	{
		campaign_header->name_len = RGE_MAX_CHAR - 1;
		campaign_header->name = calloc_d(campaign_header->name_len + 1, 1);
		fread(campaign_header->name, campaign_header->name_len + 1, 1, f_i); // null terminated

		uint8_t byte;
		fread(&byte, sizeof(byte), 1, f_i); // read 1 struct padding byte

		fread(&campaign_header->scenario_num, sizeof(campaign_header->scenario_num), 1, f_i);
	}
	else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE1)
	{
		fread(&campaign_header->scenario_num, sizeof(campaign_header->scenario_num), 1, f_i);

		fread_de_string_id(f_i);

		fread(&campaign_header->name_len, sizeof(campaign_header->name_len), 1, f_i);
		campaign_header->name = calloc_d(campaign_header->name_len + 1, 1);
		fread(campaign_header->name, campaign_header->name_len, 1, f_i); // not null terminated
	}
	else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE2)
	{
		fread(&de2_dependency_num, sizeof(de2_dependency_num), 1, f_i);

		// dummy read into nowhere
		int32_t *dependencies = calloc_d(de2_dependency_num, sizeof(*dependencies));
		fread(dependencies, sizeof(*dependencies), de2_dependency_num, f_i);
		free(dependencies);

		campaign_header->name_len = RGE_DE2_MAX_CHAR - 1;
		campaign_header->name = calloc_d(campaign_header->name_len + 1, 1);
		fread(campaign_header->name, campaign_header->name_len + 1, 1, f_i); // null terminated

		/*
		// de2 writes uninitialised memory here - fix that by setting the remaining bytes to zero ... not needed since we don't actually use any data past the null terminator
		int32_t campaign_actual_len = strlen(campaign_header->name);
		memset(campaign_header->name + campaign_actual_len, '\0', campaign_header->name_len - campaign_actual_len);
		*/

		fread(&campaign_header->scenario_num, sizeof(campaign_header->scenario_num), 1, f_i);
	}
	else
	{
		printf("error: not an RGE_Campaign file, version invalid\n");
		exit(1);
	}

	printf("version: %.*s, name: %s, scenarion_num: %d\n",
		(int32_t)sizeof(int32_t), (char *)&campaign_header->version, campaign_header->name, campaign_header->scenario_num);

	if (out_folder)
	{
		char *new_out_folder = calloc_d(UINT16_MAX, 1);
		strcpy(new_out_folder, out_folder);
		out_folder = new_out_folder;

		get_file_time_info(&ti, in_filename);

		strncat(out_folder, "/", PATH_MAX_WIN);
	}

	campaign->scenario_offsets = calloc_d(campaign_header->scenario_num, sizeof(*campaign->scenario_offsets));

	for (int32_t i = 0; i < campaign_header->scenario_num; i++)
	{
		scenario_offset = &campaign->scenario_offsets[i];

		if (campaign_header->version == RGE_CAMPAIGN_VERSION)
		{
			fread(&scenario_offset->size, sizeof(scenario_offset->size), 1, f_i);
			fread(&scenario_offset->offset, sizeof(scenario_offset->offset), 1, f_i);

			scenario_offset->name_len = RGE_MAX_CHAR - 1;
			scenario_offset->name = calloc_d(scenario_offset->name_len + 1, 1);
			fread(scenario_offset->name, scenario_offset->name_len + 1, 1, f_i); // null terminated

			scenario_offset->file_name_len = RGE_MAX_CHAR - 1;
			scenario_offset->file_name = calloc_d(scenario_offset->file_name_len + 1, 1);
			fread(scenario_offset->file_name, scenario_offset->file_name_len + 1, 1, f_i); // null terminated

			uint8_t byte;
			fread(&byte, sizeof(byte), 1, f_i); // read 1 struct padding byte
			fread(&byte, sizeof(byte), 1, f_i); // read 1 struct padding byte
		}
		else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE1)
		{
			int64_t size, offset;

			fread(&size, sizeof(size), 1, f_i);
			fread(&offset, sizeof(offset), 1, f_i);

			if (size >= UINT32_MAX || offset >= UINT32_MAX)
			{
				printf("error: DE1 scenarios over 4GB not supported (wtf are you doing anyway?)\n");
				exit(1);
			}

			scenario_offset->size = (int32_t)size;
			scenario_offset->offset = (int32_t)offset;

			fread_de_string_id(f_i);

			fread(&scenario_offset->name_len, sizeof(scenario_offset->name_len), 1, f_i);
			scenario_offset->name = calloc_d(scenario_offset->name_len + 1, 1);
			fread(scenario_offset->name, scenario_offset->name_len, 1, f_i); // not null terminated

			fread_de_string_id(f_i);

			fread(&scenario_offset->file_name_len, sizeof(scenario_offset->file_name_len), 1, f_i);
			scenario_offset->file_name = calloc_d(scenario_offset->file_name_len + 1, 1);
			fread(scenario_offset->file_name, scenario_offset->file_name_len, 1, f_i); // not null terminated
		}
		else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE2)
		{
			fread(&scenario_offset->size, sizeof(scenario_offset->size), 1, f_i);
			fread(&scenario_offset->offset, sizeof(scenario_offset->offset), 1, f_i);

			fread_de_string_id(f_i);

			fread(&scenario_offset->name_len, sizeof(scenario_offset->name_len), 1, f_i);
			scenario_offset->name = calloc_d(scenario_offset->name_len + 1, 1);
			fread(scenario_offset->name, scenario_offset->name_len, 1, f_i); // not null terminated

			fread_de_string_id(f_i);

			fread(&scenario_offset->file_name_len, sizeof(scenario_offset->file_name_len), 1, f_i);
			scenario_offset->file_name = calloc_d(scenario_offset->file_name_len + 1, 1);
			fread(scenario_offset->file_name, scenario_offset->file_name_len, 1, f_i); // not null terminated
		}

		memset(scenario_tmp_name, '\0', sizeof(scenario_tmp_name));
		strncpy(scenario_tmp_name, scenario_offset->name, scenario_offset->name_len);
		strcat(scenario_tmp_name, ",");

		printf("scenario: %2d, size: %8d, offset: %8X, name: %-20s file_name: %s\n",
			i + 1, scenario_offset->size, scenario_offset->offset, scenario_tmp_name, scenario_offset->file_name);
	}

	for (int32_t i = 0; i < campaign_header->scenario_num; i++)
	{
		scenario_offset = &campaign->scenario_offsets[i];

		if (out_folder)
		{
			strncpy_d(out_path, out_folder, sizeof(out_path));
			strcat(out_path, scenario_offset->file_name);

			f_o = fopen_r(out_path);
			scenario = malloc_d(scenario_offset->size);

			fseek(f_i, scenario_offset->offset, SEEK_SET);
			fread(scenario, scenario_offset->size, 1, f_i);
			fwrite(scenario, scenario_offset->size, 1, f_o);

			fclose(f_o);
			free(scenario);

			set_file_time_info(&ti, out_path);
		}

		free(scenario_offset->name);
		free(scenario_offset->file_name);
	}

	free(campaign->scenario_offsets);
	free(campaign_header->name);
	free(campaign);
	fclose(f_i);

	free(out_folder);
}

void RGE_Campaign_write(char *campaign_filename, char *campaign_name, int32_t scenario_num, char **scenarios)
{
	RGE_Campaign *campaign = calloc(1, sizeof(*campaign));
	RGE_Campaign_Header *campaign_header = &campaign->campaign_header;
	RGE_Scenario_Offset *scenario_offset;
	FILE *f_i, *f_o;
	RGE_Scenario *scenario = NULL;
	char *scenario_file_name, *scenario_file_ext;
	char scenario_tmp_name[UINT16_MAX];
	uint64_t offset, size;

	char *ext = strrchr(campaign_filename, '.');

	if (!ext)
	{
		printf("warning: no extension found, assuming .cpn/.cpx format\n\n");

		campaign_header->version = RGE_CAMPAIGN_VERSION;
	}
	else
	{
		ext++;

		if (!stricmp(ext, "cpn") || !stricmp(ext, "cpx"))
		{
			campaign_header->version = RGE_CAMPAIGN_VERSION;
		}
		else if (!stricmp(ext, "aoecpn"))
		{
			campaign_header->version = RGE_CAMPAIGN_VERSION_DE1;
		}
		else if (!stricmp(ext, "aoe2campaign"))
		{
			campaign_header->version = RGE_CAMPAIGN_VERSION_DE2;
		}
		else
		{
			printf("warning: unknown extension, assuming .cpn/.cpx format\n\n");

			campaign_header->version = RGE_CAMPAIGN_VERSION;
		}
	}

	campaign_header->name_len = strlen(campaign_name);

	if (campaign_header->version == RGE_CAMPAIGN_VERSION)
	{
		if (campaign_header->name_len > RGE_MAX_CHAR - 1)
		{
			printf("error: input campaign name too long for .cpn/.cpx (max of %d characters allowed)\n", RGE_MAX_CHAR - 1);
			exit(1);
		}

		campaign_header->name_len = RGE_MAX_CHAR - 1;
	}
	else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE2)
	{
		if (campaign_header->name_len > RGE_DE2_MAX_CHAR - 1)
		{
			printf("error: input campaign name too long for .aoe2campaign (max of %d characters allowed)\n", RGE_DE2_MAX_CHAR - 1);
			exit(1);
		}

		campaign_header->name_len = RGE_DE2_MAX_CHAR - 1;
	}

	campaign_header->name = calloc_d(campaign_header->name_len + 1, 1);
	strcpy(campaign_header->name, campaign_name);

	campaign_header->scenario_num = scenario_num;

	printf("version: %.*s, name: %s, scenarion_num: %d\n",
		(int32_t)sizeof(int32_t), (char *)&campaign_header->version, campaign_header->name, campaign_header->scenario_num);

	campaign->scenario_offsets = calloc_d(scenario_num, sizeof(*campaign->scenario_offsets));

	for (int32_t i = 0; i < scenario_num; i++)
	{
		scenario_offset = &campaign->scenario_offsets[i];

		memset(scenario_tmp_name, '\0', sizeof(scenario_tmp_name));
		strcpy(scenario_tmp_name, scenarios[i]);

		scenario_file_name = strrchr(unixify_path(scenario_tmp_name), '/');

		if (scenario_file_name)
		{
			scenario_file_name++;
		}
		else
		{
			scenario_file_name = scenario_tmp_name;
		}

		scenario_file_ext = strrchr(scenario_file_name, '.');

		if (scenario_file_ext)
		{
			scenario_file_ext++;

			if (!stricmp(scenario_file_ext, "aoescn"))
			{
				if (campaign_header->version == RGE_CAMPAIGN_VERSION)
				{
					printf("error: .aoescn not supported in .cpn/.cpx\n");
					exit(1);
				}
			}
			else if (!stricmp(scenario_file_ext, "aoe2scenario"))
			{
				if (campaign_header->version == RGE_CAMPAIGN_VERSION || campaign_header->version == RGE_CAMPAIGN_VERSION_DE1)
				{
					printf("error: .aoe2scenario not supported in .aoecpn/.cpn/.cpx\n");
					exit(1);
				}
			}
			else if (stricmp(scenario_file_ext, "scn") && stricmp(scenario_file_ext, "scx"))
			{
				goto ext_error;
			}
		}
		else
		{
		ext_error:;
			printf("error: scenario file needs to have one of the following extensions: .scn/.scx/.aoescn/.aoe2scenario\n");
			exit(1);
		}

		scenario_offset->file_name_len = strlen(scenario_file_name);
		scenario_offset->name_len = scenario_offset->file_name_len - strlen(scenario_file_ext) - 1;

		if (campaign_header->version == RGE_CAMPAIGN_VERSION)
		{
			if (scenario_offset->file_name_len > RGE_MAX_CHAR - 1)
			{
				printf("error: scenario name too long for .cpn/.cpx (max of %d characters allowed)\n", RGE_MAX_CHAR - 1);
				exit(1);
			}

			scenario_offset->file_name_len = RGE_MAX_CHAR - 1;
			scenario_offset->name_len = RGE_MAX_CHAR - 1;
		}
		else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE2)
		{
			scenario_offset->name_len = scenario_offset->file_name_len;
		}

		scenario_offset->name = calloc_d(scenario_offset->name_len + 1, 1);

		scenario_offset->file_name = calloc_d(scenario_offset->file_name_len + 1, 1);
		strcpy(scenario_offset->file_name, scenario_file_name);

		if (campaign_header->version != RGE_CAMPAIGN_VERSION_DE2)
		{
			scenario_file_ext--;
			*scenario_file_ext = '\0';
		}

		strcpy(scenario_offset->name, scenario_file_name);
	}

	scenario_offset = &campaign->scenario_offsets[0];

	if (campaign_header->version == RGE_CAMPAIGN_VERSION)
	{
		scenario_offset->offset = sizeof(campaign_header->version) + RGE_MAX_CHAR + 1 + sizeof(campaign_header->scenario_num) +
			((sizeof(scenario_offset->size) + sizeof(scenario_offset->offset) +
				RGE_MAX_CHAR + RGE_MAX_CHAR +
				1 + 1) *
				campaign_header->scenario_num);
	}
	else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE1)
	{
		scenario_offset->offset = sizeof(campaign_header->version) + sizeof(de_string_id) + sizeof(campaign_header->name_len) + campaign_header->name_len + sizeof(campaign_header->scenario_num);

		RGE_Scenario_Offset *tmp_scenario_offset;

		for (int32_t i = 0; i < campaign_header->scenario_num; i++)
		{
			tmp_scenario_offset = &campaign->scenario_offsets[i];

			scenario_offset->offset += ((sizeof(size) + sizeof(offset) +
				sizeof(de_string_id) + sizeof(tmp_scenario_offset->name_len) + tmp_scenario_offset->name_len +
				sizeof(de_string_id) + sizeof(tmp_scenario_offset->file_name_len) + tmp_scenario_offset->file_name_len));
		}
	}
	else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE2)
	{
		scenario_offset->offset = sizeof(campaign_header->version) + sizeof(de2_dependency_num) + sizeof(de2_depencencies) + RGE_DE2_MAX_CHAR + sizeof(campaign_header->scenario_num);

		RGE_Scenario_Offset *tmp_scenario_offset;

		for (int32_t i = 0; i < campaign_header->scenario_num; i++)
		{
			tmp_scenario_offset = &campaign->scenario_offsets[i];

			scenario_offset->offset += ((sizeof(tmp_scenario_offset->size) + sizeof(tmp_scenario_offset->offset) +
				sizeof(de_string_id) + sizeof(tmp_scenario_offset->name_len) + tmp_scenario_offset->name_len +
				sizeof(de_string_id) + sizeof(tmp_scenario_offset->file_name_len) + tmp_scenario_offset->file_name_len));
		}
	}

	f_o = fopen_d(campaign_filename, "wb");

	for (int32_t i = 0; i < scenario_num; i++)
	{
		scenario_offset = &campaign->scenario_offsets[i];

		scenario_offset->size = (int32_t)fsize(scenarios[i]);

		if (i)
		{
			scenario_offset->offset = campaign->scenario_offsets[i - 1].offset + campaign->scenario_offsets[i - 1].size;
		}

		memset(scenario_tmp_name, '\0', sizeof(scenario_tmp_name));
		strcpy(scenario_tmp_name, scenario_offset->name);
		strcat(scenario_tmp_name, ",");

		printf("scenario: %2d, size: %8d, offset: %8X, name: %-20s file_name: %s\n",
			i + 1, scenario_offset->size, scenario_offset->offset, scenario_tmp_name, scenario_offset->file_name);

		f_i = fopen_d(scenarios[i], "rb");
		scenario = malloc_d(scenario_offset->size);

		fread(scenario, scenario_offset->size, 1, f_i);
		fseek(f_o, scenario_offset->offset, SEEK_SET);
		fwrite(scenario, scenario_offset->size, 1, f_o);

		free(scenario);
		fclose(f_i);
	}

	fseek(f_o, 0, SEEK_SET);

	uint8_t byte = 0;

	fwrite(&campaign_header->version, sizeof(campaign_header->version), 1, f_o);

	if (campaign_header->version == RGE_CAMPAIGN_VERSION)
	{
		fwrite(campaign_header->name, campaign_header->name_len + 1, 1, f_o); // null terminated
		fwrite(&byte, sizeof(byte), 1, f_o); // write 1 struct padding byte

		fwrite(&campaign_header->scenario_num, sizeof(campaign_header->scenario_num), 1, f_o);
	}
	else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE1)
	{
		fwrite(&campaign_header->scenario_num, sizeof(campaign_header->scenario_num), 1, f_o);

		fwrite_de_string_id(f_o);
		fwrite(&campaign_header->name_len, sizeof(campaign_header->name_len), 1, f_o);
		fwrite(campaign_header->name, campaign_header->name_len, 1, f_o); // not null terminated
	}
	else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE2)
	{
		// hardcoded default dependencies
		fwrite(&de2_dependency_num, sizeof(de2_dependency_num), 1, f_o);
		fwrite(de2_depencencies, sizeof(de2_depencencies), 1, f_o);

		fwrite(campaign_header->name, campaign_header->name_len + 1, 1, f_o); // null terminated

		fwrite(&campaign_header->scenario_num, sizeof(campaign_header->scenario_num), 1, f_o);
	}

	for (int32_t i = 0; i < scenario_num; i++)
	{
		scenario_offset = &campaign->scenario_offsets[i];

		if (campaign_header->version == RGE_CAMPAIGN_VERSION)
		{
			fwrite(&scenario_offset->size, sizeof(scenario_offset->size), 1, f_o);
			fwrite(&scenario_offset->offset, sizeof(scenario_offset->size), 1, f_o);

			fwrite(scenario_offset->name, scenario_offset->name_len + 1, 1, f_o); // null terminated
			fwrite(scenario_offset->file_name, scenario_offset->file_name_len + 1, 1, f_o); // null terminated

			fwrite(&byte, sizeof(byte), 1, f_o); // write 1 struct padding byte
			fwrite(&byte, sizeof(byte), 1, f_o); // write 1 struct padding byte
		}
		else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE1)
		{
			size = scenario_offset->size;
			offset = scenario_offset->offset;

			fwrite(&size, sizeof(size), 1, f_o);
			fwrite(&offset, sizeof(size), 1, f_o);

			fwrite_de_string_id(f_o);
			fwrite(&scenario_offset->name_len, sizeof(scenario_offset->name_len), 1, f_o);
			fwrite(scenario_offset->name, scenario_offset->name_len, 1, f_o); // not null terminated

			fwrite_de_string_id(f_o);
			fwrite(&scenario_offset->file_name_len, sizeof(scenario_offset->file_name_len), 1, f_o);
			fwrite(scenario_offset->file_name, scenario_offset->file_name_len, 1, f_o); // not null terminated
		}
		else if (campaign_header->version == RGE_CAMPAIGN_VERSION_DE2)
		{
			fwrite(&scenario_offset->size, sizeof(scenario_offset->size), 1, f_o);
			fwrite(&scenario_offset->offset, sizeof(scenario_offset->size), 1, f_o);

			fwrite_de_string_id(f_o);
			fwrite(&scenario_offset->name_len, sizeof(scenario_offset->name_len), 1, f_o);
			fwrite(scenario_offset->name, scenario_offset->name_len, 1, f_o); // not null terminated

			fwrite_de_string_id(f_o);
			fwrite(&scenario_offset->file_name_len, sizeof(scenario_offset->file_name_len), 1, f_o);
			fwrite(scenario_offset->file_name, scenario_offset->file_name_len, 1, f_o); // not null terminated
		}

		free(scenario_offset->name);
		free(scenario_offset->file_name);
	}

	free(campaign_header->name);
	free(campaign->scenario_offsets);
	free(campaign);
	fclose(f_o);
}

void printf_help_exit(int exit_code)
{
	printf("rge_campaign v%u.%u by withmorten\n\n", VERSION_MAJOR, VERSION_MINOR);
	printf("rge_campaign supports the following arguments:\n\n");
	printf("rge_campaign l <campaign file> to list a campaign file's contents (any format)\n");
	printf("rge_campaign x <campaign file> <directory> to extract a campaign file\n");
	printf("rge_campaign c <campaign file> <campaign name> <scenario 1> [scenario n ...] to create a campaign file\n\n");
	printf("the extension determines which campaign format will be created:\n\n");
	printf(".cpn/.cpx creates an AoE1 to AoC campaign (default)\n");
	printf(".aoecpn creates a DE1 campaign\n");
	printf(".aoe2campaign creates a DE2 campaign\n\n");
	printf(".cpn/.cpx are identical formats, only difference is the extension\n\n");
	printf("rge_campaigns's source and readme are available at https://github.com/withmorten/rge_campaign\n");
	exit(exit_code);
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf_help_exit(0);
	}

	if (argv[1][1])
	{
		printf_help_exit(1);
	}

	printf("\n");

	switch (*argv[1])
	{
	case 'l':
		if (argc < 3)
		{
			printf_help_exit(1);
		}

		RGE_Campaign_read(argv[2], NULL);

		printf("\nlisted %s successfully\n", argv[2]);

		break;
	case 'x':
		if (argc < 4)
		{
			printf_help_exit(1);
		}

		RGE_Campaign_read(argv[2], argv[3]);

		printf("\nextracted %s successfully\n", argv[2]);

		break;
	case 'c':
		if (argc < 5)
		{
			printf_help_exit(1);
		}

		RGE_Campaign_write(argv[2], argv[3], argc - 4, argv + 4);

		printf("\nwrote %s successfully\n", argv[2]);

		break;
	default:
		printf_help_exit(1);

		break;
	}

	return 0;
}
