#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

using namespace std;

constexpr float K_MAX_THRUST = 100.0f;
constexpr float K_POD_RADIUS = 400.0f;
constexpr float K_CHECKPOINT_RADIUS = 600.0f;
constexpr int K_MAX_CHECKPOINTS = 8;
constexpr float K_DEG_TO_RAD = M_PI / 180.0f;
constexpr float K_RAD_TO_DEG = 180.0f / M_PI;
constexpr float K_EPS = 1e-7f;
constexpr int K_SHIPCOUNT = 2;
constexpr int K_ENEMYCOUNT = 2;

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

    bool operator != (const Vec2& other) const
    {
        return (x - other.x) >= K_EPS || (y - other.y) >= K_EPS;
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

    float ToAngle() const //radians
    {
        return atan2(y, x);
    }
};

float clamp01(float x) { return std::clamp(x, 0.0f, 1.0f); }
float lerp(float a, float b, float t) { return a + (b-a) * clamp01(t); }

struct Ship
{
    // inputs
    Vec2 pos;
    Vec2 velocity;
    float angle;
    int nextCheckpointIdx;

    //helper vars
    float nextCheckpointAngle; //degrees
    Vec2 dest;

    //outputs for player ships
    Vec2 targetCoord;
    float thrust;
    bool doShield;
    bool doBoost;
};

struct GameState
{
    Vec2 checkpoints[K_MAX_CHECKPOINTS];
    int checkpointCount;
    int lapCount;

    Ship player[K_SHIPCOUNT];
    Ship enemy[K_ENEMYCOUNT];

    bool usedBoost = false;
    int turnCount = 0;

    void ReadVec (Vec2& vec){ int x, y; cin >> x >> y; vec.x = x; vec.y = y; };

    void ReadCheckpoints()
    {
        cin >> lapCount >> checkpointCount;
        for(int i=0; i < checkpointCount; ++i)
        {
            ReadVec(checkpoints[i]);
        }
    }

    void ReadInput()
    {
        auto ReadShip = [&](Ship& ship)
        {
            ReadVec(ship.pos);
            ReadVec(ship.velocity);
            cin >> ship.angle >> ship.nextCheckpointIdx;
            ship.nextCheckpointAngle = (checkpoints[ship.nextCheckpointIdx] - ship.pos).ToAngle() * K_RAD_TO_DEG;
        };

        for(int i=0; i < K_SHIPCOUNT; ++i)
        {
            ReadShip(player[i]);
        }

        for(int i=0; i < K_ENEMYCOUNT; ++i)
        {
            ReadShip(enemy[i]);
        }
    }
};

void EvaluateTargetCoord(const GameState& gs, Ship& ship)
{
    // Don't bother going all the way to the center of the point, use a more outer point of contact
    Vec2 point = gs.checkpoints[ship.nextCheckpointIdx];
    constexpr float radius = 250.0f;
    Vec2 diff = (ship.pos - point);
    ship.dest = point + diff.Normalized() * radius;

    //set up the output target coord accounting for inertia
    ship.targetCoord = ship.dest - ship.velocity * 2.75f;
}

//outputs the desired travel direction and thrust value
void EvaluateThrust(const GameState& gs, Ship& ship)
{
    Vec2 direction = ship.dest - ship.pos;
    float dist = direction.Length();

    float thrust = K_MAX_THRUST;
    thrust *= clamp01(dist / (K_CHECKPOINT_RADIUS * 2.0f)); //slow down next to checkpoints to avoid overshooting

    //fine steering when not facing away from point
    float dot = direction.Normalized().Dot(ship.velocity.Normalized());
    if(dot > 0.0f)
    {
        thrust *= clamp01(dot);
    }

    ship.thrust = thrust;
}

// When catching a long road to the next checkpoint, why not also boost?
void EvaluateShouldBoost(const GameState& gs, Ship& ship)
{
    float distToPoint = (ship.dest - ship.pos).Length();
    ship.doBoost = !ship.doShield && !gs.usedBoost && (distToPoint > 3000) && (ship.nextCheckpointAngle <= 10.0f);
}

// When enemy gets near, shield self
void EvaluateShouldShield(const GameState& gs, Ship& ship)
{
    bool doShield = false;
    for(int i=0; i < K_ENEMYCOUNT; ++i)
    {
        bool enemyIsClose = (ship.pos - gs.enemy[i].pos).Length() <= K_POD_RADIUS * 2.1f;
        bool impactAngleIsBad = ship.velocity.Normalized().Dot(gs.enemy[i].velocity.Normalized()) <= 0.25f;
        if(enemyIsClose && impactAngleIsBad)
        {
            doShield = true;
            break;
        }
    }
    
    ship.doShield = doShield;
}

void WriteOutput(const Ship& ship)
{
    cout << (int)ship.targetCoord.x << " " << (int)ship.targetCoord.y << " ";
    if(ship.doShield) cout << "SHIELD";
    else if(ship.doBoost) cout << "BOOST";
    else cout << (int)std::clamp(ship.thrust, 0.0f, K_MAX_THRUST);

    //cout << " debugmsg"; //debug messages

    cout << endl;
}

int main()
{
    GameState gs;
    gs.ReadCheckpoints();

    // game loop
    while (1) 
    {
        gs.ReadInput();

        for(int i=0; i < K_SHIPCOUNT; ++i)
        {
            EvaluateTargetCoord(gs, gs.player[i]);
            EvaluateThrust(gs, gs.player[i]);
            EvaluateShouldBoost(gs, gs.player[i]);
            EvaluateShouldShield(gs, gs.player[i]);

            WriteOutput(gs.player[i]);
        }

        gs.turnCount++;
    }
}