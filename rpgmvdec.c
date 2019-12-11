#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <linux/limits.h>

int decodefile(char* filename, uint8_t key[16])
{
	char outfilename[PATH_MAX];
	int filenamelen;
	char buf[8192];
	int bytes;
	int encfd;
	int decfd;

	filenamelen = strlen(filename);
	if ( filenamelen > PATH_MAX )
	{
		fprintf(stderr, "Error: filename '%s' too long\n", filename);
		return -1;
	}

	strncpy(outfilename, filename, PATH_MAX);
	outfilename[filenamelen] = '\0';

	if ( filenamelen < 6 || 
	     (memcmp(&outfilename[filenamelen-6], "rpgmvp", 6) != 0 ) )
	{
		fprintf(stderr, "Caution: filename extension unknown, appending instead of replacing\n");
		strncat(outfilename, ".png", 4);
	}
	else
	{
		memcpy(&outfilename[filenamelen-6], "png\0", 4);
	}

	encfd = open(filename, O_RDONLY);
	if ( encfd < 0 )
	{
		perror(filename);
		return -1;
	}

	bytes = read(encfd, buf, 16);
	if ( bytes < 16 )
	{
		perror(filename);
		close(encfd);
		return -1;
	}

	if ( memcmp(buf, "RPGMV", 5) != 0 )
	{
		fprintf(stderr, "%s does not appear to be a rpgmv file, skipping\n", filename);
		close(encfd);
		return -1;
	}

	bytes = read(encfd, buf, 16);
	if ( bytes < 16 )
	{
		perror(filename);
		close(encfd);
	}

	uint8_t decrypted[16];
	for ( int byte = 0; byte < 16; byte++ )
	{
		decrypted[byte] = buf[byte] ^ key[byte];
	}

	if ( memcmp(decrypted, "\211PNG", 4) != 0 )
	{
		fprintf(stderr, "Decrypt failed for '%s', PNG header not found\n", filename);
		close(encfd);
		return -1;
	}

	decfd = open(outfilename, O_WRONLY | O_CREAT, 0644);
	if ( decfd < 0 )
	{
		perror(outfilename);
		close(encfd);
		return -1;
	}

	bytes = write(decfd, decrypted, 16);

	for ( bytes = read(encfd, buf, 8192);
		bytes > 0;
		bytes = read(encfd, buf, 8192) )
	{
		write(decfd, buf, bytes);
	}
	close(encfd);
	close(decfd);

	printf("%s -> %s\n", filename, outfilename);

	return 0;
}

int printhelp()
{
	printf("Decrypts rpgmvp files, writes the output to a file with the\n");
	printf(".png extension instead of the .rpgmvp extension\n");
	printf("Usage: rpgmvpdec -k <keystring> <files>\n");
	printf("To get the key: json_pp < System.json | grep encryptionKey\n");
	return 0;
}

int main(int argc, char** argv)
{
	uint8_t key[16];
	int opt;
	char nybblestr[3];
	int filenum;
	int keyset = 0;

	memset(key, 0, sizeof(key));
	memset(nybblestr, 0, 3);

	while ((opt = getopt(argc, argv, "k:h")) != -1 )
	{
		switch(opt)
		{
		case 'k':
			keyset = 1;
			for ( int lcv = 0; lcv < 16; lcv++ )
			{
				memcpy(nybblestr, &optarg[lcv*2], 2);
				unsigned int scankey;
				sscanf(nybblestr, "%x", &scankey);
				key[lcv] = scankey;
			}
		break;

		case 'h':
		default:
			printhelp();
			return 0;
		break;
		}
	}

	if ( keyset == 0 )
	{
		printhelp();
		return 0;
	}

	for ( filenum = optind; filenum < argc; filenum++ )
	{
		decodefile(argv[filenum], key);
	}

	return 0;
}
