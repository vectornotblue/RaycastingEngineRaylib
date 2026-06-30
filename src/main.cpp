#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <cmath>

constexpr int rows = 24;
constexpr int cols = 24;
bool MinimapMode = true;
constexpr int screenWidth = 1080;
constexpr int screenHeight = 1080;
constexpr int cellSize = screenHeight/rows;
constexpr int wallSize  = cellSize;
constexpr int FPS = 30; 
constexpr int miniFPS = 15;
constexpr float miniScale = 0.25f;
constexpr float miniMapWidth = (float)screenWidth*miniScale;
constexpr float miniMapHeight = (float)screenHeight*miniScale;
constexpr float miniMapX = screenWidth - miniMapWidth - 20.0f;
constexpr float miniMapY = 20.0f;
constexpr float FOV = 60 * DEG2RAD;
float pPlane = (screenWidth/2.0f)/tanf(FOV/2.0f);
constexpr int RES = 7;
constexpr int rayCount = screenWidth/RES;
constexpr int maxRaySteps = 100;
float NormalizeAngle(float angle){
    angle = fmod(angle,2*PI);
    if(angle<0) angle+= 2*PI;
    return angle;
}



struct Vector2Int{
    int x, y;
};


Color bgColor = {30,30,100,250};

//grid
constexpr int grid[rows][cols] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,1,0,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,1,1,1,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,1,1,1,0,1},
    {1,0,1,1,1,1,1,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,1,1,1,1,1,0,1,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1},
    {1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};
enum gridValues{
    Empty = 0,
    Wall = 1
};
bool GridHasValueAtPos(int value, Vector2Int pos /*in rowcol*/){
    if( pos.x >= 0 && pos.y >= 0 &&
        pos.x <cols && pos.y < rows &&
        grid[pos.y][pos.x] == value){
        return true;
    }
    return false;
}
Color wallColor = BLUE;
Color emptyColor = BLACK;
void DrawGrid(){
    float miniCellSize = (float)cellSize * miniScale;
    //DrawRectangle(miniMapX, miniMapY, miniScale * cols, miniScale * rows, WHITE);
    for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j ++){
            Color cellColor = (grid[i][j] == 1) ? wallColor : emptyColor;
            DrawRectangle(j*miniCellSize,i*miniCellSize, miniCellSize-1, miniCellSize-1, cellColor);
        }
    }
};

//player
class Player{
    private:    
        float size = 5;
        Color color = RED;
        float turnSpeed = .003f, moveSpeed = 200.0f, shiftSpeed = 500.0f;
public:

    Vector2 pos = {screenWidth/2, screenHeight/2}, dir;
    float angle = -PI;

    void DrawPlayer(){
        float miniPosX = miniScale * pos.x;
        float miniPosY = miniScale * pos.y;
        DrawCircle(miniPosX,miniPosY, size, color);
        //DrawLineEx(pos, {pos.x+cellSize*dir.x, pos.y+dir.y*cellSize}, 3, RED);
    }
    void MovePlayer(Vector2 moveDir, float dt, bool shift){
        float currentSpeed = shift ? shiftSpeed : moveSpeed;
        float offset = 90*DEG2RAD;
        moveDir = Vector2Rotate(moveDir, angle + offset);
        pos.x += moveDir.x * currentSpeed * dt;
        //Collision Check
        if(GridHasValueAtPos(Wall, {int(pos.x/cellSize), int(pos.y/cellSize)})){
            pos.x -= moveDir.x * currentSpeed * dt;
        }
        pos.y += moveDir.y * currentSpeed * dt;
        if(GridHasValueAtPos(Wall, {int(pos.x/cellSize), int(pos.y/cellSize)})){
            pos.y -= moveDir.y * currentSpeed * dt;
        }
    }
    void RotatePlayer(float mouseD){
        float angleChange = mouseD * turnSpeed;
        angle += angleChange;
        angle = NormalizeAngle(angle);
    }
};

//rays
Color rayColor = WHITE;
struct Ray2D{
    Vector2 pos; //pixels
    Vector2Int gridPos; //rows or cols
    Vector2 dir;
    float length = 100, angle;
    Vector2Int step;
    int currentStep;
    bool horWallHit = false, verWallHit = false;
    void Draw(const Player& player){
        float miniPosX = player.pos.x * miniScale;
        float miniPosY = player.pos.y * miniScale;
        float miniLength = length * miniScale;
        DrawLineEx({miniPosX, miniPosY}, {miniPosX+dir.x*miniLength, miniPosY+dir.y*miniLength}, 1, rayColor);
    }
    void DrawBar(const Player& player, int i){
        float lengthPerp = length * cosf(angle-player.angle);
        float xpos = i*RES;
        float BarSize = (wallSize/lengthPerp) * pPlane;
        unsigned char barColorValue = (Clamp(2*BarSize/screenHeight, 0.0001f, 1.0f))*255;
        Color barColor = {0, 0, barColorValue, 255};
        DrawRectangle(xpos, screenHeight/2.0f - 0.5*BarSize, RES, BarSize, barColor);
        //DrawLineEx({xpos, screenHeight/2.0f - halfBarSize}, {xpos, screenHeight/2.0f + halfBarSize}, RES, barColor);
    }
    void FindStep(){
        step.x = (dir.x>=0) ? 1 : -1;
        step.y = (dir.y>=0) ? 1 : -1;
    }

    void Cast(){
        float XDist, YDist, deltaXDist, deltaYDist; // firstDist for distance to first point, deltaDist between all others

        gridPos = {int(pos.x/cellSize), int(pos.y/cellSize)};
        FindStep();
        deltaXDist = (dir.x == 0.0f) ? INFINITY : fabsf(1.0f/dir.x);
        deltaYDist = (dir.y == 0.0f) ? INFINITY : fabsf(1.0f/dir.y);
        XDist = (gridPos.x-pos.x/cellSize) * step.x + .5f + step.x/2.0f;
        XDist *= deltaXDist;
        YDist = (gridPos.y-pos.y/cellSize) * step.y + .5f + step.y/2.0f;
        YDist *= deltaYDist;
        //DDA
        horWallHit = verWallHit = false;
        currentStep = 0;
        while(!verWallHit && !horWallHit && currentStep < maxRaySteps){
            if(XDist < YDist){
                XDist += deltaXDist;
                gridPos.x += step.x;
                verWallHit = GridHasValueAtPos(Wall, {gridPos.x, gridPos.y});
            } else{
                YDist += deltaYDist;
                gridPos.y += step.y;
                horWallHit = GridHasValueAtPos(Wall, {gridPos.x, gridPos.y});
            }
            currentStep ++;
        }
        if (verWallHit){
            length = (XDist-deltaXDist) * cellSize;
        } else if(horWallHit){
            length = (YDist-deltaYDist) *cellSize;
        }
    }
    
};
std::vector<Ray2D> rays(rayCount);
float firstRayOffset = 0.5f*(-FOV);
const float addRayAngle = FOV * 1.0f / rayCount;
void RayCast(const Player& player){
    float currentRayAngle = firstRayOffset + player.angle;
    float currentCos = cosf(currentRayAngle);
    float currentSin = sinf(currentRayAngle);
    const float deltaCos = cosf(addRayAngle);
    const float deltaSin = sinf(addRayAngle);
    for(Ray2D& ray : rays){
        ray.pos = player.pos;
        ray.angle = currentRayAngle;
        ray.dir = {currentCos, currentSin};
        float tempCos = currentCos * deltaCos - currentSin * deltaSin;
        float tempSin = currentCos * deltaSin + currentSin * deltaCos;
        currentCos = tempCos;
        currentSin = tempSin;
        ray.Cast();
        currentRayAngle += addRayAngle;
    }
    
}
void DrawRays(const Player& player){
    constexpr int minRayCount = 20;
    constexpr int miniStep = rayCount/minRayCount ;
    for(int i = 0; i < rayCount; i+= miniStep){
        rays[i].Draw(player);
    }
}
void DrawRayBars(const Player& player){
    for(int i = 0; i < rayCount; i++){
        rays[i].DrawBar(player, i);
    }
}




int main() 
{   
    InitWindow(screenWidth, screenHeight, "Raylib Raycast Engine");
    RenderTexture2D minimapTexture = LoadRenderTexture(miniMapWidth, miniMapHeight);
    Player player;
    HideCursor();
    SetTargetFPS(FPS);
    int frame = 0;
    while (!WindowShouldClose())
    {
         //reset frame sometimes
        if(IsKeyPressed(KEY_ESCAPE)){
            CloseWindow();
        }
        if(IsKeyPressed(KEY_M)){
            MinimapMode = !MinimapMode;
        }
        float deltaTime = GetFrameTime(); 
        Vector2 middle = { screenWidth*0.5f, screenHeight*0.5f};
        Vector2 mousePos = GetMousePosition();
        Vector2 mouseDelta = Vector2Subtract(mousePos, middle);
        player.RotatePlayer(mouseDelta.x);
        SetMousePosition((int)middle.x, (int)middle.y); 
        Vector2 moveDir;   
        moveDir.x = (((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) - (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))));
        moveDir.y = (((IsKeyDown(KEY_DOWN)|| IsKeyDown(KEY_S)) - (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))));
        moveDir = Vector2Normalize(moveDir);
        IsKeyDown(KEY_LEFT_SHIFT) ? player.MovePlayer(moveDir, deltaTime, true) : player.MovePlayer(moveDir, deltaTime, false);
        RayCast(player);
        if(MinimapMode && (frame % FPS/miniFPS) == 0){
            BeginTextureMode(minimapTexture);
            ClearBackground(BLACK);
            DrawGrid();
            player.DrawPlayer();
            DrawRays(player);
            EndTextureMode();
            frame = 0;
        };
        frame ++;
        BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleGradientV(0,0, screenWidth, screenHeight/2.0f, bgColor, BLACK);
            DrawRectangleGradientV(0,screenHeight/2.0f,screenWidth, screenHeight/2.0f, BLACK, bgColor);
            DrawRayBars(player);
            DrawTextureRec(minimapTexture.texture, Rectangle{0, 0, miniMapWidth, -miniMapHeight}, {miniMapX, miniMapY}, WHITE);
            //Vector2Int mousePos = {GetMousePosition().x/cellSize, GetMousePosition().y/cellSize};
            //DrawText(GridHasValueAtPos(Wall, mousePos) ? "true" : "false", 10, 10, 20, RED);
            int fps = GetFPS();
            DrawText(TextFormat("FPS:%d", fps),10,10,10, (fps==FPS) ? GREEN : RED);
        EndDrawing();

    }
    
    CloseWindow();
}