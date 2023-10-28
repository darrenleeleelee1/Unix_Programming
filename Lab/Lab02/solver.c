#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
char *target;
char line[100];
void func(DIR *dp, char* path)
{
	struct dirent	*dirp;
	while ((dirp = readdir(dp)) != NULL){
		if (dirp->d_type == DT_DIR) {
			if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0) {
                continue;
            }
            char newpath[1024];
            snprintf(newpath, sizeof(newpath), "%s/%s", path, dirp->d_name);
            func(opendir(newpath), newpath);
		}
		else{
			FILE *fp;
			char newpath[1024];
            snprintf(newpath, sizeof(newpath), "%s/%s", path, dirp->d_name);
			fp = fopen(newpath, "r");
			if (fp == NULL) {
				fprintf(stderr, "Failed to open the file.\n");
			}
			while (fgets(line, sizeof(line), fp) != NULL) {
				if (strstr(line, target) != NULL || strcmp(line, target) == 0) {
					printf("%s\n", newpath);
				}
			}
			// Perform operations on the file here
			fclose(fp);
		}
	}
}
int main(int argc, char *argv[])
{
	DIR				*dp;
	target = argv[2];

	if ((dp = opendir(argv[1])) == NULL){
		fprintf(stderr, "can't open %s", argv[1]);
	}
	func(dp, argv[1]);

	closedir(dp);
	exit(0);
}
