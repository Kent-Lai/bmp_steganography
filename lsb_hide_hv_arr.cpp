#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <cstdlib>

#define width_ofst 0x0012 // bmp width offset
#define height_ofst 0x0016 // bmp height offset
#define paofst_ofst 0x000A // pixel array offset offset
#define bpp_ofst 0x001C // bit per pixel offset

using namespace std;

typedef unsigned char byte;
typedef bool bit;

int main(int argc, char **argv)
{
	ifstream i_bmp(argv[1], ifstream::binary);
	vector<byte> bmp_data;
	while(i_bmp.good())
		bmp_data.push_back(i_bmp.get());
	i_bmp.close();

	//why is the orders of two output different?
	/*
	cout << (int)bmp_data[0] << ' ' << (int)bmp_data[1] << endl;
	i_bmp.open(argv[1], ifstream::binary);
	cout << i_bmp.get() << ' ' << i_bmp.get() << endl;
	i_bmp.close();
	*/

	unsigned int offset;
	for(int i = 3; i >= 0; i--)
	{
		//cout << (int)bmp_data[paofs_ofst + i] << ' ';
		offset = offset << 8 | bmp_data[paofst_ofst + i];
	}
	cout << "Offset : " << offset << endl;

	unsigned int width;
	for(int i = 3; i >= 0; i--)
	{
		width = width << 8 | bmp_data[width_ofst + i];
	}
	cout << "Width : " << width << endl;

	unsigned int height;
	for(int i = 3; i >= 0; i--)
	{
		height = height << 8 | bmp_data[height_ofst + i];
	}
	cout << "Height : " << height << endl;

	unsigned short bpp;
	for(int i = 1; i >= 0; i--)
	{
		//cout << (int)bmp_data[bpp_ofst + i] << ' ';
		bpp = bpp << 8 | bmp_data[bpp_ofst + i];
	}
	cout << "Bpp : " << bpp << endl;

	cout << "PA size(no padding) : "<< width * height * (bpp / 8) << endl;

	int pad_byte = 0;
	if(width * (bpp / 8) % 4 != 0)
	{
		pad_byte = 4 - width * (bpp / 8) % 4;
	}
	cout << "PA size(padding) : "<< (width * (bpp / 8) + pad_byte) * height << endl;

	int hb = atoi(argv[4]);
	if(hb > 8)
		hb = 8;
	cout << "Hidden bits per byte: " << hb << endl;

	cout << "Max hidden data size : " << width * height * (bpp / 8) * hb / 8 - 4 << endl;

	ifstream i_hd(argv[2], ifstream::binary);
	vector<byte> byte_hidden_data;
	while(i_hd.good())
		byte_hidden_data.push_back(i_hd.get());
	i_hd.close();

	cout << "Hidden data size : " << byte_hidden_data.size() << endl;

	unsigned int hidden_data_size = byte_hidden_data.size();
	for(int i = 0; i < 4; i++)
		byte_hidden_data.insert(byte_hidden_data.begin(), hidden_data_size >> (i * 8) & 0xff);

	int header = 0;
	for(int i = 0; i < 4; i++)
		header = header << 8 | byte_hidden_data[i];
	cout << "Check byte header : " << header << endl;

	vector<bit> bit_hidden_data;
	int exp2[8];
	for(int i = 0; i < 8; i++)
		exp2[i] = pow(2.0, i);
	for(int i = 0; i < byte_hidden_data.size(); i++)
	{
		byte t_byte = byte_hidden_data[i];
		for(int j = 7; j >= 0; j--)
		{
			bit_hidden_data.push_back(t_byte / exp2[j]);
			t_byte %= exp2[j];
		}
	}

	for(int i = 0; i < 32; i++)
		header = header << 1 | bit_hidden_data[i];
	cout << "Check bit header : " << header << endl;

	vector<int> hv;
	int bit_count = 0;
	while(bit_count < bit_hidden_data.size() - 1)
	{
		int t_hv = 0;
		for(int i = 0; i < hb; i++)
		{
			t_hv = t_hv << 1 | bit_hidden_data[bit_count];
			if(bit_count == bit_hidden_data.size() - 1)
				continue;
			bit_count++;
		}
		hv.push_back(t_hv);
	}

	ofstream o_hv(argv[5]);
	for(int i = 0; i < hv.size(); i++)
	{
		o_hv << hv[i] << endl;
	}
	o_hv.close();

	int hb_mask = ~((int)pow(2.0, hb) - 1);
	int idx = 0;
	#pragma parellel for
	for(int i = offset; i < bmp_data.size(); i += width * (bpp / 8) + pad_byte)
	{
		bool is_end = false;
		for(int j = 0; j < width * (bpp / 8); j++)
		{
			//Change pixel values here.
			bmp_data[i + j] = (bmp_data[i + j] & hb_mask) | hv[idx++];

			if(idx == hv.size())
			{
				is_end = true;
				break;
			}
		}

		if(is_end)
		{
			break;
		}
	}

	//why is the output bmp file a little different from input bmp file?
	ofstream o_bmp(argv[3], ofstream::binary);
	for(int i = 0; i < bmp_data.size() - 1; i++)
		o_bmp.put(bmp_data[i]);
	o_bmp.close();

	return 0;
}
