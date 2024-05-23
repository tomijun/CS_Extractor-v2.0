#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cmath>
#include "SFmpqapi.h"
#include "SFmpq_static.h"
#include "SFmpqapi_no-lib.h"

using namespace std;
typedef unsigned long long QWORD;

int f_Sopen(MPQHANDLE hMPQ, LPCSTR lpFileName, MPQHANDLE* hFile)
{
    // SFileOpenFileEx([MPQ핸들], [파일명], 0, &[리턴받을 파일핸들]);
    if (!SFileOpenFileEx(hMPQ, lpFileName, 0, hFile)) {
        printf("%s이(가) 존재하지 않습니다. [%d]\n", lpFileName, GetLastError());
        exit(-1);
    }
}

int f_Scopy(MPQHANDLE hMPQ, MPQHANDLE* hFile, LPCSTR foutName, FILE** fout)
{
    fopen_s(fout, foutName, "w+b");
    DWORD fsize = SFileGetFileSize(*hFile, NULL);
    char buffer[4096] = { 0 };
    while (fsize > 0) {
        DWORD transfersize = min(4096, fsize);
        DWORD readbyte;
        SFileReadFile(*hFile, buffer, transfersize, &readbyte, NULL); //fread에 해당
        if (readbyte != transfersize) {
            printf("SFileReadFile read %d bytes / %d bytes expected.\n", readbyte, transfersize);
            return -1;
        }
        fwrite(buffer, 1, readbyte, *fout);
        fsize -= transfersize;
    }
}

DWORD Ret1 = 0;
DWORD Ret2 = 0;
void GetChkSection(FILE* fp, const char* Name)
{
    DWORD Key = Name[0] + Name[1] * 256 + Name[2] * 65536 + Name[3] * 16777216;
    int ret = 0, size;
    fseek(fp, 0, 2);
    size = ftell(fp);
    fseek(fp, 0, 0);

    DWORD Section[2];
    DWORD Check = 0;
    for (int i = 0; i < size;)
    {
        fseek(fp, i, 0);
        fread(Section, 4, 2, fp);
        if (Section[0] == Key)
        {
            Ret1 = i;
            Ret2 = i + (Section[1] + 8);
            Check = 1;
            break;
        }
        else
            i += (Section[1] + 8);
    }
    if (Check == 0)
        printf("맵의 %s단락을 찾을 수 없습니다.", Name);
    return;
}

int SCount = 0;
void CS_Extract(FILE* fin, FILE* fout)
{
    GetChkSection(fin, "UNIT");
    int UNIToff1 = Ret1;
    int UNIToff2 = Ret2;
    int fsize = UNIToff2 - UNIToff1 - 8;
    int UNIToff = UNIToff1 + 8;
    printf("UNIT Section : 0x%X, %dbytes\n\n", UNIToff1, fsize);
    int line = 0;
    while (true) {
        int PlayerId, UnitId, Count=0, Player = 0;
        printf("플레이어(0입력시 종료), 유닛ID 입력 : P");
        scanf_s("%d %d", &PlayerId, &UnitId);
        fflush(stdin);
        if (PlayerId == 0)
            break;
        SCount++;
        Player = PlayerId;
        PlayerId--;
        int Loc = 0, temp;
        fprintf(fout, "Shape0000 = {    ");
        for (int i = 0; i < fsize; i += 0x24)
        {
            int LocX = 0, LocY = 0, Pid = 0, Uid = 0;
            fseek(fin, UNIToff + i + 8, 0);
            fread(&Uid, 2, 1, fin);
            fseek(fin, UNIToff + i + 16, 0);
            fread(&Pid, 1, 1, fin);

            if (Pid == PlayerId && Uid == UnitId)
            {
      
                fseek(fin, UNIToff + i + 4, 0);
                fread(&LocX, 2, 1, fin);
                fread(&LocY, 2, 1, fin);
                fprintf(fout, ",{%d, %d}", LocX, LocY);
                Count++;
                int Index = i/0x24;
                printf("Index=%d → {%d, %d} (%d)\n", Index, LocX, LocY, Count);
            }
        }
        fprintf(fout, "}");
        temp = ftell(fout);
        fseek(fout, strlen("Shape0000 = {") + line, 0);
        fprintf(fout, "%d", Count);
        fseek(fout, strlen("Shape") + line, 0);
        fprintf(fout, "%X", Player);
        if (UnitId < 10)
        {
            fseek(fout, strlen("Shape000") + line, 0);
            printf("Shape%X00%d : Total %d Unit Converted\n", Player, UnitId,  Count);
        }
        else if (UnitId < 100)
        {
            fseek(fout, strlen("Shape00") + line, 0);
            printf("Shape%X0%d : Total %d Unit Converted\n", Player, UnitId, Count);
        }
        else
        {
            fseek(fout, strlen("Shape0") + line, 0);
            printf("Shape%X%d : Total %d Unit Converted\n", Player, UnitId, Count);
        }
        fprintf(fout, "%d", UnitId);
        fseek(fout, temp, 0);
        fprintf(fout, "\n");
        line = ftell(fout);
    }
    return;
}

int main(int argc, char* argv[])
{

    FILE* fout, * fin;
    MPQHANDLE hMPQ;
    MPQHANDLE hFile;
    // Open MPQ

    printf("--------------------------------------\n     。`+˚CS_Extractor v2.0 。 + .˚\n--------------------------------------\n\t\t\tMade By Ninfia\n");

    char* input = argv[1];
    // Test
    //char input2[] = "1.scx";
    //if (argc == 1)
    //    input = input2;
    // Test

    if (argc == 1) // Selected No file
    {
        printf("선택된 파일이 없습니다.\n");
        system("pause");
        return 0;
    }

    char iname[512];
    strcpy_s(iname, 512, input);
    int ilength = strlen(iname);
    iname[ilength - 4] = '_';
    iname[ilength - 3] = 'o';
    iname[ilength - 2] = 'u';
    iname[ilength - 1] = 't';
    iname[ilength - 0] = '.';
    iname[ilength + 1] = 't';
    iname[ilength + 2] = 'x';
    iname[ilength + 3] = 't';
    iname[ilength + 4] = 0;

    if (!SFileOpenArchive(input, 0, 0, &hMPQ)) {
        // SFileOpenArchive([파일명], 0, 0, &[리턴받을 MPQ핸들]);
        printf("SFileOpenArchive failed. [%d]\n", GetLastError());
        return -1;
    }
    printf("%s 의 MPQ 로드 완료\n", input);
    // Open Files
    f_Sopen(hMPQ, "staredit\\scenario.chk", &hFile);
    f_Scopy(hMPQ, &hFile, "scenario.chk", &fout);

    fseek(fout, 0, 2);
    int chksize = ftell(fout);
    fseek(fout, 0, 0);
    printf("scenario.chk %d bytes\n", chksize);
    SFileCloseFile(hFile);
    SFileCloseArchive(hMPQ);
    fclose(fout);

    // Extract UNIT
    fopen_s(&fin, "scenario.chk", "rb");
    fopen_s(&fout, iname, "w+");

    CS_Extract(fin, fout);

    fclose(fout);
    fclose(fin);
    DeleteFileA("scenario.chk");
    printf("\n유닛 좌표 → 도형 데이터 추출을 마칩니다.\n");
    printf("%s 로 저장됨 (총 %d개의 Shape)\n", iname, SCount);
    system("pause");
    return 0;
}