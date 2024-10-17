#include <gba_console.h>
#include <gba_video.h>
#include <gba_interrupt.h>
#include <gba_systemcalls.h>
#include <gba_input.h>
#include <stdio.h>
#include <stdlib.h>

#define GRID_COLOR RGB5(10, 10, 10)  // グリッドの色（黒）
#define GRID_FIVE_COLOR RGB5(24, 24, 24)  // グリッドの色（黒）
#define BG_COLOR RGB5(31, 31, 31)       // 背景色（白）
#define HINT_COLOR RGB5(28, 28, 28)    // ヒント部分の色（薄い白）
#define SELECT_CELL_COLOR RGB5(24, 0, 0)       // 選択中セルの色（青）


// 色塗り部分の縦横のセル数（ピクロスのパズル部分）
#define GRID_CELLS_X 15   // 横方向のセル数
#define GRID_CELLS_Y 15   // 縦方向のセル数

// ヒント部分の最大数
#define MAX_HINT_X 7  // 横方向のヒントの最大セル数
#define MAX_HINT_Y 7  // 縦方向のヒントの最大セル数

#define LONG_PRESS_THRESHOLD 30  // 長押しとみなすフレーム数

// ボタンの状態を保持する構造体
typedef struct 
{
    u16 key;  // 現在のボタンの状態
    u16 keyr; // 長押し状態
    u16 key_up; // 上げ判定
    u16 keysHeld; // ボタンが押され続けている時間（フレーム数）
} KeyState;

KeyState keyState;

//RAM variable
u32 sel_cell_x = 1; //選択中セル横方向位置（初期位置：左上）
u32 sel_cell_y = 1; //選択中セル縦方向位置（初期位置：左上）

bool cel_state[50][50];

// グリッドの描画領域に基づいてCELL_SIZEを自動調整する
u32 calculateCellSize() 
{
    // ヒント部分のセル数＋色塗り部分のセル数
    u32 Widthcells = MAX_HINT_X + GRID_CELLS_X;  // 横方向
    u32 Heightcells = MAX_HINT_Y + GRID_CELLS_Y; // 縦方向

    // 色塗り部分のセル数に基づいて、各セルのサイズを決定
    u32 cellSizeX = SCREEN_WIDTH / Widthcells;
    u32 cellSizeY = SCREEN_HEIGHT / Heightcells;

    // どちらか小さい方をCELL_SIZEとして使用（セルがはみ出ないようにする）
    return (cellSizeX < cellSizeY) ? cellSizeX : cellSizeY;
}

void Mode3PutPixel(u32 x, u32 y, u16 color)
{
    u16* ScreenBuffer = (u16*)0x6000000;
    ScreenBuffer[y * 240 + x] = color;
}

// グリッドを描画する関数（ヒント部分を追加）
void drawGrid(u32 cellSize)
{
    // グリッド全体の幅と高さ（ヒント部分を含む）
    u32 totalWidth = (MAX_HINT_X + GRID_CELLS_X) * cellSize;
    u32 totalHeight = (MAX_HINT_Y + GRID_CELLS_Y) * cellSize;

    for (u32 y = 0; y <= totalHeight; y++)
    {
        for (u32 x = 0; x <= totalWidth; x++)
        {
            // ヒント部分の描画（上側と左側の部分）
            if (x < MAX_HINT_X * cellSize || y < MAX_HINT_Y * cellSize)
            {
                Mode3PutPixel(x, y, HINT_COLOR);
            }
            // 色塗り部分のグリッド線の描画（セルの境界）
            else if (x % cellSize == 0 || y % cellSize == 0) 
            {
                if((x == MAX_HINT_X * cellSize + 5*cellSize)
                || (x == MAX_HINT_X * cellSize + 10*cellSize)
                || (y == MAX_HINT_Y * cellSize + 5*cellSize)
                || (y == MAX_HINT_Y * cellSize + 10*cellSize))
                {
                    Mode3PutPixel(x, y, GRID_FIVE_COLOR);
                }
                else
                {
                    Mode3PutPixel(x, y, GRID_COLOR);
                }
            }
            else
            {
                Mode3PutPixel(x, y, BG_COLOR);  // 背景色を塗りつぶす
            }
        }
    }
}

void drawSelCell(u32 cellSize, u32 sel_x, u32 sel_y, bool col)
{
    //セル初期位置
    u32 start_cell_picx = MAX_HINT_X*cellSize;
    u32 start_cell_picy = MAX_HINT_Y*cellSize;

    //描画開始位置
    u32 start_draw_picx = start_cell_picx + (sel_x - 1) * cellSize;
    u32 start_draw_picy = start_cell_picy + (sel_y - 1) * cellSize;

    int draw_color;

    if(col == 1)
    {
        draw_color = SELECT_CELL_COLOR;
    }
    else
    {
        draw_color = GRID_COLOR;
    }

    for (u32 i = 0; i <= cellSize; i++) 
    {
        Mode3PutPixel(start_draw_picx + i, start_draw_picy, draw_color);             //選択セル描画（上辺）
        Mode3PutPixel(start_draw_picx + i, start_draw_picy + cellSize, draw_color);  //選択セル描画（下辺）
        Mode3PutPixel(start_draw_picx, start_draw_picy + i, draw_color);             //選択セル描画（左辺）
        Mode3PutPixel(start_draw_picx + cellSize, start_draw_picy + i, draw_color);  //選択セル描画（右辺）
    }

    if(col == 0)
    {
        if(sel_x == 5 || sel_x == 10)
        {
            //右辺FIVEセル塗
            for (u32 i = 0; i <= cellSize; i++) 
            {
                Mode3PutPixel(start_draw_picx + cellSize, start_draw_picy + i, GRID_FIVE_COLOR);  //選択セル描画（右辺）
            }
        }
        if(sel_x == 6 || sel_x == 11)
        {
            //左辺FIVEセル塗
            for (u32 i = 0; i <= cellSize; i++) 
            {
                Mode3PutPixel(start_draw_picx, start_draw_picy + i, GRID_FIVE_COLOR);             //選択セル描画（左辺）
            }
        }
        if(sel_y == 5 || sel_y == 10)
        {
            //下辺FIVEセル塗
            for (u32 i = 0; i <= cellSize; i++) 
            {
                Mode3PutPixel(start_draw_picx + i, start_draw_picy + cellSize, GRID_FIVE_COLOR);  //選択セル描画（下辺）
            }
        }
        if(sel_y == 6 || sel_y == 11)
        {
            //上辺FIVEセル塗
            for (u32 i = 0; i <= cellSize; i++) 
            {
                Mode3PutPixel(start_draw_picx + i, start_draw_picy, GRID_FIVE_COLOR);             //選択セル描画（上辺）
            }
        }
    }
}

void drawCell(u32 cellSize, bool col) 
{
    //塗りセル初期位置
    u32 start_cell_picx = MAX_HINT_X*cellSize;
    u32 start_cell_picy = MAX_HINT_Y*cellSize;

    //描画開始位置
    u32 start_draw_picx = start_cell_picx + (sel_cell_x - 1) * cellSize;
    u32 start_draw_picy = start_cell_picy + (sel_cell_y - 1) * cellSize;

    int draw_color;

    if(col == 1)
    {
        draw_color = GRID_COLOR;
    }
    else
    {
        draw_color = BG_COLOR;
    }

    for (u32 i = 1; i < cellSize; i++) 
    {
        for (u32 j = 1; j < cellSize; j++) 
        {
            Mode3PutPixel(start_draw_picx + i, start_draw_picy +j, draw_color);
        }
    }
}

// ボタンの状態を更新する関数
void updateKeyState(void)
{
    scanKeys();
    keyState.key = keysDown();  // 1フレーム前の状態を保持
    keyState.keyr = keysDownRepeat(); // 現在のボタンの状態を読み取る
    keyState.key_up = keysUp();
    keyState.keysHeld = keysHeld();
}

void select_cell_update()
{
    if (((keyState.key & KEY_LEFT) || (keyState.keyr & KEY_LEFT)) && (sel_cell_x > 1))
    {
        sel_cell_x--;
    }

    if (((keyState.key & KEY_RIGHT) || (keyState.keyr & KEY_RIGHT)) && (sel_cell_x < GRID_CELLS_X))
    {
        sel_cell_x++;
    }

    if (((keyState.key & KEY_UP) || (keyState.keyr & KEY_UP)) && (sel_cell_y > 1))
    {
        sel_cell_y--;
    }

    if (((keyState.key & KEY_DOWN) || (keyState.keyr & KEY_DOWN)) && (sel_cell_y < GRID_CELLS_Y))
    {
        sel_cell_y++;
    }
}

int main(void) 
{
    // GBAをモード3に設定（240x160の16ビットカラー）
    REG_DISPCNT = MODE_3 | BG2_ENABLE;

    irqInit();
	irqEnable(IRQ_VBLANK);
    setRepeat(10,5);

    // CELL_SIZE自動調整
    u32 cellSize = calculateCellSize();

    // グリッドを描画
    drawGrid(cellSize);
    drawSelCell(cellSize, sel_cell_x, sel_cell_y, 1);

    //長押し描画指定
    int draw_cell_color = 2; //長押し中の塗セル指定をリセット

	while (1)
    {
        VBlankIntrWait();

        u32 sel_cell_x_old = sel_cell_x;
        u32 sel_cell_y_old = sel_cell_y;

        updateKeyState();  // ボタン状態更新
        select_cell_update(); //選択セル更新

        if(keyState.key & KEY_A)
        {
            if(cel_state[sel_cell_x][sel_cell_y] == 0)
            {
                drawCell(cellSize, 1);
                cel_state[sel_cell_x][sel_cell_y] = 1;
                draw_cell_color = 1;                     //長押し中の塗セル指定を黒に
            }
            else
            {
                drawCell(cellSize, 0);
                cel_state[sel_cell_x][sel_cell_y] = 0;
                draw_cell_color = 0;                     //長押し中の塗セル指定を白に
            }
        }

        if((sel_cell_x != sel_cell_x_old) || (sel_cell_y != sel_cell_y_old))
        {
            drawSelCell(cellSize, sel_cell_x_old, sel_cell_y_old, 0);
            drawSelCell(cellSize, sel_cell_x, sel_cell_y, 1);

            if(keyState.key & KEY_A)
            {
                if(cel_state[sel_cell_x][sel_cell_y] == 0)
                {
                    drawCell(cellSize, 1);
                    cel_state[sel_cell_x][sel_cell_y] = 1;
                    draw_cell_color = 1;
                }
                else
                {
                    drawCell(cellSize, 0);
                    cel_state[sel_cell_x][sel_cell_y] = 0;
                    draw_cell_color = 0;
                }
            }
            else if(keyState.keysHeld & KEY_A)
            {
                if((cel_state[sel_cell_x][sel_cell_y] == 0) && (draw_cell_color == 1))
                {
                    drawCell(cellSize, 1);
                    cel_state[sel_cell_x][sel_cell_y] = 1;
                }
                else if((cel_state[sel_cell_x][sel_cell_y] == 1) && (draw_cell_color == 0))
                {
                    drawCell(cellSize, 0);
                    cel_state[sel_cell_x][sel_cell_y] = 0;
                }
            }
        }

        if(keyState.key_up & KEY_A){
            draw_cell_color = 2; //長押し中の塗セル指定をリセット
        }
	}

    return 0;
}