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
constexpr int K_PLAYERCOUNT = 2;
constexpr int K_ENEMYCOUNT = 2;
constexpr int K_TOTAL_SHIPCOUNT = K_PLAYERCOUNT + K_ENEMYCOUNT;

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

enum ShipFlags : int
{
    IsPlayer = 1 << 0,
};

struct Ship
{
    // inputs
    Vec2 pos;
    Vec2 velocity;
    float angle;
    int nextCheckpointIdx;

    //helper vars
    int id;
    int flags;
    float nextCheckpointAngle; //degrees
    Vec2 dest;

    //outputs
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

    Ship ships[K_TOTAL_SHIPCOUNT];

    bool usedBoost = false;
    int turnCount = 0;
    int optimalBoostIdx = 0;

    void ReadVec (Vec2& vec){ int x, y; cin >> x >> y; vec.x = x; vec.y = y; };

    void Initialize()
    {
        cin >> lapCount >> checkpointCount;
        for(int i=0; i < checkpointCount; ++i)
        {
            ReadVec(checkpoints[i]);
        }

        float bestDist = -1.0f;
        for(int i=0; i < checkpointCount; ++i)
        {
            Vec2 next = checkpoints[(i+1)%checkpointCount];
            float dist = (next - checkpoints[i]).Length();
            if(dist > bestDist)
            {
                bestDist = dist;
                optimalBoostIdx = (i+1)%checkpointCount;
            }
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

        for(int i=0; i < K_TOTAL_SHIPCOUNT; ++i)
        {
            ReadShip(ships[i]);
            ships[i].flags |= (i < K_PLAYERCOUNT) ? ShipFlags::IsPlayer : 0;
            ships[i].id = i;
        }
    }

    Ship& Player(int idx) { return ships[idx]; }
    Ship& Enemy(int idx) { return ships[idx + K_PLAYERCOUNT]; }
};

void EvaluateTargetCoord(GameState& gs, Ship& ship)
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
void EvaluateThrust(GameState& gs, Ship& ship)
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
void EvaluateShouldBoost(GameState& gs, Ship& ship)
{
    Vec2 direction = ship.dest - ship.pos;
    float dist = direction.Length();
    float dot = direction.Normalized().Dot(ship.velocity.Normalized());
    ship.doBoost = !ship.doShield && !gs.usedBoost && (ship.nextCheckpointIdx == gs.optimalBoostIdx) && (dot >= 0.9f);
}

// When enemy gets near, shield self
void EvaluateShouldShield(GameState& gs, Ship& ship)
{
    constexpr float impactDistTolerance = K_POD_RADIUS * 2.0f;
    bool doShield = false;
    for(int i=0; i < K_TOTAL_SHIPCOUNT; ++i)
    {
        Ship& other = gs.ships[i];
        if(other.id == ship.id) continue;
        if((other.flags & ShipFlags::IsPlayer) == (ship.flags & ShipFlags::IsPlayer)) continue; //maybe not needed

        bool willImpact = ((ship.pos + ship.velocity) - (other.pos + other.velocity)).Length() <= impactDistTolerance;
        bool impactAngleIsBad = ship.velocity.Normalized().Dot(other.velocity.Normalized()) <= 0.25f;
        if(willImpact && impactAngleIsBad)
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
    gs.Initialize();

    // game loop
    while (1) 
    {
        gs.ReadInput();

        for(int i=0; i < K_PLAYERCOUNT; ++i)
        {
            EvaluateTargetCoord(gs, gs.Player(i));
            EvaluateThrust(gs, gs.Player(i));
            EvaluateShouldBoost(gs, gs.Player(i));
            EvaluateShouldShield(gs, gs.Player(i));

            WriteOutput(gs.Player(i));
        }

        gs.turnCount++;
    }
}