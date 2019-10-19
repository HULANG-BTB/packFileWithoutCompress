#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <io.h>
#include <direct.h>

#define MAXFILENUM 1024

struct FILEINFO {
	char name[1024];
	char path[1024];
	unsigned long size;
	bool type;
}list[MAXFILENUM];

int fileNums;

int findLastChar(const char * str, char c) {
	int len = strlen(str);
	while (--len >= 0)
	{
		if (str[len] == c) return len;
	}
	return -1;
}

void getFileList(const char* fullPath, const char* parentPath = ".\\") {
	intptr_t hFile = 0;
	char type[20];
	char path[1024];
	char full[1024];
	struct _finddata_t fileInfo;
	int lastPos = findLastChar(fullPath, '\\');
	if (lastPos == strlen(fullPath) - 1) {
		strcpy(type, "*");
		strcpy(path, fullPath);
	}
	else {
		strcpy(type, fullPath + lastPos + 1);
		strncpy(path, fullPath, lastPos + 1);
		path[lastPos + 1] = 0;
	}
	strcpy(full, path);
	strcat(full, type);
	if ((hFile = _findfirst(full, &fileInfo)) == -1) {
		return;
	}
	do {
		if (strcmp(fileInfo.name, ".") == 0 || strcmp(fileInfo.name, "..") == 0) {
			continue;
		}
		fileNums++;
		list[fileNums].size = fileInfo.size;
		strcpy(list[fileNums].name, fileInfo.name);
		strcpy(list[fileNums].path, parentPath);
		if (fileInfo.attrib == _A_SUBDIR) {
			list[fileNums].type = 1;
			char newPath[1024];
			strcpy(newPath, path);
			strcat(newPath, list[fileNums].name);
			strcat(newPath, "\\");
			strcat(newPath, type);
			strcat(list[fileNums].path, list[fileNums].name);
			strcat(list[fileNums].path, "\\");
			getFileList(newPath, list[fileNums].path);
		}
		else {
			list[fileNums].type = 0;
		}
		
	} while (_findnext(hFile, &fileInfo) == 0);
	_findclose(hFile);
}

void createDir(const char* path) {
	for (int i = 0; i <= strlen(path); i++)
	{
		if (path[i] == '\\' || path[i] == 0) {
			char currPath[1024] = { 0 };
			strncpy(currPath, path, i);
			_mkdir(currPath);
		}
	}
}

void compress(const char* realPath, const char * fileName) {
	getFileList(realPath);
	FILE* fp;
	fp = fopen(fileName, "wb+");
	if (fp == NULL) {
		printf("创建或打开文件出错:%s\n", fileName);
		return;
	}
	fwrite(&fileNums, sizeof(int), 1, fp);
	fwrite(&list, sizeof(FILEINFO), fileNums + 1, fp);
	for (int i = 1; i <= fileNums; i++)
	{
		char fileFullPath[1024];
		char buffer[1024] = {0};
		int pos = findLastChar(realPath, '\\');

		strncpy(fileFullPath, realPath, pos+1);
		fileFullPath[pos + 1] = 0;
		strcat(fileFullPath, list[i].name);

		if (list[i].type == 0) {
			FILE* tmpFile = NULL;
			tmpFile = fopen(fileFullPath, "rb");
			printf("%s\n", fileFullPath);
			if (tmpFile == NULL) {
				printf("读取文件出错: %s\n", fileFullPath);
				tmpFile = NULL;
				break;
			}
			while (!feof(tmpFile)) {
				int count = fread(buffer, sizeof(char), 1024, tmpFile);
				fwrite(buffer, sizeof(char), count, fp);
			}
			if (tmpFile != NULL) {
				fclose(tmpFile);
				tmpFile = NULL;
			}
		}
	}
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
}

void decompress(const char* fileName,const char* path) {
	createDir(path);
	FILE* fp = NULL;
	fp = fopen(fileName, "rb");
	if (fp == NULL) {
		printf("打开文件失败：%s\n", fileName);
		return;
	}
	fread(&fileNums, sizeof(int), 1, fp);
	fread(&list, sizeof(FILEINFO), fileNums + 1, fp);
	
	for (int i = 1; i <= fileNums; i++)
	{
		char fullPath[1024];
		strcpy(fullPath, path);
		strcat(fullPath, "\\");
		strcat(fullPath, list[i].path);
		strcat(fullPath, "\\");
		strcat(fullPath, list[i].name);
		if (list[i].type == 1) {
			createDir(fullPath);
		}
		else {
			FILE* tmpFile = NULL;
			tmpFile = fopen(fullPath, "wb+");
			if (tmpFile == NULL) {
				printf("创建文件失败：%s\n", fullPath);
				break;
			}
			int read = 0;
			while (read < list[i].size) {
				char buffer[1024];
				int rdsize = list[i].size - read > 1024 ? 1024 : list[i].size - read;
				int len = fread(buffer, sizeof(char), rdsize, fp);
				fwrite(buffer, sizeof(char), len, tmpFile);
				read += len;
			}
			if (tmpFile != NULL) {
				fclose(tmpFile);
				tmpFile = NULL;
			}
			printf("%s%s\n", list[i].path, list[i].name);
		}
	}
	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}
	
}

int main(int argc, char* argv[]) {
	
	// getFileList("C:\\Users\\AloneH\\Desktop\\1\\*");
	// compress("D:\\LazyOJ\\Judger\\run\\node_1\\*", "xy.z");
	
	
	// decompress("xy.z", ".\\out\\");
	
	if (strcmp(argv[1], "u") == 0) {
		decompress(argv[2], argv[3]);
	}
	else {
		compress(argv[1], argv[2]);
	}
	
	return 0;
}