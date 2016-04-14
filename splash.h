#ifndef SPLASH_H_
#define SPLASH_H_

#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <list>
#include <string>
#include <iostream>

using namespace std;

bool getimagefromdir(const char *dir, list<string> &image_list)
{
	DIR* dp;
	char cwddir[35];
	char image_path[50];
	struct  dirent  *entry;
	struct  stat    statbuf;

	getcwd(cwddir, sizeof(cwddir));

	if ((dp = opendir(dir)) == NULL) {
		fprintf(stderr, "Cannot open directory: %s\n", dir);
		return false;
	}

	chdir(dir);
	while ((entry = readdir(dp)) != NULL ) {

		lstat( entry->d_name, &statbuf );
		if (S_ISREG(statbuf.st_mode) && (entry->d_name)[0] != '.') {
			strcpy(image_path, dir);
			strcat(image_path, entry->d_name);
			image_list.push_back(image_path);
		}
	}
	image_list.sort();

	chdir(cwddir);
	closedir(dp);

	return true;
}
#endif
