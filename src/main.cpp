#include <raylib.h>
#include <raymath.h>
#include <vector>
#include <cmath>

#define wallSize cellSize
#define rows 24
#define cols 24
#define worldX cols*cellSize
#define worldY rows*cellSize
bool MinimapMode = true;

int screenWidth = 1080;
int screenHeight = 1080;
int cellSize = screenHeight/rows;
constexpr int FPS = 30; 
constexpr int miniFPS = 15;
float miniScale = 0.25f;
float miniMapWidth = (float)screenWidth*miniScale;
float miniMapHeight = (float)screenHeight*miniScale;
float miniMapX = screenWidth - miniMapWidth - 20.0f;
float miniMapY = 20.0f;
constexpr float FOV = 60 * DEG2RAD;
float pPlane = (screenWidth/2.0f)/tanf(FOV/2.0f);
constexpr int RES = 6;
int rayCount = screenWidth/RES;
constexpr int maxRaySteps = 30;
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
int grid[rows][cols] = {
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
    int gridPosX = pos.x;
    int gridPosY = pos.y;
    if(gridPosX >= cols || gridPosY >= rows){
        return false;
    }
    if(gridPosX<0 || gridPosY <0){
        return false;
    }
    if(grid[gridPosY][gridPosX] == value){
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
float turnSpeed = .003f, moveSpeed = 200.0f, shiftSpeed = 500.0f;

Vector2 playerMoveDir;
Vector2 playerPos = {screenWidth/2, screenHeight/2};
float playerAngle = -PI;
float playerRadius = 5;
Color playerColor = RED;
float miniPosX;
float miniPosY;
void DrawPlayer(){
    miniPosX = miniScale * playerPos.x;
    miniPosY = miniScale * playerPos.y;
    DrawCircle(miniPosX,miniPosY, playerRadius, playerColor);
    //DrawLineEx(pos, {pos.x+cellSize*dir.x, pos.y+dir.y*cellSize}, 3, RED);
}
void MovePlayer(float dt, bool shift){
    float currentSpeed = shift ? shiftSpeed : moveSpeed;
    float offset = 90*DEG2RAD;
    playerMoveDir = Vector2Rotate(playerMoveDir, playerAngle+ offset);
    playerPos.x += playerMoveDir.x * currentSpeed * dt;
    playerPos.y += playerMoveDir.y * currentSpeed * dt;
    if(GridHasValueAtPos(Wall, {int(playerPos.x/cellSize), int(playerPos.y/cellSize)})){
        playerPos.x -= playerMoveDir.x * currentSpeed * dt;
    playerPos.y -= playerMoveDir.y * currentSpeed * dt;
    }
}
void RotatePlayer(float mouseD){
    float angleChange = mouseD * turnSpeed;
    playerAngle += angleChange;
    playerAngle = NormalizeAngle(playerAngle);
    
}

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
    void Draw(){
        float miniLength = length * miniScale;
        DrawLineEx({miniPosX, miniPosY}, {miniPosX+dir.x*miniLength, miniPosY+dir.y*miniLength}, 1, rayColor);
    }
    void DrawBar(int i){
        float lengthPerp = length * cosf(angle-playerAngle);
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
    float XDist, YDist, deltaXDist, deltaYDist; // firstDist for distance to first point, deltaDist between all others
    void DDA(){
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
    void Cast(){
        
        gridPos = {int(pos.x/cellSize), int(pos.y/cellSize)};
        FindStep();
        deltaXDist = (dir.x == 0.0f) ? INFINITY : fabsf(1.0f/dir.x);
        deltaYDist = (dir.y == 0.0f) ? INFINITY : fabsf(1.0f/dir.y);
        XDist = (gridPos.x-pos.x/cellSize) * step.x + .5f + step.x/2.0f;
        XDist *= deltaXDist;
        YDist = (gridPos.y-pos.y/cellSize) * step.y + .5f + step.y/2.0f;
        YDist *= deltaYDist;
        DDA();
    }
    
};
std::vector<Ray2D> rays(rayCount);
float firstRayAngle = 0.5f*(-FOV);
float addRayAngle = FOV * 1.0f / rayCount;
void RayCast(){
    for(int i = 0; i < rayCount; i++){
        rays[i].pos = playerPos;
        float rayAngle = NormalizeAngle(playerAngle + firstRayAngle + addRayAngle*i); 
        rays[i].angle = rayAngle;
        rays[i].dir = {cosf(rayAngle), sinf(rayAngle)};
        rays[i].Cast();
    }
    
}
void DrawRays(){
    for(int i = 0; i < rayCount; i+= 1.0f/miniScale){
        rays[i].Draw();
    }
}
void DrawRayBars(){
    for(int i = 0; i < rayCount; i++){
        rays[i].DrawBar(i);
    }
}




int main() 
{   
    
    InitWindow(screenWidth, screenHeight, "Raylib Raycast Engine");
    RenderTexture2D minimapTexture = LoadRenderTexture(miniMapWidth, miniMapHeight);
    
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
        RotatePlayer(mouseDelta.x);
        SetMousePosition((int)middle.x, (int)middle.y);    
        playerMoveDir.x = (((IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) - (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))));
        playerMoveDir.y = (((IsKeyDown(KEY_DOWN)|| IsKeyDown(KEY_S)) - (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W))));
        IsKeyDown(KEY_LEFT_SHIFT) ? MovePlayer(deltaTime, true) : MovePlayer(deltaTime, false);
        RayCast();
        if(MinimapMode && (frame % FPS/miniFPS) == 0){
            BeginTextureMode(minimapTexture);
            ClearBackground(BLACK);
            DrawGrid();
            DrawPlayer();
            DrawRays();
            EndTextureMode();
            frame = 0;
        };
        frame ++;
        BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleGradientV(0,0, screenWidth, screenHeight/2.0f, bgColor, BLACK);
            DrawRectangleGradientV(0,screenHeight/2.0f,screenWidth, screenHeight/2.0f, BLACK, bgColor);
            DrawRayBars();
            DrawTextureRec(minimapTexture.texture, Rectangle{0, 0, miniMapWidth, -miniMapHeight}, {miniMapX, miniMapY}, WHITE);
            //Vector2Int mousePos = {GetMousePosition().x/cellSize, GetMousePosition().y/cellSize};
            //DrawText(GridHasValueAtPos(Wall, mousePos) ? "true" : "false", 10, 10, 20, RED);
            int fps = GetFPS();
            DrawText(TextFormat("FPS:%d", fps),10,10,10, (fps==FPS) ? GREEN : RED);
        EndDrawing();

    }
    
    CloseWindow();
}