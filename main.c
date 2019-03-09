#include "rge_campaign.h"

void RGE_Campaign_read(char *in_filename, char *out_folder, enum read_mode read_mode)
{
	RGE_Campaign *campaign = calloc_d(1, sizeof(RGE_Campaign));
	RGE_Campaign_Header *campaign_header = &campaign->campaign_header;
	RGE_Scenario_Offset *scenario_offset;
	FILE *f_i, *f_o;
	RGE_Scenario *scenario = NULL;
	file_time_info ti;
	char out_path[PATH_MAX_WIN];
	int i;

	if (read_mode == READ_MODE_EXTRACT)
	{
		get_file_time_info(&ti, in_filename);

		ti.ctime = ti.mtime;
		ti.atime = ti.mtime;
	}

	f_i = fopen_d(in_filename, "rb");

	fread(campaign_header, sizeof(RGE_Campaign_Header), 1, f_i);

	if (campaign_header->version != RGE_CAMPAIGN_VERSION)
	{
		printf("error: not an RGE_Campaign file, version invalid\n");
		exit(1);
	}

	printf("version: %.*s, name: %s, scenarion_num: %d\n",
		(int)sizeof(int32_t), (char *)&campaign_header->version, campaign_header->name, campaign_header->scenario_num);

	campaign->scenario_offsets = calloc_d(campaign_header->scenario_num, sizeof(RGE_Scenario_Offset));
	fread(campaign->scenario_offsets, sizeof(RGE_Scenario_Offset), campaign_header->scenario_num, f_i);

	strncat(out_folder, "/", PATH_MAX_WIN);

	for (i = 0; i < campaign_header->scenario_num; i++)
	{
		scenario_offset = &campaign->scenario_offsets[i];

		printf("scenario: %2d, size: %8d, offset: %8X, name: %s, file_name: %s\n",
			i + 1, scenario_offset->size, scenario_offset->offset, scenario_offset->name, scenario_offset->file_name);

		if (read_mode == READ_MODE_EXTRACT)
		{
			strncpy(out_path, out_folder, PATH_MAX_WIN);
			strncat(out_path, scenario_offset->file_name, PATH_MAX_WIN - 1);

			f_o = fopen_r(out_path);
			scenario = malloc_d(scenario_offset->size);

			fseek(f_i, scenario_offset->offset, SEEK_SET);
			fread(scenario, scenario_offset->size, 1, f_i);
			fwrite(scenario, scenario_offset->size, 1, f_o);

			fclose(f_o);
			free(scenario);

			set_file_time_info(&ti, out_path);
		}
	}

	free(campaign->scenario_offsets);
	free(campaign);
	fclose(f_i);
}

void RGE_Campaign_write(char *campaign_filename, char *campaign_name, int scenario_num, char **scenarios)
{
	RGE_Campaign *campaign = calloc(1, sizeof(RGE_Campaign));
	RGE_Campaign_Header *campaign_header = &campaign->campaign_header;
	RGE_Scenario_Offset *scenario_offset;
	FILE *f_i, *f_o;
	RGE_Scenario *scenario = NULL;
	char *scenario_file_name, *scenario_file_ext;
	int i;

	campaign_header->version = RGE_CAMPAIGN_VERSION;
	strncpy(campaign_header->name, campaign_name, RGE_MAX_CHAR);
	campaign_header->scenario_num = scenario_num;

	campaign->scenario_offsets = calloc_d(scenario_num, sizeof(RGE_Scenario_Offset));

	f_o = fopen_d(campaign_filename, "wb");

	campaign->scenario_offsets[0].offset = sizeof(RGE_Campaign_Header) + (campaign_header->scenario_num * sizeof(RGE_Scenario_Offset));

	for (i = 0; i < scenario_num; i++)
	{
		scenario_offset = &campaign->scenario_offsets[i];

		scenario_offset->size = fsize(scenarios[i]);

		if (i)
		{
			scenario_offset->offset = campaign->scenario_offsets[i - 1].offset + campaign->scenario_offsets[i - 1].size;
		}

		scenario_file_name = strrchr(unixify_path(scenarios[i]), '/');

		if (scenario_file_name)
		{
			strncpy(scenario_offset->file_name, scenario_file_name + 1, RGE_MAX_CHAR);
		}
		else
		{
			strncpy(scenario_offset->file_name, scenarios[i], RGE_MAX_CHAR);
		}

		scenario_file_ext = strrchr(scenario_offset->file_name, '.');

		if (!scenario_file_ext || (stricmp(scenario_file_ext, ".scn") && stricmp(scenario_file_ext, ".scx")))
		{
			printf("error: file %s does not have scn/scx extension, exiting\n", scenarios[i]);
			exit(1);
		}

		strncpy(scenario_offset->name, scenario_offset->file_name, strlen(scenario_offset->file_name) - strlen(scenario_file_ext));

		printf("scenario: %2d, size: %8d, offset: %8X, name: %s, file_name: %s\n",
			i + 1, scenario_offset->size, scenario_offset->offset, scenario_offset->name, scenario_offset->file_name);

		f_i = fopen_d(scenarios[i], "rb");
		scenario = malloc_d(scenario_offset->size);

		fread(scenario, scenario_offset->size, 1, f_i);
		fseek(f_o, scenario_offset->offset, SEEK_SET);
		fwrite(scenario, scenario_offset->size, 1, f_o);

		free(scenario);
		fclose(f_i);
	}

	fseek(f_o, 0, SEEK_SET);

	fwrite(campaign_header, 1, sizeof(RGE_Campaign_Header), f_o);
	fwrite(campaign->scenario_offsets, campaign_header->scenario_num, sizeof(RGE_Scenario_Offset), f_o);

	fclose(f_o);

	free(campaign->scenario_offsets);
	free(campaign);
}

void printf_help_exit(int exit_code)
{
	printf("rge_campaign v%u.%u by withmorten\n\n", VERSION_MAJOR, VERSION_MINOR);
	printf("rge_campaign supports the following arguments:\n\n");
	printf("rge_campaign l <campaign file> to list a .cpn/cpx file's contents\n");
	printf("rge_campaign x <campaign file> <directory> to extract a .cpn/cpx file\n");
	printf("rge_campaign c <campaign file> <campaign name> <scenario 1> [scenario n ...] to create a .cpn/cpx file\n\n");
	printf("cpn/cpx are identical formats, only difference is the extension\n\n");
	printf("rge_campaigns's source and readme are available at https://github.com/withmorten/rge_campaign\n");
	exit(exit_code);
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf_help_exit(0);
	}

	if (strlen(argv[1]) != 1)
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

		RGE_Campaign_read(argv[2], NULL, READ_MODE_LIST);

		printf("\nlisted %s successfully\n", argv[2]);

		break;
	case 'x':
		if (argc < 4)
		{
			printf_help_exit(1);
		}

		RGE_Campaign_read(argv[2], argv[3], READ_MODE_EXTRACT);

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