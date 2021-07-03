#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

const float K_MAX_THRUST = 100.0f;

struct Vec2
{
    float x,y;

    Vec2 operator- (const Vec2& other)
    {
        return {x-other.x, y-other.y};
    }

    float Dot(const Vec2& other)
    {
        return x * other.x + y * other.y;
    }

    float Length()
    {
        return sqrt(x * x + y * y);
    }
};

struct GameInput
{
    Vec2 posPod;
    Vec2 posCheckpoint;
    int nextCheckpointDist; // distance to the next checkpoint
    int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
    Vec2 posEnemy;

    void Read()
    {
        auto ReadVec = [](Vec2& vec){ int x, y; cin >> x >> y; vec.x = x; vec.y = y; };

        ReadVec(posPod);
        ReadVec(posCheckpoint);
        cin >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        ReadVec(posEnemy); cin.ignore();
    }
};

struct GameState
{
    bool usedBoost = false;
};

float CalculateAngleMultiplier(const GameInput& gi)
{
    if(abs(gi.nextCheckpointAngle) > 90.0f)
        return 0.0f;

    return 1.0f;
}

bool ShouldBoost(const GameInput& gi, const GameState& gs, float thrust)
{
    float distToPoint = gi.nextCheckpointDist;
    return !gs.usedBoost && (gi.nextCheckpointDist > 3000) && (thrust >= (int)(K_MAX_THRUST) * 0.95f);
}

int main()
{
    GameInput gi;
    GameState gs;

    // game loop
    while (1) {
        gi.Read();
        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


        // You have to output the target position
        // followed by the power (0 <= thrust <= 100)
        // i.e.: "x y thrust"

        int thrust = (int)(K_MAX_THRUST * CalculateAngleMultiplier(gi));

        cout << (int)gi.posCheckpoint.x << " " << (int)gi.posCheckpoint.y << " ";
        
        if(ShouldBoost(gi, gs, thrust))
        {
            cout << "BOOST" << endl;
            gs.usedBoost = true;
        }
        else
        {
            cout << thrust << endl;
        }
        // << thrust << endl;
    }
}