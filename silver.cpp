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

constexpr float K_MAX_THRUST = 100.0f;
constexpr float K_POD_RADIUS = 400.0f;
constexpr float K_DEG_TO_RAD = M_PI / 180.0f;

//2d math helper
struct Vec2
{
    float x,y;

    Vec2 operator+ (const Vec2& other) const
    {
        return {x+other.x, y+other.y};
    }

    Vec2 operator- (const Vec2& other) const
    {
        return {x-other.x, y-other.y};
    }

    Vec2 operator* (const float m) const
    {
        return {x * m, y * m};
    }

    float Dot(const Vec2& other) const
    {
        return x * other.x + y * other.y;
    }

    float Length() const
    {
        return sqrt(x * x + y * y);
    }

    Vec2 Normalized() const
    {
        float l = Length();
        return {x/l , y/l};
    }

    Vec2 Rotate(float rad) const
    {
        float c = cos(rad);
        float s = sin(rad);
        return {c * x - s * y, s * x + c * y};
    }
};

struct GameState
{
    Vec2 posPod; // position of pod
    Vec2 forwardPod; // forward direction of pod
    Vec2 posCheckpoint; // position of checkpoint
    int nextCheckpointDist; // distance to the next checkpoint
    int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
    Vec2 posEnemy; // position of enemy

    bool usedBoost = false;
    Vec2 dest; // target point to fly towards

    int turnCount = 0;

    void ReadInput()
    {
        auto ReadVec = [](Vec2& vec){ int x, y; cin >> x >> y; vec.x = x; vec.y = y; };

        ReadVec(posPod);
        ReadVec(posCheckpoint);
        cin >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        ReadVec(posEnemy); cin.ignore();

        forwardPod = (posCheckpoint - posPod).Normalized().Rotate(nextCheckpointAngle * K_DEG_TO_RAD).Normalized();
    }
};

// Don't push away from the target if we overshot it
float CalculateAngleMultiplier(const GameState& gs)
{
    Vec2 diff = gs.dest - gs.posPod;
    return (gs.forwardPod.Dot(diff.Normalized()) > 0.0f) ? 1.0f : 0.0f;
}

// Slow down when getting close to target to avoid huge overshotting
float CalculateDistanceMultiplier(const GameState& gs)
{
    Vec2 diff = gs.dest - gs.posPod;
    constexpr float threshold = 1500.0f;
    return std::clamp(diff.Length() / threshold, 0.5f, 1.0f);
}

// When catching a long road to the next checkpoint, why not also boost?
bool ShouldBoost(const GameState& gs, float thrust)
{
    float distToPoint = gs.nextCheckpointDist;
    return !gs.usedBoost && (gs.nextCheckpointDist > 3000) && (thrust >= K_MAX_THRUST * 0.95f);
}

// When enemy gets near, shield self
bool ShouldShield(const GameState& gs)
{
    constexpr float threshold = 2.2f;
    return (gs.posPod - gs.posEnemy).Length() <= K_POD_RADIUS * threshold;
}

// Don't bother going all the way to the center of the point, use a more outer point of contact
Vec2 FindBestDestInCheckpoint(const GameState& gs)
{
    constexpr float radius = 250.0f;
    Vec2 diff = (gs.posPod - gs.posCheckpoint);
    return gs.posCheckpoint + diff.Normalized() * radius;
}

int main()
{
    GameState gs;

    // game loop
    while (1) {
        gs.ReadInput();

        gs.dest = FindBestDestInCheckpoint(gs);
        int thrust = (int)(K_MAX_THRUST * CalculateAngleMultiplier(gs) * CalculateDistanceMultiplier(gs));

        cout << (int)gs.dest.x << " " << (int)gs.dest.y << " ";
        
        if(ShouldShield(gs))
        {
            cout << "SHIELD" << endl;
        }
        else if(ShouldBoost(gs, thrust))
        {
            cout << "BOOST" << endl;
            gs.usedBoost = true;
        }
        else
        {
            cout << thrust << endl;
        }
        // << thrust << endl;

        gs.turnCount++;
    }
}