#include <iostream>
#include <fstream>
#include <vector>

#define width_adr 0x0012 // bmp width address
#define height_adr 0x0016 // bmp height address
#define paofs_adr 0x000A // pixel array offset address
#define bpp_adr 0x001C // bit per pixel address

using namespace std;

typedef unsigned char byte;

int main(int argc, char **argv)
{
	ifstream ibmp(argv[1], ifstream::binary);
	vector<byte> bmp_data;
	while(ibmp.good())
		bmp_data.push_back(ibmp.get());
	ibmp.close();

	//why is the orders of two output different?
	/*
	cout << (int)bmp_data[0] << ' ' << (int)bmp_data[1] << endl;
	ibmp.open(argv[1], ifstream::binary);
	cout << ibmp.get() << ' ' << ibmp.get() << endl;
	ibmp.close();
	*/

	unsigned int offset;
	for(int i = 3; i >= 0; i--)
	{
		//cout << (int)bmp_data[paofs_adr + i] << ' ';
		offset = offset << 8 | bmp_data[paofs_adr + i];
	}
	cout << "Offset : " << offset << endl;

	unsigned int width;
	for(int i = 3; i >= 0; i--)
	{
		width = width << 8 | bmp_data[width_adr + i];
	}
	cout << "Width : " << width << endl;

	unsigned int height;
	for(int i = 3; i >= 0; i--)
	{
		height = height << 8 | bmp_data[height_adr + i];
	}
	cout << "Height : " << height << endl;

	cout << "PA size(no padding) : "<< width * height * 3 << endl
		<< "PA size(padding) : " << bmp_data.size() - offset << endl;

	unsigned short bpp;
	for(int i = 1; i >= 0; i--)
	{
		//cout << (int)bmp_data[bpp_adr + i] << ' ';
		bpp = bpp << 8 | bmp_data[bpp_adr + i];
	}
	cout << "Bpp : " << bpp << endl;

	int pad_byte = 0;
	if(width * (bpp / 8) % 4 != 0)
	{
		pad_byte = 4 - width * (bpp / 8) % 4;
	}

	for(int i = offset; i < bmp_data.size(); i += width * (bpp / 8) + pad_byte)
	{
		for(int j = 0; j < width * (bpp / 8); j++)
		{
			//Change pixel values here.
		}
	}

	//why is the output bmp file a little different from input bmp file?
	ofstream obmp(argv[2], ofstream::binary);
	for(int i = 0; i < bmp_data.size(); i++)
		obmp.put(bmp_data[i]);
	obmp.close();
	return 0;
}
