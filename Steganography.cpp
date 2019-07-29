#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <bitset>
#include <limits>

using namespace std;

struct picture {
	long type;
	long size;
	long offset;
	long width;
	long height;
};
struct variance {
	long offset;
	long variance2;
	int color;
};

void quickSort(vector<variance> &varColor, int left, int right) {
	int l = left, r = right;
	vector<variance> tmp(1);
	int pivot = varColor[(left + right) / 2].variance2;

	while (l <= r) {
		while (varColor[l].variance2 < pivot)
			l++;
		while (varColor[r].variance2 > pivot)
			r--;
		if (l <= r) {
			tmp[0] = varColor[l];
			varColor[l] = varColor[r];
			varColor[r] = tmp[0];
			l++;
			r--;
		}
	}

	if (left < r)
		quickSort(varColor, left, r);
	if (l < right)
		quickSort(varColor, l, right);
}

void varianceFunction(picture bmp, vector<int> colortemp, vector<long> image, vector<long> temp, int widthJunk, int &varColor_counter, vector<variance> &varColor, int color) {
	int a = 0;
	for (int w = 0; w<bmp.height / 8;w++) {
		for (int z = 0; z<bmp.width / 8;z++) {
			for (int j = 0; j<8; j++) {
				for (int i = 0; i<8; i++) {
					colortemp[a] = image[bmp.offset + color + i * 3 + bmp.width * 3 * j + j*widthJunk + z * 24 + w*bmp.width * 3 * 3 + w*widthJunk * 3];
					colortemp[a] = colortemp[a] & 254;
					a++;
				}
			}
			temp[0] = 0;
			for (int i = 0; i<64;i++) {
				temp[0] = colortemp[i] + temp[0];
			}
			temp[0] = temp[0] / 64;
			temp[1] = 0;
			for (int i = 0; i<64;i++) {
				temp[1] = (colortemp[i] - temp[0])*(colortemp[i] - temp[0]) + temp[1];
			}
			temp[1] = temp[1] / 64;
			varColor_counter++;
			varColor[varColor_counter].variance2 = temp[1];
			varColor[varColor_counter].color = color;
			varColor[varColor_counter].offset = bmp.offset + color + z * 24 + w*bmp.width * 3 * 3 + w*widthJunk * 3;
			a = 0;
		}
	}
}

void savePicture(ifstream &img, picture &bmp, vector<long> temp, int &widthJunk, vector<long> &image) {
	img.seekg(10, img.beg);
	bmp.offset = img.get();

	img.seekg(18, img.beg);
	temp[0] = img.get();
	img.seekg(19, img.beg);
	temp[1] = img.get();
	for (int i = 0; i<8; i++)
		temp[1] = temp[1] * 2;
	bmp.width = temp[0] + temp[1];

	img.seekg(22, img.beg);
	temp[0] = img.get();
	img.seekg(23, img.beg);
	temp[1] = img.get();
	for (int i = 0; i<8; i++)
		temp[1] = temp[1] * 2;
	bmp.height = temp[0] + temp[1];

	if (widthJunk != 0)
		widthJunk = 4 - ((bmp.width * 3) % 4);

	img.seekg(2, img.beg);
	temp[0] = img.get();
	img.seekg(3, img.beg);
	temp[1] = img.get();
	img.seekg(4, img.beg);
	temp[2] = img.get();
	img.seekg(5, img.beg);
	temp[3] = img.get();
	for (int i = 0; i<24; i++)
		temp[3] = temp[3] * 2;
	for (int i = 0; i<16; i++)
		temp[2] = temp[2] * 2;
	for (int i = 0; i<8; i++)
		temp[1] = temp[1] * 2;
	bmp.size = temp[0] + temp[1] + temp[2] + temp[3];

	img.seekg(0, img.beg);
	image.resize(bmp.size);
	for (int i = 0; i<image.size(); i++) {
		image[i] = img.get();
	}
}

void encryptFunction(string &msg, int randomSeed, vector<long> temp, picture bmp, int widthJunk, vector<variance> varColor, vector<long> &image, vector<char> &buffer) {
	msg.push_back('\n');
	int w = 0;
	srand(randomSeed);
	for (int j = 0; j < msg.length(); j++) {
		int a = int(msg.at(j));
		for (int q = 0;q < 8;q++) {
			temp[q] = a % 2;
			if (a % 2)
				a--;
			a = a / 2;
		}

		for (int i = 0;i < 8;i++) {
			int key = rand() % 64;
			temp[9] = key / 8;
			key = (key * 3) + bmp.width * 3 * temp[9] + temp[9] * widthJunk;
			temp[8] = varColor[w].offset + key;
			if (image[temp[8]] % 2 && temp[i] % 2) { ; }
			if (!(image[temp[8]] % 2) && !(temp[i] % 2)) { ; }
			if (!(image[temp[8]] % 2) && (temp[i] % 2)) {
				image[temp[8]] = image[temp[8]] + 0x00000001;
			}
			if ((image[temp[8]] % 2) && !(temp[i] % 2)) {
				image[temp[8]] = image[temp[8]] - 0x00000001;
			}
			w++;
		}
	}

	buffer.resize(image.size());
	for (int i = 0;i < image.size();i++)
		buffer[i] = image[i];
}

void decryptFunction(string &msg, int randomSeed, vector<long> temp, picture bmp, int widthJunk, vector<variance> varColor, vector<long> image, vector<char> &codedmsg, int &correctMsg) {
	int w = 0;
	srand(randomSeed);
	for (int j = 0; j < varColor.size(); j++) {

		for (int i = 0;i < 8;i++) {
			int key = rand() % 64;
			temp[9] = key / 8;
			key = (key * 3) + bmp.width * 3 * temp[9] + temp[9] * widthJunk;
			temp[8] = varColor[w].offset + key;
			temp[i] = image[temp[8]] & 0x00000001;
			w++;
		}

		int a = temp[0] + 2 * temp[1] + 4 * temp[2] + 8 * temp[3] + 16 * temp[4] + 32 * temp[5] + 64 * temp[6] + 128 * temp[7];

		if (a == '\n') {
			codedmsg.push_back(a);
			correctMsg = 1;
			break;
		}

		codedmsg.push_back(a);

	}

	if (correctMsg == 1) {
		for (int i = 0;i < codedmsg.size(); i++) {
			cout << codedmsg[i];
		}
	}
	else {
		cout << "No message found.";
	}
}

int main() {

	vector<char> codedmsg;

	int commandCode = 2;
	string command;
	while (true) {
		cout << "Enter command : ";
		cin >> command;
		if (command == "encrypt") {
			commandCode = 0;
			break;
		}
		else if (command == "decrypt") {
			commandCode = 1;
			break;
		}
		else cout << "No such command. Please try again." << endl;
	}

	ifstream img;
	while (true) {
		string picName;
		cout << "Enter picture's name : ";
		cin >> picName;
		img.open(picName.c_str(), ios::in | ios::binary);
		if (!img) {
			cerr << "can't open file [" << picName << "]. Please try again.\n";
			continue;
		}
		break;
	}

	int randomSeed;
	int color;
	int correctMsg = 0;
	string msg;
	int varColor_counter = -1;
	vector<long> image;
	vector<long> temp(10);
	vector<int> colortemp(64);
	int widthJunk;
	vector<char> buffer;
	picture bmp;
	vector<variance> varColor(100);
	vector<variance> reverse(100);

	savePicture(img, bmp, temp, widthJunk, image);

	temp[3] = (bmp.height / 8)*(bmp.width / 8) * 3;
	varColor.resize(temp[3]);
	reverse.resize(temp[3]);

	color = 0;
	varianceFunction(bmp, colortemp, image, temp, widthJunk, varColor_counter, varColor, color);
	color = 1;
	varianceFunction(bmp, colortemp, image, temp, widthJunk, varColor_counter, varColor, color);
	color = 2;
	varianceFunction(bmp, colortemp, image, temp, widthJunk, varColor_counter, varColor, color);

	temp[3] = (bmp.height / 8)*(bmp.width / 8) * 3;


	int l = 0;
	int r = temp[3] - 1;

	for (int i = 0;i < temp[3];i++)
		for (int j = i;j < temp[3];j++) {
			if (varColor[i].variance2 == 0)
				break;
			if (varColor[i].variance2 == varColor[j].variance2)
				varColor[j].variance2 = 0;
		}

	quickSort(varColor, l, r);

	for (int i = 0;i < temp[3];i++)
		reverse[i] = varColor[temp[3] - 1 - i];
	for (int i = 0;i < temp[3];i++)
		varColor[i] = reverse[i];

	while (true) {
		cout << "Enter a key with 4 to 6 digits : ";
		cin >> randomSeed;
		if (randomSeed<1000 || randomSeed>999999) {
			cout << "the key is too long or too short. please try again." << endl;
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			continue;
		}
		break;
	}

	if (commandCode == 0) {
		while (true) {
			cout << "Enter the message : ";
			cin.clear();
			cin.ignore(numeric_limits<streamsize>::max(), '\n');
			getline(cin, msg);
			if ((msg.length() + 1) * 8 > varColor.size()) {
				cout << "the message is too long. please try again." << endl;
				continue;
			}
			break;
		}
	}

	if (commandCode == 0) {
		encryptFunction(msg, randomSeed, temp, bmp, widthJunk, varColor, image, buffer);

		ofstream pic("output.bmp", ios::out | ios::binary);
		pic.write(&buffer[0], image.size());
	}

	if (commandCode == 1) {
		decryptFunction(msg, randomSeed, temp, bmp, widthJunk, varColor, image, codedmsg, correctMsg);
	}
	img.close();
}
