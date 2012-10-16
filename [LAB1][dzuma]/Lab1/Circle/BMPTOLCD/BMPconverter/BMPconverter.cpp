/* BMPconverter
This programm convert any BMP file to header file for the CircleOS
*/

#include "stdio.h"
#include "string.h"
#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	FILE *pFile=NULL,*pRes;
	char name[100];
	int s,buff;
	unsigned long int width,offset,j;

	//BMP file opening
	if(argc>1)
	{
		strcpy(name,argv[1]);
		pFile = fopen(name,"rb");
	}
	while (pFile==NULL)
	{
		printf("File to convert : ");
		scanf("%s",name);
		pFile = fopen(name,"rb");
	}

	//Creation of the header file with the same name but the ".h" extension
	j = (int) strlen(name);
	name[j-3]='h';
	name[j-2]='\0';
	pRes = fopen(name,"w");
	printf("Converting BMP file %s.\n",name);

	//Reading the image width in the header
	fseek(pFile,0x12,SEEK_SET);
	if( fread( &width , 4, 1, pFile) != 1 )
	{
		printf("Image width : %u\n",width);
		printf("Invalid BMP file.\n");
		system("PAUSE");
		return 0;
	}
	printf("Image width : %u\n",width);
	if( 8*width > 4096)
		width = 512;

	//Reading the header offset in the header
	fseek(pFile,0xA,SEEK_SET);
	if( fread( &offset , 4, 1, pFile) != 1 )
	{
		printf("Header size : %u\n",offset);
		printf("Invalid BMP file.\n");
		system("PAUSE");
		return 0;
	}

	//From the beggining of the file
	fseek(pFile,0,SEEK_SET);
	j = 0;
	s = -1; //this variable is used to know if the buffer is the first byte of a woard or the second.

	//Copying the header
	buff = fgetc(pFile);
    while ( (buff!=EOF) && (j<offset))
	{
		j++;
		s *= -1;
		if (s==1)
			fprintf(pRes,"0x%02X",(unsigned char)buff); //the first byte needs to be followed by "0x"
		else
			fprintf(pRes,"%02X, ",(unsigned char)buff); //the second byte needs to be ended with ", "
		if ( j%16==0 )									//while in the header line are 16 bytes long
			fprintf(pRes,"\n",j);
		buff = fgetc(pFile);
	}
	fprintf(pRes,"\n",j);

	//Copying the image
	s=-1;
    while ( buff!=EOF )
	{
		j++;
		s *= -1;
		if (s==1)
			fprintf(pRes,"0x%02X",(unsigned char)buff);
		else
			fprintf(pRes,"%02X, ",(unsigned char)buff);

		if ( j%(2*width)==0 ) //line return when reaching the end of the image width
			fprintf(pRes,"\n",j);

		buff = fgetc(pFile);
	}

	fclose(pFile);
	fclose(pRes);
	printf( "Done.\n");
	system("Pause");
	return 0;
}