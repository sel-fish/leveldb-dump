#include <stdio.h>
#include <unistd.h>

int main(int argc, char** argv)
{
  char* manifest_file = NULL;
  char* db_path = NULL;
  char* dump_file = NULL;
	int i = 0;

  while ((i = getopt(argc, argv, "p:f:d:")) != EOF)
  {
    switch (i)
    {
    case 'p':
      db_path = optarg;
      break;
    case 'f':
      manifest_file = optarg;
      break;
    case 'd':
      dump_file = optarg;
      break;
    default:
      // print_help(argv[0]);
      return 1;
    }
  }
}
