#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

const float K_MAX_THRUST = 100.0f;


float CalculateAngleMultiplier(float nextCheckpointAngle)
{
    if(abs(nextCheckpointAngle) > 90.0f)
        return 0.0f;

    return 1.0f;
}

int main()
{

    // game loop
    while (1) {
        int x;
        int y;
        int nextCheckpointX; // x position of the next check point
        int nextCheckpointY; // y position of the next check point
        int nextCheckpointDist; // distance to the next checkpoint
        int nextCheckpointAngle; // angle between your pod orientation and the direction of the next checkpoint
        cin >> x >> y >> nextCheckpointX >> nextCheckpointY >> nextCheckpointDist >> nextCheckpointAngle; cin.ignore();
        int opponentX;
        int opponentY;
        cin >> opponentX >> opponentY; cin.ignore();

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


        // You have to output the target position
        // followed by the power (0 <= thrust <= 100)
        // i.e.: "x y thrust"

        int thrust = (int)(K_MAX_THRUST * CalculateAngleMultiplier(nextCheckpointAngle));
        cout << nextCheckpointX << " " << nextCheckpointY << " " << thrust << endl;
    }
}