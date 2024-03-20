char *pk_dirname(char *path);

int main(UNUSED int argc, const char **argv)
{
	argv++;
	fputs(pk_dirname(*argv ? strdup(*argv) : NULL), stdout);
	return 0;
}