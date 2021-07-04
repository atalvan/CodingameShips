#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

constexpr float K_MAX_THRUST = 100.0f;
constexpr float K_POD_RADIUS = 400.0f;
constexpr float K_CHECKPOINT_RADIUS = 600.0f;
constexpr float K_DEG_TO_RAD = M_PI / 180.0f;
constexpr float K_EPS = 1e-7f;

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

    bool operator != (const Vec2& other) const
    {
        return (x - other.x) >= K_EPS || (y - other.y) >= K_EPS;
    }
};

float clamp01(float x) { return std::clamp(x, 0.0f, 1.0f); }
float lerp(float a, float b, float t) { return a + (b-a) * clamp01(t); }

struct Ship
{
    Vec2 pos;
    Vec2 prevPos; //position last turn
    Vec2 velocity;
};

struct GameState
{
    Ship player;
    Ship enemy;
    Vec2 posCheckpoint; // position of checkpoint
    int nextCheckpointDist; // distance to the next checkpoint
    int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint

    bool usedBoost = false;
    int turnCount = 0;

    //output
    Vec2 dest; // target point to fly towards
    float thrust;

    void ReadInput()
    {
        auto ReadVec = [](Vec2& vec){ int x, y; cin >> x >> y; vec.x = x; vec.y = y; };

        player.prevPos = player.pos;
        ReadVec(player.pos);

        ReadVec(posCheckpoint);
        cin >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();

        enemy.prevPos = enemy.pos;
        ReadVec(enemy.pos); cin.ignore();

        if(turnCount > 0)
        {
            player.velocity = player.pos - player.prevPos;
            enemy.velocity = enemy.pos - enemy.prevPos;
        }
        else
        {
            player.prevPos = player.pos;
            player.velocity = (posCheckpoint - player.pos).Normalized();
            enemy.prevPos = enemy.pos;
            enemy.velocity = (posCheckpoint - enemy.pos).Normalized();
        }
    }
};

void WriteOutput(float thrust, Vec2 dest, bool boost, bool shield)
{
    cout << (int)dest.x << " " << (int)dest.y << " ";
    if(shield) cout << "SHIELD";
    else if(boost) cout << "BOOST";
    else cout << (int)std::clamp(thrust, 0.0f, K_MAX_THRUST);
    cout << endl;
}

//outputs the desired travel direction and thrust value
float CalculateThrust(const GameState& gs)
{
    Vec2 direction = gs.dest - gs.player.pos;
    float dist = direction.Length();

    float thrust = K_MAX_THRUST;
    thrust *= clamp01(dist / (K_CHECKPOINT_RADIUS * 2.0f)); //slow down next to checkpoints to avoid overshooting

    //fine steering when not facing away from point
    if(direction.Normalized().Dot(gs.player.velocity) > 0.0f) //if we end up facing away from next checkpoint
    {
        thrust *= clamp01(1.0f - abs(gs.nextCheckpointAngle) / 90.0f);
    }

    return thrust;
}

// When catching a long road to the next checkpoint, why not also boost?
bool ShouldBoost(const GameState& gs)
{
    float distToPoint = gs.nextCheckpointDist;
    return !gs.usedBoost && (gs.nextCheckpointDist > 3000) && (gs.nextCheckpointAngle <= 10.0f);
}

// When enemy gets near, shield self
bool ShouldShield(const GameState& gs)
{
    bool enemyIsClose = (gs.player.pos - gs.enemy.pos).Length() <= K_POD_RADIUS * 2.1f;
    bool impactAngleIsBad = gs.player.velocity.Normalized().Dot(gs.enemy.velocity.Normalized()) <= 0.25f;
    return enemyIsClose && impactAngleIsBad;
}

// Don't bother going all the way to the center of the point, use a more outer point of contact
Vec2 FindBestDestInCheckpoint(const GameState& gs)
{
    constexpr float radius = 250.0f;
    Vec2 diff = (gs.player.pos - gs.posCheckpoint);
    return gs.posCheckpoint + diff.Normalized() * radius;
}

int main()
{
    GameState gs;

    // game loop
    while (1) 
    {
        gs.ReadInput();

        gs.dest = FindBestDestInCheckpoint(gs);
        gs.thrust = CalculateThrust(gs);

        Vec2 targetPos = gs.player.pos + (gs.dest - gs.player.pos).Normalized() * 1000.0f;

        bool useShield = ShouldShield(gs);
        bool useBoost = !useShield && ShouldBoost(gs);
        gs.usedBoost = gs.usedBoost || useBoost;

        WriteOutput(gs.thrust, targetPos, useBoost, useShield);

        gs.turnCount++;
    }
}