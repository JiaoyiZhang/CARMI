#ifndef LONGITUDES_H
#define LONGITUDES_H

#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

class LongitudesDataset
{
public:
	LongitudesDataset(double initRatio)
	{
		if (initRatio == 0)
			num = 0;
		else if (initRatio == 1)
			num = -1;
		else
			num = initRatio / (1 - initRatio);
	}

	void GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset);

private:
	int num;
};

void LongitudesDataset::GenerateDataset(vector<pair<double, double>> &initDataset, vector<pair<double, double>> &insertDataset)
{
	vector<pair<double, double>>().swap(initDataset);
	vector<pair<double, double>>().swap(insertDataset);

	vector<pair<double, double>> ds;
	ifstream inFile("../src/dataset/iranLon.csv", ios::in);
	if (!inFile)
	{
		cout << "打开文件失败！" << endl;
		exit(1);
	}
	string line;
	while (getline(inFile, line))
	{
		if (line.empty())
			continue;
		istringstream sin(line);
		vector<string> fields;
		string field;
		while (getline(sin, field, ','))
			fields.push_back(field);
		string key = fields[0];
		string value = fields[1];
		double k = stod(key);
		double v = stod(value);
		ds.push_back({k, v});
	}

	std::sort(ds.begin(), ds.end());
	if (num == 0)
	{
		int i = 0;
		for (; i < 0.6 * ds.size(); i++)
			initDataset.push_back(ds[i]);
		for (; i < 0.9 * ds.size(); i += 2)
		{
			initDataset.push_back(ds[i]);
			insertDataset.push_back(ds[i + 1]);
		}
		for (; i < ds.size(); i++)
			initDataset.push_back(ds[i]);
	}
	else if (num == -1)
	{
		for (int i = 0; i < ds.size(); i++)
			initDataset.push_back(ds[i]);
	}
	else
	{
		int cnt = 0;
		for (int i = 0; i < ds.size(); i++)
		{
			cnt++;
			if (cnt <= num)
				initDataset.push_back(ds[i]);
			else
			{
				insertDataset.push_back(ds[i]);
				cnt = 0;
			}
		}
	}
	cout << "longitudes: Read size:" << initDataset.size() << "\tWrite size:" << insertDataset.size() << endl;
}

#endif