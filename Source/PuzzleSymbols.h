#pragma once
#include <vector>
#include <map>
#include "Panel.h"
#include "Random.h"

struct PuzzleSymbols {
	std::map<int, std::vector<std::pair<int, int>>> symbols;
	std::vector<std::pair<int, int>>& operator[](int symbolType) { return symbols[symbolType]; }
	int style;
	int getNum(int symbolType) {
		int total = 0;
		for (auto& pair : symbols[symbolType]) total += pair.second;
		return total;
	}
	bool any(int symbolType) { return symbols[symbolType].size() > 0; }
	int popRandomSymbol() {
		std::vector<int> types;
		for (auto& pair : symbols)
			if (pair.second.size() > 0 && pair.first != Decoration::Start && pair.first != Decoration::Exit && pair.first != Decoration::Gap && pair.first != Decoration::Eraser)
				types.push_back(pair.first);
		int randType = types[Random::rand() % types.size()];
		int randIndex = Random::rand() % symbols[randType].size();
		while (symbols[randType][randIndex].second == 0 || symbols[randType][randIndex].second >= 25) {
			randType = types[Random::rand() % types.size()];
			randIndex = Random::rand() % symbols[randType].size();
		}
		symbols[randType][randIndex].second--;
		return symbols[randType][randIndex].first;
	}
	PuzzleSymbols(std::vector<std::pair<int, int>> symbolVec) {
		for (std::pair<int, int> s : symbolVec) {
			if (s.first == Decoration::Gap || s.first == Decoration::Start || s.first == Decoration::Exit) symbols[s.first].push_back(s);
			else if (s.first & Decoration::Dot) symbols[Decoration::Dot].push_back(s);
			else if (s.first == Decoration::NewSymbols) symbols[Decoration::NewSymbols].push_back(s);
			else if (s.first == Decoration::NewSymbolsF) symbols[Decoration::NewSymbolsF].push_back(s);
			else if (s.first == Decoration::NewSymbolsE) symbols[Decoration::NewSymbolsE].push_back(s);
			else if (s.first == Decoration::NewSymbolsD) symbols[Decoration::NewSymbolsD].push_back(s);
			else if (s.first == Decoration::NewSymbolsC) symbols[Decoration::NewSymbolsC].push_back(s);
			else if (s.first == Decoration::NewSymbolsB) symbols[Decoration::NewSymbolsB].push_back(s);
			else if (s.first == Decoration::NewSymbolsA) symbols[Decoration::NewSymbolsA].push_back(s);
			else if (s.first == Decoration::NewSymbols9) symbols[Decoration::NewSymbols9].push_back(s);
			else if (s.first == Decoration::NewSymbols8) symbols[Decoration::NewSymbols8].push_back(s);
			else if (s.first == Decoration::NewSymbols7) symbols[Decoration::NewSymbols7].push_back(s);
			else if (s.first == Decoration::NewSymbols6) symbols[Decoration::NewSymbols6].push_back(s);
			else if (s.first == Decoration::NewSymbols5_B) symbols[Decoration::NewSymbols5_B].push_back(s);
			else if (s.first == Decoration::NewSymbols5_A) symbols[Decoration::NewSymbols5_A].push_back(s);
			else if (s.first == Decoration::NewSymbols5_9) symbols[Decoration::NewSymbols5_9].push_back(s);
			else if (s.first == Decoration::NewSymbols5_8) symbols[Decoration::NewSymbols5_8].push_back(s);
			else if (s.first == Decoration::NewSymbols5_7) symbols[Decoration::NewSymbols5_7].push_back(s);
			else if (s.first == Decoration::NewSymbols5_6) symbols[Decoration::NewSymbols5_6].push_back(s);
			else if (s.first == Decoration::NewSymbols5_5) symbols[Decoration::NewSymbols5_5].push_back(s);
			else if (s.first == Decoration::NewSymbols5_4) symbols[Decoration::NewSymbols5_4].push_back(s);
			else if (s.first == Decoration::NewSymbols5_3) symbols[Decoration::NewSymbols5_3].push_back(s);
			else if (s.first == Decoration::NewSymbols5_2) symbols[Decoration::NewSymbols5_2].push_back(s);
			else if (s.first == Decoration::NewSymbols5_1) symbols[Decoration::NewSymbols5_1].push_back(s);
			else if (s.first == Decoration::NewSymbols5) symbols[Decoration::NewSymbols5].push_back(s);
			else if (s.first == Decoration::NewSymbols4) symbols[Decoration::NewSymbols4].push_back(s);
			else if (s.first == Decoration::NewSymbols3) symbols[Decoration::NewSymbols3].push_back(s);
			else if (s.first == Decoration::NewSymbols2) symbols[Decoration::NewSymbols2].push_back(s);
			else symbols[s.first & 0x700].push_back(s);
		}
		style = 0;
		if (any(Decoration::Dot)) style |= Panel::Style::HAS_DOTS;
		if (any(Decoration::Stone)) style |= Panel::Style::HAS_STONES;
		if (any(Decoration::Star)) style |= Panel::Style::HAS_STARS;
		if (any(Decoration::Poly)) style |= Panel::Style::HAS_SHAPERS;
		if (any(Decoration::Triangle)) style |= Panel::Style::HAS_TRIANGLES;
		if (any(Decoration::Arrow)) style |= Panel::Style::HAS_TRIANGLES;
	}
};

