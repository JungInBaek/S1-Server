#include "pch.h"
#include "fstream"
#include "sstream"
#include "tuple"
#include "S1Node.h"
#include "PathFinder.h"


PathFinder GPathFinder;

Vector<S1Vector> PathFinder::AStar(S1Vector& start, S1Vector& goal, Map<S1Vector, Vector<S1Vector>>& edges)
{
    if (!IsRead)
    {
        cout << "맵 정보 파일 안읽음" << endl;
        return {};
    }

    PriorityQueue<S1Node, Vector<S1Node>, greater<S1Node>> open;
    HashSet<S1Vector> closed;

    // 유클리드 거리 계산
    S1Node startNode = { start, 0, sqrt(pow(goal.x - start.x, 2.f) + pow(goal.y - start.y, 2.f) + pow(goal.z - start.z, 2.f)), nullptr };
    open.push(startNode);

    cout << "A* 실행" << endl;

    while (!open.empty())
    {
        S1Node current = open.top();
        open.pop();

        if (current.position == goal)
        {
            // 경로 역추적
            Vector<S1Vector> path;
            S1Node* pathNode = &current;

            while (pathNode != nullptr)
            {
                path.push_back(pathNode->position);
                pathNode = pathNode->parent;
            }

            reverse(path.begin(), path.end());
            cout << "A* 성공" << endl;
            return path;
        }

        closed.insert(current.position);

        for (const S1Vector& neighbor : edges[current.position])
        {
            if (closed.find(neighbor) != closed.end())
            {
                continue;
            }

            float tempG = current.GCost + sqrt(pow(neighbor.x - current.position.x, 2) + pow(neighbor.y - current.position.y, 2) + pow(neighbor.z - current.position.z, 2));
            float heuristic = sqrt(pow(goal.x - neighbor.x, 2) + pow(goal.y - neighbor.y, 2) + pow(goal.z - neighbor.z, 2));
            S1Node neighborNode = { neighbor, tempG, heuristic, new S1Node(current)};
            open.push(neighborNode);
        }
    }

    cout << "A* 실패" << endl;
    return {};
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
            EdgeMap[currentKey];
        }
        else if (line.substr(0, 6) == "Value:")
        {
            S1Vector value = parseToS1Vector(line.substr(6));
            EdgeMap[currentKey].push_back(value);
        }
    }
    IsRead = true;

    cout << "MapInfo read done!" << endl;
}

S1Vector PathFinder::parseToS1Vector(const string& str)
{
    S1Vector vector;
    sscanf_s(str.c_str(), "(%f,%f,%f)", &vector.x, &vector.y, &vector.z);
    return vector;
}
