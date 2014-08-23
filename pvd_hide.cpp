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

int add_to_limit(unsigned char* p_var, int sign, int expected_add_num, int limit);
bool modify_d(unsigned char* p_var1, unsigned char* p_var2, int new_d);

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
			//Change pixel values here.
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

	int bit_count = 0;
	int old_d, hv, new_d;
	for(int i = 0; i + 1 < pixel_array.size(); i += 2)
	{	
		// RED
		old_d = abs(*pixel_array[i].r - *pixel_array[i + 1].r);
		hv = 0;
		for(int j = 0; j < hb[old_d]; j++)
		{
			hv = hv << 1 | bit_hidden_data[bit_count];
			if(bit_count == bit_hidden_data.size() - 1)
				continue;
			bit_count++;
		}
		new_d = lower_bound[old_d] | hv;

		//cout << "Old pair : " << (int)*pixel_array[i].r << ' ' << (int)*pixel_array[i + 1].r << endl;

		if(!modify_d(pixel_array[i].r, pixel_array[i + 1].r, new_d))
		{
			cout << "New pair : " << (int)*pixel_array[i].r << ' ' << (int)*pixel_array[i + 1].r << endl;
			cout << "Expected new d : " << new_d << endl;
			cout << "Real new d : " << abs(*pixel_array[i].r - *pixel_array[i + 1].r) << endl;
			cout << "modify_d fail" << endl;
			return 1;
		}

		if(bit_count == bit_hidden_data.size() - 1)
			break;

		// GREEN
		old_d = abs(*pixel_array[i].g - *pixel_array[i + 1].g);
		hv = 0;
		for(int j = 0; j < hb[old_d]; j++)
		{
			hv = hv << 1 | bit_hidden_data[bit_count];
			if(bit_count == bit_hidden_data.size() - 1)
				continue;
			bit_count++;
		}
		new_d = lower_bound[old_d] | hv;

		//cout << "Old pair : " << (int)*pixel_array[i].g << ' ' << (int)*pixel_array[i + 1].g << endl;

		if(!modify_d(pixel_array[i].g, pixel_array[i + 1].g, new_d))
		{
			cout << "New pair : " << (int)*pixel_array[i].g << ' ' << (int)*pixel_array[i + 1].g << endl;
			cout << "Expected new d : " << new_d << endl;
			cout << "Real new d : " << abs(*pixel_array[i].g - *pixel_array[i + 1].g) << endl;
			cout << "modify_d fail" << endl;
			return 1;
		}

		if(bit_count == bit_hidden_data.size() - 1)
			break;

		// BLUE
		old_d = abs(*pixel_array[i].b - *pixel_array[i + 1].b);
		hv = 0;
		for(int j = 0; j < hb[old_d]; j++)
		{
			hv = hv << 1 | bit_hidden_data[bit_count];
			if(bit_count == bit_hidden_data.size() - 1)
				continue;
			bit_count++;
		}
		new_d = lower_bound[old_d] | hv;

		//cout << "Old pair : " << (int)*pixel_array[i].b << ' ' << (int)*pixel_array[i + 1].b << endl;

		if(!modify_d(pixel_array[i].b, pixel_array[i + 1].b, new_d))
		{
			cout << "New pair : " << (int)*pixel_array[i].b << ' ' << (int)*pixel_array[i + 1].b << endl;
			cout << "Expected new d : " << new_d << endl;
			cout << "Real new d : " << abs(*pixel_array[i].b - *pixel_array[i + 1].b) << endl;
			cout << "modify_d fail" << endl;
			return 1;
		}

		if(bit_count == bit_hidden_data.size() - 1)
			break;
	}

	//why is the output bmp file a little different from input bmp file?
	ofstream o_bmp(argv[3], ofstream::binary);
	for(int i = 0; i < bmp_data.size(); i++)
		o_bmp.put(bmp_data[i]);
	o_bmp.close();
	return 0;
}

int add_to_limit(unsigned char* p_var, int sign, int expected_add_num, int limit)
{
	if(sign > 0)
		while(*p_var < limit && expected_add_num > 0)
		{
			*p_var += sign;
			expected_add_num--;
		}
	else
		while(*p_var > limit && expected_add_num > 0)
		{
			*p_var += sign;
			expected_add_num--;
		}

	return expected_add_num;
}

bool modify_d(unsigned char* p_var1, unsigned char* p_var2, int new_d)
{
	int old_d = abs(*p_var1 - *p_var2);
	int expected_add_num1 = ceil(abs(old_d - new_d) / 2.0);
	int expected_add_num2 = floor(abs(old_d - new_d) / 2.0);

	int remain1, remain2, remain;
	if(old_d >= new_d) // shrink d
	{
		if(*p_var1 >= *p_var2)
		{
			remain1 = add_to_limit(p_var1, -1, expected_add_num1, 0);
			remain2 = add_to_limit(p_var2, 1, expected_add_num2, 255);

			if(remain1 != 0)
			{
				remain = add_to_limit(p_var2, 1, remain1, 255);
			}
			else if(remain2 != 0)
				{
					remain = add_to_limit(p_var1, -1, remain2, 0);
				}
		}
		else // *p_var1 < *p_var2
		{
			remain1 = add_to_limit(p_var1, 1, expected_add_num1, 255);
			remain2 = add_to_limit(p_var2, -1, expected_add_num2, 0);

			if(remain1 != 0)
			{
				remain = add_to_limit(p_var2, -1, remain1, 0);
			}
			else if(remain2 != 0)
				{
					remain = add_to_limit(p_var1, 1, remain2, 255);
				}
		}
	}
	else // expand d
	{
		if(*p_var1 >= *p_var2)
		{
			remain1 = add_to_limit(p_var1, 1, expected_add_num1, 255);
			remain2 = add_to_limit(p_var2, -1, expected_add_num2, 0);

			if(remain1 != 0)
			{
				remain = add_to_limit(p_var2, -1, remain1, 0);
			}
			else if(remain2 != 0)
				{
					remain = add_to_limit(p_var1, 1, remain2, 255);
				}
		}
		else // *p_var1 < *p_var2
		{
			remain1 = add_to_limit(p_var1, -1, expected_add_num1, 0);
			remain2 = add_to_limit(p_var2, 1, expected_add_num2, 255);

			if(remain1 != 0)
			{
				remain = add_to_limit(p_var2, 1, remain1, 255);
			}
			else if(remain2 != 0)
				{
					remain = add_to_limit(p_var1, -1, remain2, 0);
				}
		}
	}

	if(abs(*p_var1 - *p_var2) != new_d)
		return false;
	else
		return true;
}
