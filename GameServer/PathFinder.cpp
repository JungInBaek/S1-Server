#include "pch.h"
#include "fstream"
#include "sstream"
#include "tuple"
#include "S1Node.h"
#include "PathFinder.h"


Vector<S1Vector> PathFinder::AStar(const S1Vector& start, const S1Vector& goal/*, Map<S1Vector, Vector<S1Vector>>& edges*/)
{
    if (!IsRead)
    {
        cout << "맵 정보 파일 안읽음" << endl;
        return {};
    }

    PriorityQueue<S1NodeRef, Vector<S1NodeRef>, greater<S1NodeRef>> open;
    HashSet<S1Vector> closed;

    // 맨해튼 거리 계산
    S1NodeRef startNode = ObjectPool<S1Node>::MakeShared(start, 0, abs(goal.x - start.x) + abs(goal.y - start.y) + abs(goal.z - start.z));
    // 유클리드 거리 계산
    //S1Node startNode(start, 0, abs(goal.x - start.x) + abs(goal.y - start.y) + abs(goal.z - start.z), nullptr);

    open.push(startNode);

    cout << "A* 실행" << endl;

    while (!open.empty())
    {
        S1NodeRef current = open.top();
        open.pop();

        if (current->position == goal)
        {
            // 경로 역추적
            Vector<S1Vector> path;
            S1NodeRef pathNode = current;

            while (pathNode != nullptr)
            {
                path.push_back(pathNode->position);
                pathNode = pathNode->parent;
            }

            reverse(path.begin(), path.end());
            cout << "A* 성공" << endl;
            return path;
        }

        closed.insert(current->position);

        for (const S1Vector& neighbor : edgeMap[current->position])
        {
            if (closed.find(neighbor) != closed.end())
            {
                continue;
            }

            /*float tentativeG = current.GCost + ManhattenDistance(current.position, neighbor);

            if (allNodes.find(neighbor) == allNodes.end())
            {
                S1Node* neighborNode = new S1Node(neighbor, tentativeG, ManhattanDistance(neighbor, goal), new S1Node(current));
                open.push(*neighborNode);
                allNodes[neighbor] = neighborNode;
            }
            else
            {
                S1Node* neighborNode = allNodes[neighbor];
                if (tentativeG < neighborNode->GCost)
                {
                    neighborNode->GCost = tentativeG;
                    neighborNode->parent = new S1Node(current);
                    open.push(*neighborNode);
                }
            }*/

            float tempG = current->GCost + abs(neighbor.x - current->position.x) + abs(neighbor.y - current->position.y) + abs(neighbor.z - current->position.z);
            //float tempG = current.GCost + sqrt(pow(neighbor.x - current.position.x, 2) + pow(neighbor.y - current.position.y, 2) + pow(neighbor.z - current.position.z, 2));
            float heuristic = abs(goal.x - neighbor.x) + abs(goal.y - neighbor.y) + abs(goal.z - neighbor.z);
            //float heuristic = sqrt(pow(goal.x - neighbor.x, 2) + pow(goal.y - neighbor.y, 2) + pow(goal.z - neighbor.z, 2));

            S1NodeRef neighborNode = ObjectPool<S1Node>::MakeShared(neighbor, tempG, heuristic);
            neighborNode->parent = current;
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
            edgeMap[currentKey];
        }
        else if (line.substr(0, 6) == "Value:")
        {
            S1Vector value = parseToS1Vector(line.substr(6));
            edgeMap[currentKey].push_back(value);
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
