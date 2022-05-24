// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "Watchdog.h"
#include "Quaternion.h"
#include <thread>
#include <iostream>
#include <stdlib.h>

void Watchdog::start()
{
	std::thread{ &Watchdog::run, this }.detach();
}

void Watchdog::run()
{
	while (!terminate) {
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepTime * 1000)));
		action();
	}
}

//Keep Watchdog - Keep the big panel off until all panels are solved

void KeepWatchdog::action() {
	if (ReadPanelData<int>(0x01BE9, SOLVED)) {
		WritePanelData<float>(0x03317, POWER, { 1, 1 });
		WritePanelData<int>(0x03317, NEEDS_REDRAW, { 1 });
		terminate = true;
	}
	else {
		WritePanelData<float>(0x03317, POWER, { 0, 0 });
		WritePanelData<int>(0x03317, NEEDS_REDRAW, { 1 });
	}
}

//Arrow Watchdog - To run the arrow puzzles

void ArrowWatchdog::action() {
	int length = ReadPanelData<int>(id, TRACED_EDGES);
	if (length != tracedLength) {
		complete = false;
	}
	if (length == 0 || complete) {
		sleepTime = 0.1f;
		return;
	}
	sleepTime = 0.01f;
	if (length == tracedLength) return;
	initPath();
	if (complete) {
		for (int x = 1; x < width; x++) {
			for (int y = 1; y < height; y++) {
				if (!checkArrow(x, y)) {
					OutputDebugStringW(L"No");
					WritePanelData<int>(id, STYLE_FLAGS, { style | Panel::Style::HAS_TRIANGLES });
					return;
				}
			}
		}
		OutputDebugStringW(L"Yes");
		WritePanelData<int>(id, STYLE_FLAGS, { style & ~Panel::Style::HAS_TRIANGLES });
	}
}

void ArrowWatchdog::initPath()
{
	int numTraced = ReadPanelData<int>(id, TRACED_EDGES);
	int tracedptr = ReadPanelData<int>(id, TRACED_EDGE_DATA);
	if (!tracedptr) return;
	std::vector<SolutionPoint> traced = ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, numTraced);
	if (style & Panel::Style::SYMMETRICAL) {
		for (int i = 0; i < numTraced; i++) {
			SolutionPoint sp;
			if (traced[i].pointA >= exitPoint || traced[i].pointB >= exitPoint) {
				sp.pointA = sp.pointB = exitPoint;
			}
			else {
				sp.pointA = (width / 2 + 1) * (height / 2 + 1) - 1 - traced[i].pointA;
				sp.pointB = (width / 2 + 1) * (height / 2 + 1) - 1 - traced[i].pointB;
			}
			traced.push_back(sp);
		}
	}
	grid = backupGrid;
	tracedLength = numTraced;
	complete = false;
	if (traced.size() == 0) return;
	for (const SolutionPoint& p : traced) {
		int p1 = p.pointA, p2 = p.pointB;
		if (p1 == exitPoint || p2 == exitPoint) {
			complete = true;
			continue;
		}
		else if (p1 > exitPoint || p2 > exitPoint) continue;
		if (p1 == 0 && p2 == 0 || p1 < 0 || p2 < 0) {
			return;
		}
		int x1 = (p1 % (width / 2 + 1)) * 2, y1 = height - 1 - (p1 / (width / 2 + 1)) * 2;
		int x2 = (p2 % (width / 2 + 1)) * 2, y2 = height - 1 - (p2 / (width / 2 + 1)) * 2;
		if (pillarWidth > 0) {
			x1 = (p1 % (width / 2)) * 2, y1 = height - 1 - (p1 / (width / 2)) * 2;
			x2 = (p2 % (width / 2)) * 2, y2 = height - 1 - (p2 / (width / 2)) * 2;
			grid[x1][y1] = PATH;
			grid[x2][y2] = PATH;
			if (x1 == x2 || x1 == x2 + 2 || x1 == x2 - 2) grid[(x1 + x2) / 2][(y1 + y2) / 2] = PATH;
			else grid[width - 1][(y1 + y2) / 2] = PATH;
		}
		else {
			grid[x1][y1] = PATH;
			grid[x2][y2] = PATH;
			grid[(x1 + x2) / 2][(y1 + y2) / 2] = PATH;
		}
		if (p1 == exitPos || p2 == exitPos || (style & Panel::Style::SYMMETRICAL) && (p1 == exitPosSym || p2 == exitPosSym)) {
			complete = !complete;
		}
		else complete = false;
	}
}

bool ArrowWatchdog::checkArrow(int x, int y)
{
	if (pillarWidth > 0) return checkArrowPillar(x, y);
	int symbol = grid[x][y];
	if ((symbol & 0x700) == Decoration::Triangle && (symbol & 0xf0000) != 0) {
		int count = 0;
		if (grid[x - 1][y] == PATH) count++;
		if (grid[x + 1][y] == PATH) count++;
		if (grid[x][y - 1] == PATH) count++;
		if (grid[x][y + 1] == PATH) count++;
		return count == (symbol >> 16);
	}
	if ((symbol & 0xF000700) == Decoration::NewSymbols) {
		return checkNewSymbols(x, y,symbol);
	}
	else if ((symbol & 0xF000700) == Decoration::NewSymbols2) {
		return checkNewSymbols2(x, y, symbol);
	}
	else if ((symbol & 0xF000700) == Decoration::NewSymbols3) {
		return checkNewSymbols3(x, y, symbol);
	}
	else if ((symbol & 0xF000700) == Decoration::NewSymbols4) {
		return checkNewSymbols4(x, y, symbol);
	}
	else if ((symbol & 0xF000700) == Decoration::NewSymbols5) {
		return checkNewSymbols5(x, y, symbol);
	}
	else if ((symbol & 0xF000700) == Decoration::NewSymbols6) {
		return checkNewSymbols6(x, y, symbol);
	}
	else if ((symbol & 0xF000700) == Decoration::NewSymbols7) {
		return checkNewSymbols7(x, y, symbol);
	}
	if ((symbol & 0x700) != Decoration::Arrow)
		return true;
	int targetCount = (symbol & 0xf000) >> 12;
	Point dir = DIRECTIONS[(symbol & 0xf0000) >> 16];
	x += dir.first / 2; y += dir.second / 2;
	int count = 0;
	while (x >= 0 && x < width && y >= 0 && y < height) {
		if (grid[x][y] == PATH) {
			if (++count > targetCount)
				return false;
		}
		x += dir.first; y += dir.second;
	}
	return count == targetCount;
}

bool ArrowWatchdog::checkNewSymbols(int x, int y,int symbol) {
	Point pos = Point(x, y);
	std::set<Point> region = get_region_for_watchdog(Point(x, y));
	std::set<Point> nearby = {
			pos + Point(2,0),pos + Point(0,2),pos + Point(0,-2), pos + Point(-2,0),
			pos + Point(2,2), pos + Point(-2,2), pos + Point(-2,-2), pos + Point(2,-2),
	};
	int num = 0;
	for (Point a : nearby) {
		for (Point b : region) {
			if (a.first == b.first && a.second == b.second) {
				num += 1;
			}
		}
	}
	if ((pos.first == 1 || pos.first == width - 2) && (pos.second == 1 || pos.second == height - 2))
	{
		num = 3 - num;
	}
	else if ((pos.first == 1 || pos.first == width - 2) || (pos.second == 1 || pos.second == height - 2))
	{
		num = 5 - num;
	}
	else
	{
		num = 8 - num;
	}
	return num == (0xf0000 & symbol) >> 16;
}

bool ArrowWatchdog::checkNewSymbols2(int x, int y, int symbol) {
	if ((symbol & 0xf0000) >> 16 == 9) return true;
	std::vector<Point> _8dir = { Point(0, 2), Point(0, -2), Point(2, 0), Point(-2, 0), Point(2, 2), Point(2, -2), Point(-2, -2), Point(-2, 2) };
	Point dir = _8dir[(symbol & 0xf0000) >> 16];
	OutputDebugStringW(L"ç¿ïW:(");
	DebugLog(x);
	OutputDebugStringW(L",");
	DebugLog(y);
	OutputDebugStringW(L") ï˚å¸:(");
	DebugLog(dir.first);
	OutputDebugStringW(L",");
	DebugLog(dir.second);
	OutputDebugStringW(L")\n");
	std::set<Point> rival = {};
	for (Point p : get_region_for_watchdog(Point(x, y))) {
		OutputDebugStringW(L"åÛï‚:(");
		DebugLog(p.first);
		OutputDebugStringW(L",");
		DebugLog(p.second);
		OutputDebugStringW(L") ID:");
		DebugLog(grid[p.first][p.second]);
		OutputDebugStringW(L"\n");
		if (((dir.first == 0 || (p.first - x) * dir.first >  0) && (dir.second == 0 || (p.second - y) * dir.second > 0))) {
			if (!(grid[p.first][p.second] == 0x2090000 || grid[p.first][p.second]  == 0x600 || grid[p.first][p.second] == 0x0 || grid[p.first][p.second] == 0xA00)) return false;
		}//ãÛîíîªíËÇ™ê≥ämÇ≈Ç»Ç¢
	}
	return true;
}

void ArrowWatchdog::DebugLog(int i) {
	std::string s = std::to_string(i);
	const char* mbs = s.data();
	wchar_t* wcs;
	size_t ret;
	mbstowcs_s(&ret, nullptr, 0, mbs, s.length());
	std::wstring ws(ret, 0);
	mbstowcs_s(&ret, &ws[0], ret, mbs, s.length());
	ws.resize(ret - 1);
	OutputDebugStringW(ws.data());
}

bool ArrowWatchdog::checkNewSymbols3(int x, int y, int symbol) {
	std::vector<Point> DIRECTIONS = { Point(0, 2), Point(0, -2), Point(2, 0), Point(-2, 0) };
	for (Point dir : DIRECTIONS) {
		int local_x = x + dir.first / 2;
		int local_y = y + dir.second / 2;
		bool flag = false;
		while ((local_x >= 0 && local_x < width && local_y >= 0 && local_y < height) && !flag) {
			if (grid[local_x][local_y] == PATH) flag = true;
			local_x += dir.first; local_y += dir.second;
		}
		if (!flag) return false;
	}
	
	return true;
}

bool ArrowWatchdog::checkNewSymbols4(int x, int y, int symbol) {
	Point pos = Point(x, y);
	std::set<Point> region = get_region_for_watchdog(Point(x, y));
	std::set<Point> nearby = {
			pos + Point(2,0),pos + Point(0,2),pos + Point(0,-2), pos + Point(-2,0),
			pos + Point(2,2), pos + Point(-2,2), pos + Point(-2,-2), pos + Point(2,-2),
	};
	int num = 0;
	for (Point a : nearby) {
		for (Point b : region) {
			if (a.first == b.first && a.second == b.second) {
				num += 1;
			}
		}
	}
	if ((pos.first == 1 || pos.first == width - 2) && (pos.second == 1 || pos.second == height - 2))
	{
		num = 3 - num;
	}
	else if ((pos.first == 1 || pos.first == width - 2) || (pos.second == 1 || pos.second == height - 2))
	{
		num = 5 - num;
	}
	else
	{
		num = 8 - num;
	}
	return num == (0xf0000 & symbol) >> 16;
}

bool ArrowWatchdog::checkNewSymbols5(int x, int y, int symbol) {
	Point pos = Point(x, y);
	std::set<Point> region = get_region_for_watchdog(Point(x, y));
	std::set<Point> nearby = {
			pos + Point(2,0),pos + Point(0,2),pos + Point(0,-2), pos + Point(-2,0),
			pos + Point(2,2), pos + Point(-2,2), pos + Point(-2,-2), pos + Point(2,-2),
	};
	int num = 0;
	for (Point a : nearby) {
		for (Point b : region) {
			if (a.first == b.first && a.second == b.second) {
				num += 1;
			}
		}
	}
	if ((pos.first == 1 || pos.first == width - 2) && (pos.second == 1 || pos.second == height - 2))
	{
		num = 3 - num;
	}
	else if ((pos.first == 1 || pos.first == width - 2) || (pos.second == 1 || pos.second == height - 2))
	{
		num = 5 - num;
	}
	else
	{
		num = 8 - num;
	}
	return num == (0xf0000 & symbol) >> 16;
}

bool ArrowWatchdog::checkNewSymbols6(int x, int y, int symbol) {
	Point pos = Point(x, y);
	std::set<Point> region = get_region_for_watchdog(Point(x, y));
	std::set<Point> nearby = {
			pos + Point(2,0),pos + Point(0,2),pos + Point(0,-2), pos + Point(-2,0),
			pos + Point(2,2), pos + Point(-2,2), pos + Point(-2,-2), pos + Point(2,-2),
	};
	int num = 0;
	for (Point a : nearby) {
		for (Point b : region) {
			if (a.first == b.first && a.second == b.second) {
				num += 1;
			}
		}
	}
	if ((pos.first == 1 || pos.first == width - 2) && (pos.second == 1 || pos.second == height - 2))
	{
		num = 3 - num;
	}
	else if ((pos.first == 1 || pos.first == width - 2) || (pos.second == 1 || pos.second == height - 2))
	{
		num = 5 - num;
	}
	else
	{
		num = 8 - num;
	}
	return num == (0xf0000 & symbol) >> 16;
}

bool ArrowWatchdog::checkNewSymbols7(int x, int y, int symbol) {
	Point pos = Point(x, y);
	std::set<Point> region = get_region_for_watchdog(Point(x, y));
	std::set<Point> nearby = {
			pos + Point(2,0),pos + Point(0,2),pos + Point(0,-2), pos + Point(-2,0),
			pos + Point(2,2), pos + Point(-2,2), pos + Point(-2,-2), pos + Point(2,-2),
	};
	int num = 0;
	for (Point a : nearby) {
		for (Point b : region) {
			if (a.first == b.first && a.second == b.second) {
				num += 1;
			}
		}
	}
	if ((pos.first == 1 || pos.first == width - 2) && (pos.second == 1 || pos.second == height - 2))
	{
		num = 3 - num;
	}
	else if ((pos.first == 1 || pos.first == width - 2) || (pos.second == 1 || pos.second == height - 2))
	{
		num = 5 - num;
	}
	else
	{
		num = 8 - num;
	}
	return num == (0xf0000 & symbol) >> 16;
}

std::set<Point> ArrowWatchdog::get_region_for_watchdog(Point pos) {
	std::set<Point> region;
	std::vector<Point> check;
	check.push_back(pos);
	region.insert(pos);
	while (check.size() > 0) {
		Point p = check[check.size() - 1];
		check.pop_back();
		for (Point dir : { Point(0, 1), Point(0, -1), Point(1, 0), Point(-1, 0) }) {
			Point p1 = p + dir;
			if ((p1.first == 0 || p1.first + 1 == width) || p1.second == 0 || p1.second + 1 == height) continue;
			if (grid[p1.first][p1.second] == PATH || grid[p1.first][p1.second] == OPEN) continue;
			Point p2 = p + dir * 2;
			if ((grid[p2.first][p2.second] & Decoration::Empty) == Decoration::Empty) continue;
			if (region.insert(p2).second) {
				check.push_back(p2);
			}
		}
	}
	return region;
}

std::set<int> ArrowWatchdog::get_symbols_in_region_for_watchdog(const std::set<Point>& region) {
	std::set<int> symbols;
	for (Point p : region) {
		if (grid[p.first][p.second]) symbols.insert(grid[p.first][p.second]);
	}
	return symbols;
}

bool ArrowWatchdog::checkArrowPillar(int x, int y)
{
	int symbol = grid[x][y];
	if ((symbol & 0x700) == Decoration::Triangle && (symbol & 0xf0000) != 0) {
		int count = 0;
		if (grid[x - 1][y] == PATH) count++;
		if (grid[x + 1][y] == PATH) count++;
		if (grid[x][y - 1] == PATH) count++;
		if (grid[x][y + 1] == PATH) count++;
		return count == (symbol >> 16);
	}
	if ((symbol & 0x700) != Decoration::Arrow)
		return true;
	int targetCount = (symbol & 0xf000) >> 12;
	Point dir = DIRECTIONS[(symbol & 0xf0000) >> 16];
	x = (x + (dir.first > 2 ? -2 : dir.first) / 2 + pillarWidth) % pillarWidth; y += dir.second / 2;
	int count = 0;
	while (y >= 0 && y < height) {
		if (grid[x][y] == PATH) {
			if (++count > targetCount) return false;
		}
		x = (x + dir.first + pillarWidth) % pillarWidth; y += dir.second;
	}
	return count == targetCount;
}

void BridgeWatchdog::action()
{
	int length1 = _memory->ReadPanelData<int>(id1, TRACED_EDGES);
	int length2 = _memory->ReadPanelData<int>(id2, TRACED_EDGES);
	if (solLength1 > 0 && length1 == 0) {
		_memory->WritePanelData<int>(id2, STYLE_FLAGS, { _memory->ReadPanelData<int>(id2, STYLE_FLAGS) | Panel::Style::HAS_DOTS });
	}
	if (solLength2 > 0 && length2 == 0) {
		_memory->WritePanelData<int>(id1, STYLE_FLAGS, { _memory->ReadPanelData<int>(id1, STYLE_FLAGS) | Panel::Style::HAS_DOTS });
	}
	if (length1 != solLength1 && length1 > 0 && !checkTouch(id2)) {
		_memory->WritePanelData<int>(id2, STYLE_FLAGS, { _memory->ReadPanelData<int>(id2, STYLE_FLAGS) & ~Panel::Style::HAS_DOTS });
	}
	if (length2 != solLength2 && length2 > 0 && !checkTouch(id1)) {
		_memory->WritePanelData<int>(id1, STYLE_FLAGS, { _memory->ReadPanelData<int>(id1, STYLE_FLAGS) & ~Panel::Style::HAS_DOTS });
	}
	solLength1 = length1;
	solLength2 = length2;
}

bool BridgeWatchdog::checkTouch(int id)
{
	int length = _memory->ReadPanelData<int>(id, TRACED_EDGES);
	if (length == 0) return false;
	int numIntersections = _memory->ReadPanelData<int>(id, NUM_DOTS);
	std::vector<int> intersectionFlags = _memory->ReadArray<int>(id, DOT_FLAGS, numIntersections);
	std::vector<SolutionPoint> edges = _memory->ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, length);
	for (const SolutionPoint& sp : edges) if (intersectionFlags[sp.pointA] == Decoration::Dot_Intersection || intersectionFlags[sp.pointB] == Decoration::Dot_Intersection) return true;
	return false;
}

void TreehouseWatchdog::action()
{
	if (ReadPanelData<int>(0x03613, SOLVED)) {
		WritePanelData<float>(0x17DAE, POWER, { 1.0f, 1.0f });
		WritePanelData<int>(0x17DAE, NEEDS_REDRAW, { 1 });
		terminate = true;
	}
}

void JungleWatchdog::action()
{
	int numTraced = ReadPanelData<int>(id, TRACED_EDGES);
	if (numTraced == tracedLength) return;
	tracedLength = numTraced;
	int tracedptr = ReadPanelData<int>(id, TRACED_EDGE_DATA);
	if (!tracedptr) return;
	std::vector<SolutionPoint> traced = ReadArray<SolutionPoint>(id, TRACED_EDGE_DATA, numTraced);
	int seqIndex = 0;
	for (const SolutionPoint& p : traced) {
		if ((sizes[p.pointA] & IntersectionFlags::DOT) == 0) continue;
		if (sizes[p.pointA] & (0x1000 << (state ? correctSeq1[seqIndex] : correctSeq2[seqIndex])))
			seqIndex++;
		else return;
		if (seqIndex >= 1) {
			WritePanelData<long>(id, DOT_SEQUENCE, { state ? ptr1 : ptr2 } );
			WritePanelData<long>(id, DOT_SEQUENCE_REFLECTION, { state ? ptr2 : ptr1 });
			WritePanelData<int>(id, DOT_SEQUENCE_LEN, { state ? (int)correctSeq1.size() : (int)correctSeq2.size() });
			WritePanelData<int>(id, DOT_SEQUENCE_LEN_REFLECTION, { state ? (int)correctSeq2.size() : (int)correctSeq1.size() });
			state = !state;
			return;
		}
	}
}

void TownDoorWatchdog::action()
{
	if (ReadPanelData<Quaternion>(0x03BB0, ORIENTATION).z > 0) {
		WritePanelData<float>(0x28A69, POWER, { 1.0f, 1.0f });
		WritePanelData<int>(0x28A69, NEEDS_REDRAW, { 1 });
		terminate = true;
	}
}
