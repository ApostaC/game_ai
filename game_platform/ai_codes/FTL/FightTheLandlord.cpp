#include<iostream>
#include<vector>
#include<algorithm>
#include<string>
#include<cstring>
#include "../jsoncpp/json.h" 
using namespace std;

const double eps = 1e-4;
double scores[19] = { 0,0,0,0.3,0.7,0.8,0.8,0.8,1.5,1.5,1.5,1.9,1.9,1.9,0,0,0,1.0,0 };
int CardComboScores[19] = { 0,1,2,6,6,4,4,4,10,8,8,8,8,8,10,10,10,16,0 };
/*0����1���š�2���ӡ�3˳�ӡ�4˫˳��5������6����һ��7��������8ը����9�Ĵ�����ֻ����
10�Ĵ������ԣ���11�ɻ���12�ɻ���С��13�ɻ�������14����ɻ���15����ɻ���С��
16����ɻ�������17�����18�Ƿ�����*/

/* ��->�ȼ�
3 4 5 6 7 8 9 10 J Q  K  A  2 С�� ����
0 1 2 3 4 5 6  7 8 9 10 11 12   13   14*/
inline int Card2Level(int card) { return card / 4 + card / 53; }

int myPosition;
int lastfrom;
int Level_Left[15];
int card_num[3];
int enemymin;

int player_pass[3][3] = { 0 };
int enemy_pass[3] = { 0 };

class CardCombo
{
public:
	int type, num;				//�Ƶ����͡�����
	pair<int, int> value;		//ֵ��level��ʾ

	CardCombo() :type(0) { value.first = value.second = num = 0; }
	CardCombo(int t, int v1, int v2, int n) : type(t), num(n) { value.first = v1; value.second = v2; }
	CardCombo(vector<int> c)
	{
		num = c.size();
		int maxnum = 0, index, t[15] = { 0 };
		for (int i = 0; i < num; ++i)
			t[Card2Level(c[i])]++;
		for (int i = 0; i < 15; ++i)
			if (maxnum < t[i])
				maxnum = t[i], index = i;

		value.second = 0;
		switch (maxnum)
		{
		case(0):type = 0, value.first = 0; break;
		case(1): {
			if (num == 1)
				type = 1, value.first = index;
			else if (num == 2)
				type = 17, value.first = 13;
			else {
				type = 3; value.first = index;
				bool flag = 0;
				for (int i = index; i < 15; ++i)
					if (t[i] == 0 && !flag)
						value.second = i - 1, flag = 1;
			}
			break;
		}
		case(2): {
			if (num == 2)
				type = 2, value.first = index;
			else {
				type = 4; value.first = index;
				bool flag = 0;
				for (int i = index; i < 15; ++i)
					if (t[i] == 0 && !flag)
						value.second = i - 1, flag = 1;
			}
			break;
		}
		case(3): {
			if (num == 3 || num == 4 || num == 5)
				type = num + 2, value.first = index;
			else {
				value.first = index;
				bool flag = 0;
				for (int i = index; i < 15; ++i)
					if (t[i] != 3 && !flag)
						value.second = i - 1, flag = 1;
				type = num / (value.second - value.first + 1) + 8;
			}
			break;
		}
		case(4): {
			if (c.size() == 4 || c.size() == 6)
				type = c.size() / 2 + 6, value.first = index;
			else if (c.size() == 8 && t[index + 1] != t[index])
				type = 10, value.first = index;
			else
				type = 14;			//�����ǲ���
			break;
		}
		}
		

	}
	bool isbiggest()
	{
		if (num > enemymin) return 1;
		switch (type)
		{
		case(1): {
			if (value.first > enemy_pass[0])return 1;
			for (int i = value.first + 1; i < 15; ++i)
				if (Level_Left[i] > 0)
					return 0;
			break;
		}
		case(2): {
			if (value.first > enemy_pass[1])return 1;
			for (int i = value.first + 1; i < 13; ++i)
				if (Level_Left[i] > 1)
					return 0;
			break;
		}
		case(3): {
			int cnt = 0, len = value.second - value.first + 1;
			for (int i = value.first + 1; i < 12; ++i)
			{
				if (Level_Left[i] > 0) ++cnt;
				else cnt = 0;
				if (cnt >= len)
					return 0;
			}
			break;
		}
		case(4): {
			int cnt = 0, len = value.second - value.first + 1;
			for (int i = value.first + 1; i < 12; ++i)
			{
				if (Level_Left[i] > 1) ++cnt;
				else cnt = 0;
				if (cnt >= len)
					return 0;
			}
			break;
		}
		case(5):
		case(6):
		case(7): {
			if (value.first > enemy_pass[2])return 1;
			for (int i = value.first + 1; i < 13; ++i)
				if (Level_Left[i] > 2)
					return 0;
			break;
		}
		case(8):
		case(9):
		case(10): {
			for (int i = value.first + 1; i < 13; ++i)
				if (Level_Left[i] > 3)
					return 0;
			break;
		}
		case(11):
		case(12):
		case(13): {
			int cnt = 0, len = value.second - value.first + 1;
			for (int i = value.first + 1; i < 12; ++i)
			{
				if (Level_Left[i] > 2) cnt++;
				else cnt = 0;
				if (cnt >= len)
					return 0;
			}
			break;
		}
		}
		return 1;
	}
	bool operator<(const CardCombo &t)const
	{
		if (num != t.num)return num > t.num;
		if (type != t.type)return type < t.type;
		return value.first < t.value.first;
	}
};

vector<CardCombo> Valid;	//���Գ�����
vector<int> myDecision;		//�ҵľ���
CardCombo last_combo;		//��һ��������

vector<CardCombo> best, cur;
double maxscore, curscore;
class HandCard
{
public:
	vector<int> cards;	//����
	int Level[15];		//��Ӧ�ĸ��ȼ�����

	HandCard() { cards.clear(); memset(Level, 0, sizeof(Level)); }

	void draw(int c)	//����
	{
		cards.push_back(c);
		Level[Card2Level(c)]++;
	}
	void discard(int c)	//����
	{
		vector<int>::iterator it = find(cards.begin(), cards.end(), c);
		cards.erase(it);
		Level[Card2Level(c)]--;
	}
	void getValid(CardCombo last)		//�õ����кϷ�����
	{
		Valid.clear();
		switch(last.type)
		{
		case(1): {
			for (int i = last.value.first + 1; i < 15; ++i)
				if (Level[i] > 0)
					Valid.push_back(CardCombo(1, i, 0, 1));
			break;
		}
		case(2): {
			for (int i = last.value.first + 1; i < 13; ++i)
				if (Level[i] > 1)
					Valid.push_back(CardCombo(2, i, 0, 2));
			break;
		}
		case(3): {
			int cnt = 0, len = last.value.second - last.value.first + 1;
			for (int i = last.value.first + 1; i < 12; ++i)
			{
				if (Level[i] > 0) ++cnt;
				else cnt = 0;
				if (cnt >= len)
					Valid.push_back(CardCombo(3, i - len + 1, i, len));
			}
			break;
		}
		case(4): {
			int cnt = 0, len = last.value.second - last.value.first + 1;
			for (int i = last.value.first + 1; i < 12; ++i)
			{
				if (Level[i] > 1) ++cnt;
				else cnt = 0;
				if (cnt >= len)
					Valid.push_back(CardCombo(4, i - len + 1, i, 2 * len));
			}
			break;
		}
		case(5):
		case(6):
		case(7): {
			for (int i = last.value.first + 1; i < 13; ++i)
				if (Level[i] > 2)
					Valid.push_back(CardCombo(last.type, i, 0, last.num));
			break;
		}
		case(8):
		case(9):
		case(10): {
			for (int i = last.value.first + 1; i < 13; ++i)
				if (Level[i] > 3)
					Valid.push_back(CardCombo(last.type, i, 0, last.num));
			break;
		}
		case(11):
		case(12):
		case(13): {
			int cnt = 0, len = last.value.second - last.value.first + 1;
			for (int i = last.value.first + 1; i < 12; ++i)
			{
				if (Level[i] > 2) cnt++;
				else cnt = 0;
				if (cnt >= len)
					Valid.push_back(CardCombo(last.type, i - len + 1, i, last.num));
			}
			break;
		}
		}
		if (last.type != 8 && last.type != 17)
			for (int i = 0; i < 13; ++i)
				if (Level[i] > 3)
					Valid.push_back(CardCombo(8, i, 0, 4));
		if (Level[13] && Level[14])
			Valid.push_back(CardCombo(17, 13, 0, 2));
	}
	int getCard(int L)
	{
		int ans;
		for (ans = 0; ans < cards.size(); ans++)
			if (Card2Level(cards[ans]) == L)
				break;
		int t = cards[ans];
		cards.erase(cards.begin() + ans);
		return t;
	}
	void bestDivide(int minnum)
	{
		if (minnum == 13)
		{
			if (maxscore < curscore)
			{
				maxscore = curscore;
				best = cur;
				if (Level[13] && Level[14]) best.push_back(CardCombo(17, 13, 0, 2));
				else if (Level[13])best.push_back(CardCombo(1, 13, 0, 1));
				else if (Level[14])best.push_back(CardCombo(1, 14, 0, 1));
			}
			return;
		}

		if (Level[minnum] == 0)
		{
			bestDivide(minnum + 1);
			return;
		}
		int tmp[15]; memcpy(tmp, Level, sizeof(tmp));
		CardCombo t; bool tt;
		if (Level[minnum] == 1)			//���š�˳��
		{
			Level[minnum]--;
			t = CardCombo(1, minnum, 0, 1); tt = t.isbiggest();
			cur.push_back(t);
			curscore += scores[1] - !tt;
			bestDivide(minnum + 1);
			cur.pop_back(); curscore -= scores[1] - !tt;

			int minlen = 4;
			for (int i = minnum + 1; i < 13; ++i)
			{
				if (minlen > 0 && !Level[i])
					break;
				else if (minlen > 0 && Level[i])
					Level[i]--, minlen--;
				else if (minlen <= 0)
				{
					t = CardCombo(3, minnum, i - 1, i - minnum); tt = t.isbiggest();
					cur.push_back(t);
					curscore += scores[3] - !tt;
					bestDivide(minnum + 1);
					cur.pop_back(); curscore -= scores[3] - !tt;
					if (Level[i]) Level[i]--;
					else break;
				}
			}
			memcpy(Level, tmp, sizeof(tmp));
		}
		else if (Level[minnum] == 2)	//����+˳�ӡ����ӡ�˳��*2��˫˳
		{
			Level[minnum] -= 2;
			t = CardCombo(2, minnum, 0, 2); tt = t.isbiggest();
			cur.push_back(t);
			curscore += scores[2] - !tt;
			bestDivide(minnum + 1);
			cur.pop_back(); curscore -= scores[1] - !tt;

			Level[minnum]++;
			int minlen = 4;
			for (int i = minnum + 1; i < 13; ++i)
			{
				if (minlen > 0 && !Level[i])
					break;
				else if (minlen > 0 && Level[i])
					Level[i]--, minlen--;
				else if (minlen <= 0)
				{
					t = CardCombo(3, minnum, i - 1, i - minnum); tt = t.isbiggest();
					cur.push_back(t);
					curscore += scores[3] - !tt;
					bestDivide(minnum);
					cur.pop_back(); curscore -= scores[3] - !tt;
					if (Level[i]) Level[i]--;
					else break;
				}
			}
			memcpy(Level, tmp, sizeof(tmp));

			Level[minnum] -= 2;
			minlen = 2;
			for (int i = minnum + 1; i < 13; ++i)
			{
				if (minlen > 0 && Level[i] < 2)
					break;
				else if (minlen > 0 && Level[i] >= 2)
					Level[i] -= 2, minlen--;
				else if (minlen <= 0)
				{
					t = CardCombo(4, minnum, i - 1, 2 * (i - minnum)); tt = t.isbiggest();
					cur.push_back(t);
					curscore += scores[4] - !tt;
					bestDivide(minnum + 1);
					cur.pop_back(); curscore -= scores[4] - !tt;
					if (Level[i] >= 2) Level[i] -= 2;
					else break;
				}
			}
			memcpy(Level, tmp, sizeof(tmp));
		}
		else if (Level[minnum] == 3)		//����+˳��*2������+˳�ӣ��������ɻ�
		{
			Level[minnum] -= 3;
			t = CardCombo(5, minnum, 0, 3); tt = t.isbiggest();
			cur.push_back(t);
			curscore += scores[5] - !tt;
			bestDivide(minnum + 1);
			cur.pop_back(); curscore -= scores[5] - !tt;

			Level[minnum] += 2;
			int minlen = 4;
			for (int i = minnum + 1; i < 13; ++i)
			{
				if (minlen > 0 && !Level[i])
					break;
				else if (minlen > 0 && Level[i])
					Level[i]--, minlen--;
				else if (minlen <= 0)
				{
					t = CardCombo(3, minnum, i - 1, i - minnum); tt = t.isbiggest();
					cur.push_back(t);
					curscore += scores[3] - !tt;
					bestDivide(minnum);
					cur.pop_back(); curscore -= scores[3] - !tt;
					if (Level[i]) Level[i]--;
					else break;
				}
			}
			memcpy(Level, tmp, sizeof(tmp));

			minlen = 1;
			for (int i = minnum + 1; i < 13; ++i)
			{
				if (minlen > 0 && Level[i] < 3)
					break;
				else if (minlen > 0 && Level[i] >= 3)
					Level[i] -= 3, minlen--;
				else if (minlen <= 0)
				{
					t = CardCombo(11, minnum, i - 1, 3 * (i - minnum)); tt = t.isbiggest();
					cur.push_back(t);
					curscore += scores[11] - !tt;
					bestDivide(minnum + 1);
					cur.pop_back(); curscore -= scores[11] - !tt;
					if (Level[i] >= 3) Level[i] -= 3;
					else break;
				}
			}
			memcpy(Level, tmp, sizeof(tmp));
		}
		else								//ը����˳��+������
		{
			Level[minnum] -= 4;
			t = CardCombo(8, minnum, 0, 4); tt = t.isbiggest();
			cur.push_back(t);
			curscore += scores[8] - !tt;
			bestDivide(minnum + 1);
			cur.pop_back(); curscore -= scores[8] - !tt;

			Level[minnum] += 3;
			int minlen = 4;
			for (int i = minnum + 1; i < 13; ++i)
			{
				if (minlen > 0 && !Level[i])
					break;
				else if (minlen > 0 && Level[i])
					Level[i]--, minlen--;
				else if (minlen <= 0)
				{
					t = CardCombo(3, minnum, i - 1, i - minnum); tt = t.isbiggest();
					cur.push_back(t);
					curscore += scores[3] - !tt;
					bestDivide(minnum);
					cur.pop_back(); curscore -= scores[3] - !tt;
					if (Level[i]) Level[i]--;
					else break;
				}
			}
			memcpy(Level, tmp, sizeof(tmp));
		}
	}
};

HandCard myCards;

void fetchcard(CardCombo t)
{
	if (t.type == 0) return;
	switch (t.type)
	{
	case(1):myDecision.push_back(myCards.getCard(t.value.first)); break;
	case(2): {
		myDecision.push_back(myCards.getCard(t.value.first));
		myDecision.push_back(myCards.getCard(t.value.first));
		break;
	}
	case(3): {
		for (int i = t.value.first; i <= t.value.second; ++i)
			myDecision.push_back(myCards.getCard(i));
		break;
	}
	case(4): {
		for (int i = t.value.first; i <= t.value.second; ++i) {
			myDecision.push_back(myCards.getCard(i));
			myDecision.push_back(myCards.getCard(i));
		}
		break;
	}
	case(5):
	case(6):
	case(7): {
		for (int i = 0; i < 3; ++i)
			myDecision.push_back(myCards.getCard(t.value.first));
		break;
	}
	case(8):
	case(9):
	case(10):{
		for (int i = 0; i < 4; ++i)
			myDecision.push_back(myCards.getCard(t.value.first));
		break;
	}
	case(11):
	case(12):
	case(13): {
		for (int i = t.value.first; i <= t.value.second; ++i) {
			myDecision.push_back(myCards.getCard(i));
			myDecision.push_back(myCards.getCard(i));
			myDecision.push_back(myCards.getCard(i));
		}
		break;
	}
	case(17): {
		myDecision.push_back(myCards.getCard(13));
		myDecision.push_back(myCards.getCard(14));
	}
	}
}

int level[15];
double dfs(int minnum) {
	if (minnum == 13) return 0;
	if (level[minnum] == 0)
		return dfs(minnum + 1);
	double ans = -200;
	int tmp[15]; bool tt;
	memcpy(tmp, level, sizeof(tmp));
	if (level[minnum] == 1)	//���š�˳��
	{
		level[minnum]--; tt = CardCombo(1, minnum, 0, 1).isbiggest();
		double t = dfs(minnum + 1) + scores[1] - !tt;
		ans = ans > t ? ans : t;

		int minlen = 4;
		for (int i = minnum + 1; i < 13; ++i)
		{
			if (minlen > 0 && !level[i])
				break;
			else if (minlen > 0 && level[i])
				level[i]--, minlen--;
			else if (minlen <= 0)
			{
				tt = CardCombo(3, minnum, i - 1, i - minnum).isbiggest();
				t = dfs(minnum + 1) + scores[3] - !tt;
				ans = ans > t ? ans : t;
				if (level[i]) level[i]--;
				else break;
			}
		}
		memcpy(level, tmp, sizeof(tmp));
		return ans;
	}
	else if (level[minnum] == 2)	//����+˳�ӡ����ӡ�˳��*2��˫˳
	{
		level[minnum] -= 2; tt = CardCombo(2, minnum, 0, 2).isbiggest();
		double t = dfs(minnum + 1) + scores[2] - !tt;
		ans = ans > t ? ans : t;

		level[minnum]++;
		int minlen = 4;
		for (int i = minnum + 1; i < 13; ++i)
		{
			if (minlen > 0 && !level[i])
				break;
			else if (minlen > 0 && level[i])
				level[i]--, minlen--;
			else if (minlen <= 0)
			{
				tt = CardCombo(3, minnum, i - 1, i - minnum).isbiggest();
				t = dfs(minnum) + scores[3] - !tt;
				ans = ans > t ? ans : t;
				if (level[i]) level[i]--;
				else break;
			}
		}
		memcpy(level, tmp, sizeof(tmp));
		level[minnum] -= 2;
		minlen = 2;
		for (int i = minnum + 1; i < 13; ++i)
		{
			if (minlen > 0 && level[i] < 2)
				break;
			else if (minlen > 0 && level[i] >= 2)
				level[i] -= 2, minlen--;
			else if (minlen <= 0)
			{
				tt = CardCombo(4, minnum, i - 1, 2 * (i - minnum)).isbiggest();
				t = dfs(minnum) + scores[4] - !tt;
				ans = ans > t ? ans : t;
				if (level[i] >= 2) level[i] -= 2;
				else break;
			}
		}
		memcpy(level, tmp, sizeof(tmp));
		return ans;
	}
	else if (level[minnum] == 3)		//����+˳��*2������+˳�ӣ��������ɻ�
	{
		level[minnum] -= 3; tt = CardCombo(5, minnum, 0, 3).isbiggest();
		double t = dfs(minnum + 1) + scores[5] - 1;
		ans = ans > t ? ans : t;

		level[minnum] += 2;
		int minlen = 4;
		for (int i = minnum + 1; i < 13; ++i)
		{
			if (minlen > 0 && !level[i])
				break;
			else if (minlen > 0 && level[i])
				level[i]--, minlen--;
			else if (minlen <= 0)
			{
				tt = CardCombo(3, minnum, i - 1, i - minnum).isbiggest();
				t = dfs(minnum) + scores[3] - !tt;
				ans = ans > t ? ans : t;
				if (level[i]) level[i]--;
				else break;
			}
		}
		memcpy(level, tmp, sizeof(tmp));

		level[minnum] -= 3;
		minlen = 1;
		for (int i = minnum + 1; i < 13; ++i)
		{
			if (minlen > 0 && level[i] < 3)
				break;
			else if (minlen > 0 && level[i] >= 3)
				level[i] -= 3, minlen--;
			else if (minlen <= 0)
			{
				tt = CardCombo(11, minnum, i - 1, 3 * (i - minnum)).isbiggest();
				t = dfs(minnum) + scores[11] - !tt;
				ans = ans > t ? ans : t;
				if (level[i] >= 3) level[i] -= 3;
				else break;
			}
		}
		memcpy(level, tmp, sizeof(tmp));
		return ans;
	}
	else								//ը����˳��+������
	{
		level[minnum] -= 4; tt = CardCombo(8, minnum, 0, 4).isbiggest();
		double t = dfs(minnum + 1) + scores[8] - !tt;
		ans = ans > t ? ans : t;

		level[minnum] += 3;
		int minlen = 4;
		for (int i = minnum + 1; i < 13; ++i)
		{
			if (minlen > 0 && !level[i])
				break;
			else if (minlen > 0 && level[i])
				level[i]--, minlen--;
			else if (minlen <= 0)
			{
				tt = CardCombo(3, minnum, i - 1, i - minnum).isbiggest();
				t = dfs(minnum) + scores[3] - !tt;
				ans = ans > t ? ans : t;
				if (level[i]) level[i]--;
				else break;
			}
		}
		memcpy(level, tmp, sizeof(tmp));
		return ans;
	}
}
void getlevel(int *Level, CardCombo t)
{
	memcpy(level, Level, sizeof(level));
	switch (t.type)
	{
	case(1):level[t.value.first]--; break;
	case(2):level[t.value.first] -= 2; break;
	case(3):
	case(4): {
		for (int i = t.value.first; i <= t.value.second; i++)
			level[i] -= t.type - 2;
		break;
	}
	case(5):
	case(6):
	case(7):level[t.value.first] -= 3; break;
	case(8):
	case(9):
	case(10):level[t.value.first] -= 4; break;
	case(11):
	case(12):
	case(13): {
		for (int i = t.value.first; i <= t.value.second; i++)
			level[i] -= 3;
		break;
	}
	case(17): level[13]--, level[14]--; break;
	}
}

CardCombo judge()
{	
	getlevel(myCards.Level, CardCombo());
	double previous = dfs(0), minsets = previous - 1 - 2 * eps;
	int index = -1, leftonecard = -1;
	for (int i = 0; i < Valid.size(); ++i)
	{
		getlevel(myCards.Level, Valid[i]);
		double sets = dfs(0) + scores[Valid[i].type];
		if (sets >= minsets) leftonecard = i;
		if (sets > minsets + eps) index = i, minsets = sets;
	}
	if (index == -1) return CardCombo();
	if (enemymin == last_combo.num) return Valid[leftonecard];
	return Valid[index];
}

void stragety()
{
	for (int i = 0; i < 3; ++i)
		enemy_pass[i] = (myPosition == 0) ? max(player_pass[1][i], player_pass[2][i]) : player_pass[0][i];

	enemymin = myPosition == 0 ? min(card_num[1], card_num[2]) : card_num[0];
	myDecision.clear();
	if (last_combo.type == 0)
	{
		best.clear(); cur.clear(); maxscore = -100; curscore = 0;
		myCards.bestDivide(0);
		sort(best.begin(), best.end());
		CardCombo ans;
		int type_num[19] = { 0 };
		int cnt = 0, index = -1;
		for (int i = 0; i < best.size(); ++i) {
			type_num[best[i].type]++;
			if (!best[i].isbiggest()) {
				cnt++; index = i;
			}
		}
		if (cnt == 1 || cnt == 0)
		{
			if (best.size() == 1)ans = best[0];
			else {
				for (int i = 0; i < best.size(); ++i)
					if (i != index) {
						ans = best[i]; break;
					}
			}
		}
		else {
			for (int j = 0; j < best.size(); ++j) {
				if (best[j].num > enemymin)
					ans = best[j];
				else if (type_num[1] - type_num[5] - 2 * type_num[11] >= 3) {
					if (enemymin == 1) {
						for (int i = best.size() - 1; i >= j; --i)
							if (best[i].type == 1) {
								ans = best[i]; break;
							}
					}
					else {
						for (int i = j; i < best.size(); ++i)
							if (best[i].type == 1) {
								ans = best[i]; break;
							}
					}
				}
				else ans = best[j];
				if (ans.value.first <= 11) break;
			}
		}

		fetchcard(ans);
		if (ans.type == 5) {
			if (type_num[1] || type_num[2]) {
				int minn = 100, t = -1;
				for (int i = 0; i < best.size(); ++i)
					if ((best[i].type == 1 || best[i].type == 2) && minn > best[i].value.first)
						minn = best[i].value.first, t = i;
				fetchcard(best[t]);
			}
		}
		else if (ans.type == 11) {
			int len = ans.value.second - ans.value.first + 1;
			if (type_num[1] >= len) {
				for (int i = 0; i < best.size(); ++i)
					if (best[i].type == 1 && len > 0)
						fetchcard(best[i]), len--;
			}
			else if (type_num[2] >= len) {
				for (int i = 0; i < best.size(); ++i)
					if (best[i].type == 2 && len > 0)
						fetchcard(best[i]), len--;
			}
		}
	}
	else
	{
		best.clear(); cur.clear(); maxscore = -100; curscore = 0;
		myCards.bestDivide(0);
		int cnt = 0;
		for (int i = 0; i < best.size(); ++i)
			if (!best[i].isbiggest()) {
				cnt++;
			}
		if (myPosition == 1 && lastfrom == 2) {
			if (cnt > 1)return;
		}
		else if (myPosition == 2 && lastfrom == 1 && last_combo.value.first >= 11) {
			if (cnt > 1)return;
		}
		Valid.clear();
		myCards.getValid(last_combo);
		if (myPosition == 2 && enemymin == 1 && last_combo.type == 1) {
			if (!Valid.empty()) {
				fetchcard(Valid[Valid.size() - 1]); return;
			}
		}
		CardCombo first = judge();
		fetchcard(first);
		if ((cnt > 1) && (first.type == 8 || first.type == 17) && (myPosition == 2) && (lastfrom == 1))
			myDecision.clear();
		if (first.type == 6 || first.type == 7 || first.type == 9
			|| first.type == 10 || first.type == 12 || first.type == 13)
		{
			best.clear(); cur.clear(); maxscore = -100; curscore = 0;
			myCards.bestDivide(0);
			bool flag = 0;
			if (first.type == 6) {
				for (int i = 0; i < best.size(); ++i)
					if (best[i].type == 1) {
						fetchcard(best[i]), flag = 1; break;
					}
			}
			else if (first.type == 7) {
				for (int i = 0; i < best.size(); ++i)
					if (best[i].type == 2) {
						fetchcard(best[i]), flag = 1; break;
					}
			}
			else if (first.type == 9) {
				int u = 2;
				for (int i = 0; i < best.size(); ++i)
					if (best[i].type == 1 && u > 0)
						fetchcard(best[i]), u--;
				flag = (u == 0);
			}
			else if (first.type == 10) {
				int u = 2;
				for (int i = 0; i < best.size(); ++i)
					if (best[i].type == 2 && u > 0)
						fetchcard(best[i]), u--;
				flag = (u == 0);
			}
			else if (first.type == 12) {
				int u = first.value.second - first.value.first + 1;
				for (int i = 0; i < best.size(); ++i)
					if (best[i].type == 1 && u > 0)
						fetchcard(best[i]), u--;
				flag = (u == 0);
			}
			else if (first.type == 13) {
				int u = first.value.second - first.value.first + 1;
				for (int i = 0; i < best.size(); ++i)
					if (best[i].type == 2 && u > 0)
						fetchcard(best[i]), u--;
				flag = (u == 0);
			}
			if (!flag) myDecision.clear();
		}
	}
}

void init()
{
	Valid.clear();
	myDecision.clear();
	for (int i = 0; i < 13; i++)
		Level_Left[i] = 4;
	Level_Left[13] = Level_Left[14] = 1;
	card_num[0] = 20; card_num[1] = card_num[2] = 17;
	for (int i = 0; i < 3; ++i)
		for (int j = 0; j < 3; ++j)
			player_pass[i][j] = 20;
}

void input()
{
	// �������루ƽ̨�ϵ������ǵ��У�
	string line;
	getline(cin, line);
	Json::Value input;
	Json::Reader reader;
	reader.parse(line, input);

	// ���ȴ����һ�غϣ���֪�Լ���˭������Щ��
	{
		auto firstRequest = input["requests"][0u]; // �±���Ҫ�� unsigned������ͨ�������ֺ����u������
		auto own = firstRequest["own"];
		auto llpublic = firstRequest["public"];
		auto history = firstRequest["history"];
		for (unsigned i = 0; i < own.size(); i++)
		{
			int card = own[i].asInt();
			myCards.draw(card);
			Level_Left[Card2Level(card)]--;
		}
		if (history[0u].size() == 0)
			if (history[1].size() == 0)
				myPosition = 0; // ���ϼҺ��ϼҶ�û���ƣ�˵���ǵ���
			else
				myPosition = 1; // ���ϼ�û���ƣ������ϼҳ����ˣ�˵����ũ���
		else
			myPosition = 2; // ���ϼҳ����ˣ�˵����ũ����
	}

	// history���һ����ϼң��͵ڶ���ϼң��ֱ���˭�ľ���
	int whoInHistory[] = { (myPosition + 1) % 3, (myPosition + 2) % 3 };

	int turn = input["requests"].size();
	CardCombo last;
	for (int i = 0; i < turn; i++)
	{
		// ��λָ����浽��ǰ
		auto history = input["requests"][i]["history"]; // ÿ����ʷ�����ϼҺ����ϼҳ�����
		int howManyPass = 0;
		for (int p = 0; p < 2; p++)
		{
			int player = whoInHistory[p]; // ��˭������
			auto playerAction = history[p]; // ������Щ��
			vector<int> playedCards;
			for (unsigned _ = 0; _ < playerAction.size(); _++) // ѭ��ö������˳���������
			{
				int card = playerAction[_].asInt(); // �����ǳ���һ����
				playedCards.push_back(card);
				Level_Left[Card2Level(card)]--;
			}
			card_num[player] -= playerAction.size();

			if (playerAction.size() > 0) last = CardCombo(playedCards);
			else {
				if (last.type == 1)
					player_pass[player][0] = last.value.first;
				else if (last.type == 2)
					player_pass[player][1] = last.value.first;
				else if (last.type == 5 || last.type == 6 || last.type == 7)
					player_pass[player][2] = last.value.first;
			}

			if (i == turn - 1)
			{
				if (playerAction.size() == 0)
					howManyPass++;
				else
					last_combo = CardCombo(playedCards), lastfrom = player;
			}
		}

		if (howManyPass == 2)
			last_combo = CardCombo();

		if (i < turn - 1)
		{
			// ��Ҫ�ָ��Լ�������������
			auto playerAction = input["responses"][i]; // ������Щ��
			for (unsigned _ = 0; _ < playerAction.size(); _++) // ѭ��ö���Լ�����������
			{
				int card = playerAction[_].asInt(); // �������Լ�����һ����
				myCards.discard(card);				// ���Լ�������ɾ��
			}
			card_num[myPosition] -= playerAction.size();
		}
	}
}

void output()
{
	Json::Value result, response(Json::arrayValue);
	for (int i = 0; i < myDecision.size(); ++i)
		response.append(myDecision[i]);
	result["response"] = response;

	Json::FastWriter writer;
	cout << writer.write(result) << endl;
}

int main()
{
	init();
	input();
	stragety();
	output();
	return 0;
}
