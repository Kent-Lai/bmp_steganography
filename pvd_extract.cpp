#include <iostream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cmath>

#define width_ofst 0x0012 // bmp width offset
#define height_ofst 0x0016 // bmp height offset
#define paofst_ofst 0x000A // pixel array offset offset
#define bpp_ofst 0x001C // bit per pixel offset

using namespace std;

typedef unsigned char byte;
typedef bool bit;

class pixel
{
public:
    unsigned char* a;
    unsigned char* r;
    unsigned char* g;
    unsigned char* b;
};

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

    vector<pixel> pixel_array;
    for(int i = offset; i < bmp_data.size(); i += width * (bpp / 8) + pad_byte)
    {
        for(int j = 0; j < width * (bpp / 8); j += bpp / 8)
        {
            pixel new_pixel;
            new_pixel.b = &bmp_data[i + j];
            new_pixel.g = &bmp_data[i + j + 1];
            new_pixel.r = &bmp_data[i + j + 2];
            pixel_array.push_back(new_pixel);
        }
    }

    int hb[256];
    int lower_bound[256];
    for(int i = 0; i < 8; i++)
    {
        hb[i] = 3;
        lower_bound[i] = 0;
    }
    for(int i = 8; i < 16; i++)
    {
        hb[i] = 3;
        lower_bound[i] = 8;
    }
    for(int i = 16; i < 32; i++)
    {
        hb[i] = 4;
        lower_bound[i] = 16;
    }
    for(int i = 32; i < 64; i++)
    {
        hb[i] = 5;
        lower_bound[i] = 32;
    }
    for(int i = 64; i < 128; i++)
    {
        hb[i] = 6;
        lower_bound[i] = 64;
    }
    for(int i = 128; i < 256; i++)
    {
        hb[i] = 7;
        lower_bound[i] = 128;
    }

	vector<bit> bit_hidden_data;
	unsigned int hidden_data_size = 0;
	int d, hb_mask, hv;
	for(int i = 0; i + 1 < pixel_array.size(); i += 2)
	{
		// RED
		d = abs(*pixel_array[i].r - *pixel_array[i + 1].r);
		hb_mask = (int)pow(2.0, hb[d]) - 1;
		hv = d & hb_mask;
		for(int k = hb[d] - 1; k >= 0; k--)
			bit_hidden_data.push_back(hv >> k & 1);

		if(bit_hidden_data.size() >= 32 && hidden_data_size == 0) // 4 bytes header
        {
			for(int k = 0; k < 32; k++)
                hidden_data_size = hidden_data_size << 1 | bit_hidden_data[k];
            cout << "Hidden data size : " << hidden_data_size << endl;
        }

		if((bit_hidden_data.size() - 32) / 8 >= hidden_data_size && hidden_data_size != 0)
			break;

		// GREEN
		d = abs(*pixel_array[i].g - *pixel_array[i + 1].g);
		hb_mask = (int)pow(2.0, hb[d]) - 1;
		hv = d & hb_mask;
		for(int k = hb[d] - 1; k >= 0; k--)
			bit_hidden_data.push_back(hv >> k & 1);

		if(bit_hidden_data.size() >= 32 && hidden_data_size == 0) // 4 bytes header
        {
			for(int k = 0; k < 32; k++)
                hidden_data_size = hidden_data_size << 1 | bit_hidden_data[k];
            cout << "Hidden data size : " << hidden_data_size << endl;
        }

		if((bit_hidden_data.size() - 32) / 8 >= hidden_data_size && hidden_data_size != 0)
			break;

		// BLUE
		d = abs(*pixel_array[i].b - *pixel_array[i + 1].b);
		hb_mask = (int)pow(2.0, hb[d]) - 1;
		hv = d & hb_mask;
		for(int k = hb[d] - 1; k >= 0; k--)
			bit_hidden_data.push_back(hv >> k & 1);

		if(bit_hidden_data.size() >= 32 && hidden_data_size == 0) // 4 bytes header
        {
			for(int k = 0; k < 32; k++)
                hidden_data_size = hidden_data_size << 1 | bit_hidden_data[k];
            cout << "Hidden data size : " << hidden_data_size << endl;
        }

		if((bit_hidden_data.size() - 32) / 8 >= hidden_data_size && hidden_data_size != 0)
			break;
	}

	vector<byte> byte_hidden_data;
	for(int i = 32; i < 32 + hidden_data_size * 8; i += 8)
	{
		byte t_byte = 0;
		for(int j = 0; j < 8; j++)
			t_byte = t_byte << 1 | bit_hidden_data[i + j];
		byte_hidden_data.push_back(t_byte);
	}

	ofstream o_file(argv[2], ofstream::binary);
	for(int i = 0; i < byte_hidden_data.size() - 1; i++)
		o_file.put(byte_hidden_data[i]);
	o_file.close();

	//why is the output bmp file a little different from input bmp file?
	/*
	ofstream o_bmp(argv[2], ofstream::binary);
	for(int i = 0; i < bmp_data.size(); i++)
		o_bmp.put(bmp_data[i]);
	o_bmp.close();
	*/

	return 0;
}
