#include "pch.h"
#include "fstream"
#include "sstream"
#include "tuple"
#include "S1Node.h"
#include "PathFinder.h"


Vector<S1Vector> PathFinder::AStar(const S1Vector& start, const S1Vector& goal)
{
    if (!IsRead)
    {
        cout << "맵 정보 파일 안읽음" << endl;
        return {};
    }

    if (nodeMap[goal] != 1)
    {
        cout << "갈 수 없는 위치입니다" << endl;
        return {};
    }

    enum
    {
        DIR_COUNT = 8
    };

	Vector<S1Vector> directions =
	{
		{ 1, 0, 0 },
		{ 0, 1, 0 },
		{ 0, -1, 0 },
		{ -1, 0, 0 },
		{ 1, 1, 0 },
		{ -1, 1, 0 },
		{ -1, -1, 0 },
		{ 1, -1, 0 }
	};

    Vector<int32> cost =
    {
        10,
        10,
        10,
        10,
        14,
        14,
        14,
        14,
    };

    HashSet<S1Vector> closed;
    HashMap<S1Vector, int32> best;
    HashMap<S1Vector, S1Vector> parent;
    PriorityQueue<S1NodeRef, Vector<S1NodeRef>, greater<S1NodeRef>> open;

    // 맨해튼 거리 계산
    S1NodeRef startNode;
    {
		int32 g = 0;
		int32 h = (abs(goal.x - start.x) + abs(goal.y - start.y) + abs(goal.z - start.z)) * 10;
        startNode = ObjectPool<S1Node>::MakeShared(start, g, h);
    }
    // 유클리드 거리 계산
    //S1Node startNode(start, 0, abs(goal.x - start.x) + abs(goal.y - start.y) + abs(goal.z - start.z), nullptr);

    best[start] = startNode->FCost();
    parent[start] = start;
    open.push(startNode);

    cout << "A* 실행" << endl;

    while (!open.empty())
    {
        S1NodeRef current = open.top();
        open.pop();

        // 이미 방문된 경우 스킵
		if (closed.find(current->position) != closed.end())
		{
			continue;
		}
        if (best[current->position] < current->FCost())
        {
            continue;
        }

        // 방문
        closed.insert(current->position);

        // 목적지 도착
        if (current->position == goal)
        {
            break;
        }

        for (int32 dir = 0; dir < DIR_COUNT; dir++)
        {
			S1Vector next = current->position + (directions[dir] * 100);

            // 갈 수 없으면 스킵
            if (nodeMap[next] != 1)
            {
                continue;
            }
            // 이미 방문했으면 스킵
            if (closed.find(next) != closed.end())
            {
                continue;
            }

			int32 g = current->GCost + cost[dir];
			int32 h = (abs(goal.x - next.x) + abs(goal.y - next.y) + abs(goal.z - next.z)) * 10;

            if (best.find(next) != best.end() && best[next] <= g + h)
            {
                continue;
            }

			best[next] = g + h;
			parent[next] = current->position;

			S1NodeRef nextNode = ObjectPool<S1Node>::MakeShared(next, g, h);
			open.push(nextNode);
        }
    }

    // 경로 역추적
    Vector<S1Vector> path;

    S1Vector pos = goal;
    while (true)
    {
        path.push_back(pos);
        
		if (pos == parent[pos])
		{
			break;
		}

        pos = parent[pos];
    }

    reverse(path.begin(), path.end());
    cout << "A* 성공" << endl;
    return path;
}

void PathFinder::ReadFile()
{
    ifstream file("./MapInfo.txt");
    string line;
    S1Vector currentKey;

    if (!file.is_open())
    {
        cout << "파일을 찾을 수 없음!" << endl;
        return;
    }

    while (getline(file, line))
    {
        line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line.substr(0, 4) == "Key:")
        {
            currentKey = parseToS1Vector(line.substr(4));
            nodeMap[currentKey];
        }
        else if (line.substr(0, 6) == "Value:")
        {
            int32 value = parseToInt32(line.substr(6));
            nodeMap[currentKey] = value;
        }
    }
    IsRead = true;

    cout << "MapInfo read done!" << endl;
}

float PathFinder::GetApproximationValue(int value)
{
    int min = value / 100 * 100;
    int max = min + 100;

    int temp = abs(abs(min) - abs(value));
    if (temp > abs(abs(max) - abs(value)))
    {
        return max;
    }
    return min;
}

S1Vector PathFinder::parseToS1Vector(const string& str)
{
    S1Vector vector;
    sscanf_s(str.c_str(), "(%f,%f,%f)", &vector.x, &vector.y, &vector.z);
    return vector;
}

int32 PathFinder::parseToInt32(const string& str)
{
    int32 value;
	sscanf_s(str.c_str(), "%d", &value);
    return value;
}
